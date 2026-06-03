/*
BSD 2-Clause License

Copyright (c) 2026 Matt Reilly - kb1vc
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/**
 * @file TCPSocketsTest.cxx
 * @author Matt Reilly (kb1vc)
 * @date June 3, 2026
 * @author Claude Sonnet 4.6 (Sparky) -- co-author
 *
 * Tests for SoDa::TCP::ServerSocket and ClientSocket (X.509 mutual auth).
 *
 * Test certs are generated at startup using the OpenSSL C API:
 *   CA       — self-signed, CN=TestCA
 *   server   — signed by CA, CN=server
 *   alice    — signed by CA, CN=alice
 *   bob      — signed by CA, CN=bob
 *   rogue    — self-signed (different CA), CN=rogue
 *
 * Test 1 — roundtrip: client (alice) sends, server echoes; content and
 *           identity verified.
 *
 * Test 2 — multi-client sequential: alice connects, sends, disconnects;
 *           bob then connects.  getConnectedIdentity() verified for each.
 *
 * Test 3 — rejection: a child process tries to connect with the rogue
 *           cert (not signed by the server's CA).  SSL_accept must fail;
 *           isReady() must never return true.
 */

#include "../include/TCPSockets.hxx"
#include "../include/Options.hxx"

#include <openssl/ssl.h>
#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include <cstring>
#include <sys/stat.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

static bool global_pass = true;

static void check(bool cond, const std::string & msg) {
  if (!cond) {
    std::cerr << "FAIL: " << msg << "\n";
    global_pass = false;
  }
}

// ---------------------------------------------------------------------------
// Certificate generation
// ---------------------------------------------------------------------------

struct Certs {
  std::string ca_crt;
  std::string srv_crt, srv_key;
  std::string alice_crt, alice_key;
  std::string bob_crt,   bob_key;
  std::string rogue_crt, rogue_key; // self-signed, not by CA
};

static EVP_PKEY *makeECKey() {
  return EVP_EC_gen("P-256");
}

static X509 *makeCert(EVP_PKEY *key, const char *cn,
                      X509 *issuer_cert, EVP_PKEY *issuer_key,
                      long serial, bool is_ca) {
  X509 *cert = X509_new();
  X509_set_version(cert, 2);
  ASN1_INTEGER_set(X509_get_serialNumber(cert), serial);
  X509_gmtime_adj(X509_getm_notBefore(cert), 0);
  X509_gmtime_adj(X509_getm_notAfter(cert), 365L * 24 * 3600);
  X509_set_pubkey(cert, key);

  X509_NAME *name = X509_get_subject_name(cert);
  X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC,
                             (const unsigned char *)cn, -1, -1, 0);

  if (issuer_cert) {
    X509_set_issuer_name(cert, X509_get_subject_name(issuer_cert));
  } else {
    X509_set_issuer_name(cert, name); // self-signed
    issuer_key = key;
  }

  if (is_ca) {
    X509V3_CTX v3ctx;
    X509V3_set_ctx_nodb(&v3ctx);
    X509V3_set_ctx(&v3ctx, cert, cert, nullptr, nullptr, 0);
    X509_EXTENSION *ex = X509V3_EXT_conf_nid(nullptr, &v3ctx,
                                             NID_basic_constraints, "CA:TRUE");
    if (ex) { X509_add_ext(cert, ex, -1); X509_EXTENSION_free(ex); }
  }

  X509_sign(cert, issuer_key, EVP_sha256());
  return cert;
}

static void writeCert(const std::string &path, X509 *cert) {
  FILE *f = fopen(path.c_str(), "w");
  PEM_write_X509(f, cert);
  fclose(f);
}

static void writeKey(const std::string &path, EVP_PKEY *key) {
  FILE *f = fopen(path.c_str(), "w");
  PEM_write_PrivateKey(f, key, nullptr, nullptr, 0, nullptr, nullptr);
  fclose(f);
  chmod(path.c_str(), 0600);
}

static Certs generateCerts(const std::string &prefix) {
  Certs c;

  // CA
  EVP_PKEY *ca_key  = makeECKey();
  X509     *ca_cert = makeCert(ca_key, "TestCA", nullptr, nullptr, 1, true);
  c.ca_crt = prefix + "ca.crt";
  writeCert(c.ca_crt, ca_cert);

  auto issue = [&](const char *cn, long serial,
                   std::string &crt_out, std::string &key_out) {
    EVP_PKEY *k = makeECKey();
    X509     *x = makeCert(k, cn, ca_cert, ca_key, serial, false);
    crt_out = prefix + cn + ".crt";
    key_out = prefix + cn + ".key";
    writeCert(crt_out, x);
    writeKey(key_out, k);
    X509_free(x);
    EVP_PKEY_free(k);
  };

  issue("server", 2, c.srv_crt,   c.srv_key);
  issue("alice",  3, c.alice_crt, c.alice_key);
  issue("bob",    4, c.bob_crt,   c.bob_key);

  // Rogue: self-signed (no CA) — server will reject it.
  EVP_PKEY *rk = makeECKey();
  X509     *rx = makeCert(rk, "rogue", nullptr, nullptr, 1, false);
  c.rogue_crt = prefix + "rogue.crt";
  c.rogue_key = prefix + "rogue.key";
  writeCert(c.rogue_crt, rx);
  writeKey(c.rogue_key, rk);
  X509_free(rx); EVP_PKEY_free(rk);

  X509_free(ca_cert); EVP_PKEY_free(ca_key);
  return c;
}

// ---------------------------------------------------------------------------
// Test 1: Basic roundtrip
// ---------------------------------------------------------------------------

static void testRoundtrip(int port, const Certs &c) {
  std::atomic<bool> server_up{false};
  std::string server_received;

  std::thread srv_thread([&]() {
    auto srv = SoDa::TCP::ServerSocket::make("127.0.0.1", (size_t)port,
                                             c.ca_crt, c.srv_crt, c.srv_key);
    server_up = true;

    while (!srv->isReady()) usleep(5000);
    check(srv->getConnectedIdentity() == "alice",
          "roundtrip: connected identity");

    char buf[256] = {};
    int n = srv->get(buf, sizeof(buf) - 1);
    check(n > 0, "roundtrip: server got bytes");
    server_received = buf;

    srv->put(buf, (unsigned int)n);
  });

  while (!server_up) usleep(5000);

  {
    auto cli = SoDa::TCP::ClientSocket::make("127.0.0.1", (size_t)port,
                                             c.ca_crt, c.alice_crt, c.alice_key);
    const char *msg = "Hello, TLS!";
    unsigned int msglen = (unsigned int)strlen(msg) + 1;
    cli->put(msg, msglen);

    char resp[256] = {};
    int n = cli->get(resp, sizeof(resp) - 1);
    check(n > 0, "roundtrip: client got response");
    check(std::string(resp) == msg, "roundtrip: content matches");
  }

  srv_thread.join();
  check(server_received == "Hello, TLS!", "roundtrip: server received correct content");
}

// ---------------------------------------------------------------------------
// Test 2: Multi-client sequential with identity verification
// ---------------------------------------------------------------------------

static void testMultiClient(int port, const Certs &c) {
  std::atomic<bool> server_up{false};
  std::string id1, id2;

  std::thread srv_thread([&]() {
    auto srv = SoDa::TCP::ServerSocket::make("127.0.0.1", (size_t)port,
                                             c.ca_crt, c.srv_crt, c.srv_key);
    server_up = true;

    // Alice
    while (!srv->isReady()) usleep(5000);
    id1 = srv->getConnectedIdentity();

    char buf[256] = {};
    srv->get(buf, sizeof(buf) - 1);
    srv->get(buf, sizeof(buf) - 1); // returns 0 on TLS close_notify → resets ready

    // Bob
    while (!srv->isReady()) usleep(5000);
    id2 = srv->getConnectedIdentity();

    char buf2[256] = {};
    srv->get(buf2, sizeof(buf2) - 1);
    srv->get(buf2, sizeof(buf2) - 1);
  });

  while (!server_up) usleep(5000);

  {
    auto cli = SoDa::TCP::ClientSocket::make("127.0.0.1", (size_t)port,
                                             c.ca_crt, c.alice_crt, c.alice_key);
    cli->put("from alice", 11);
  }

  {
    auto cli = SoDa::TCP::ClientSocket::make("127.0.0.1", (size_t)port,
                                             c.ca_crt, c.bob_crt, c.bob_key);
    cli->put("from bob", 9);
  }

  srv_thread.join();

  check(id1 == "alice", "multiClient: first client identity is alice");
  check(id2 == "bob",   "multiClient: second client identity is bob");
}

// ---------------------------------------------------------------------------
// Test 3: Rejection of a client with an untrusted certificate
// ---------------------------------------------------------------------------
// The rogue cert is self-signed and not issued by the server's CA.
// SSL_accept must fail; the child calls exit(-1).

static void testRejection(int port, const Certs &c) {
  int sync_pipe[2];
  if (pipe(sync_pipe) < 0) {
    std::cerr << "FAIL: rejection: pipe() failed\n";
    global_pass = false;
    return;
  }

  pid_t pid = fork();
  if (pid < 0) {
    std::cerr << "FAIL: rejection: fork() failed\n";
    global_pass = false;
    return;
  }

  if (pid == 0) {
    // Child: wait for server to start, then try with an untrusted cert.
    close(sync_pipe[1]);
    char ch;
    (void)read(sync_pipe[0], &ch, 1);
    close(sync_pipe[0]);
    SoDa::TCP::ClientSocket::make("127.0.0.1", (size_t)port,
                                  c.ca_crt,    // child trusts same CA
                                  c.rogue_crt, // but its cert isn't signed by it
                                  c.rogue_key, 3);
    exit(0); // unreachable — SSL_connect fails → exit(-1)
  }

  // Parent: create server, signal child.
  close(sync_pipe[0]);
  auto srv = SoDa::TCP::ServerSocket::make("127.0.0.1", (size_t)port,
                                           c.ca_crt, c.srv_crt, c.srv_key);
  (void)write(sync_pipe[1], "x", 1);
  close(sync_pipe[1]);

  auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(5);
  bool accepted = false;
  while (std::chrono::steady_clock::now() < deadline) {
    if (srv->isReady()) { accepted = true; break; }
    int status;
    if (waitpid(pid, &status, WNOHANG) > 0) { pid = -1; break; }
    usleep(10000);
  }

  check(!accepted, "rejection: server must not accept client with untrusted cert");

  if (pid > 0) {
    kill(pid, SIGTERM);
    waitpid(pid, nullptr, 0);
  }
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main(int argc, char ** argv) {
  SoDa::Options cmd;
  int base_port;
  std::string cert_prefix;
  cmd.add<int>(&base_port, "port", 'p', 19320,
               "Base TCP port; tests use base+1, base+2, base+3");
  cmd.add<std::string>(&cert_prefix, "certs", 'c', "/tmp/TCPSocketsTest_",
               "Prefix for generated test certificate files");
  if (!cmd.parse(argc, argv)) return 1;

  Certs certs = generateCerts(cert_prefix);

  testRoundtrip(base_port + 1, certs);
  testMultiClient(base_port + 2, certs);
  testRejection(base_port + 3, certs);

  if (global_pass) {
    std::cerr << "PASS\n";
  } else {
    std::cerr << "FAIL\n";
  }
  return global_pass ? 0 : 1;
}
