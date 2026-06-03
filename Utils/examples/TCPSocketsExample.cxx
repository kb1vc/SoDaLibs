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
 * @file TCPSocketsExample.cxx
 * @author Matt Reilly (kb1vc)
 * @date June 3, 2026
 * @author Claude Sonnet 4.6 (Sparky) -- co-author
 *
 * @brief Demonstrates SoDa::TCP::ServerSocket and ClientSocket with X.509
 *        mutual authentication.
 *
 * This self-contained example generates ephemeral test certificates at
 * startup (P-256 ECDSA, signed by a local CA) and runs an echo server with
 * two sequential clients (alice, then bob).  It shows:
 *
 *  - Loading CA, cert, and key paths into the server and client.
 *  - Reading the authenticated client identity via getConnectedIdentity().
 *  - Sending and receiving messages through the encrypted, authenticated
 *    channel.
 *  - Sequential multi-client use: the server accepts alice, then bob,
 *    without restarting.
 *
 * In a production deployment, generate certs with the openssl CLI once and
 * store them securely (key files chmod 600).  See the @ref SoDa_TCP page for
 * the exact commands.  This example generates throwaway certs in /tmp purely
 * for demonstration; do not do this in production.
 */

#include <SoDa/TCPSockets.hxx>
#include <SoDa/Options.hxx>

// OpenSSL headers are used here only for generating the demo certs.
// Production code using SoDa::TCP does not need to include these.
#include <openssl/ssl.h>
#include <openssl/evp.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/pem.h>

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <cstring>
#include <sys/stat.h>

#include <unistd.h>

// ---------------------------------------------------------------------------
// Ephemeral certificate generation (demo use only)
// ---------------------------------------------------------------------------
// In production, generate certs with the openssl CLI and store them at a
// fixed path.  The helpers below are here only to make the example
// self-contained.

static EVP_PKEY *makeECKey() { return EVP_EC_gen("P-256"); }

static X509 *makeCert(EVP_PKEY *key, const char *cn,
                      X509 *issuer_cert, EVP_PKEY *issuer_key,
                      long serial, bool is_ca) {
  X509 *cert = X509_new();
  X509_set_version(cert, 2);
  ASN1_INTEGER_set(X509_get_serialNumber(cert), serial);
  X509_gmtime_adj(X509_getm_notBefore(cert), 0);
  X509_gmtime_adj(X509_getm_notAfter(cert), 3600L); // 1-hour demo cert
  X509_set_pubkey(cert, key);

  X509_NAME *name = X509_get_subject_name(cert);
  X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC,
                             (const unsigned char *)cn, -1, -1, 0);
  if (issuer_cert) {
    X509_set_issuer_name(cert, X509_get_subject_name(issuer_cert));
  } else {
    X509_set_issuer_name(cert, name);
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

static void writePEM(const std::string &path, X509 *cert, EVP_PKEY *key) {
  if (cert) { FILE *f = fopen(path.c_str(), "w"); PEM_write_X509(f, cert); fclose(f); }
  if (key)  {
    FILE *f = fopen(path.c_str(), "w");
    PEM_write_PrivateKey(f, key, nullptr, nullptr, 0, nullptr, nullptr);
    fclose(f);
    chmod(path.c_str(), 0600);
  }
}

struct DemoCerts {
  std::string ca_crt;
  std::string srv_crt, srv_key;
  std::string alice_crt, alice_key;
  std::string bob_crt,   bob_key;
};

static DemoCerts makeDemoCerts(const std::string &prefix) {
  DemoCerts dc;
  EVP_PKEY *ca_key  = makeECKey();
  X509     *ca_cert = makeCert(ca_key, "DemoCA", nullptr, nullptr, 1, true);
  dc.ca_crt = prefix + "ca.crt";
  writePEM(dc.ca_crt, ca_cert, nullptr);

  auto issue = [&](const char *cn, long serial,
                   std::string &crt, std::string &key_path) {
    EVP_PKEY *k = makeECKey();
    X509     *x = makeCert(k, cn, ca_cert, ca_key, serial, false);
    crt      = prefix + cn + ".crt";
    key_path = prefix + cn + ".key";
    writePEM(crt, x, nullptr);
    writePEM(key_path, nullptr, k);
    X509_free(x); EVP_PKEY_free(k);
  };

  issue("server", 2, dc.srv_crt,   dc.srv_key);
  issue("alice",  3, dc.alice_crt, dc.alice_key);
  issue("bob",    4, dc.bob_crt,   dc.bob_key);

  X509_free(ca_cert); EVP_PKEY_free(ca_key);
  return dc;
}

// ---------------------------------------------------------------------------
// Echo server
// ---------------------------------------------------------------------------

static void runEchoServer(int port, const DemoCerts &dc,
                          std::atomic<bool> & ready,
                          std::vector<std::string> & log,
                          int num_clients) {
//! [cert paths]
  // In production these paths would come from a config file or --option.
  const std::string & ca         = dc.ca_crt;
  const std::string & server_crt = dc.srv_crt;
  const std::string & server_key = dc.srv_key;
//! [cert paths]

//! [create server]
  auto srv = SoDa::TCP::ServerSocket::make("127.0.0.1", (size_t)port,
                                           ca, server_crt, server_key);
//! [create server]

  ready = true;

  for (int i = 0; i < num_clients; ++i) {

//! [wait for client]
    while (!srv->isReady()) usleep(5000);
    std::string who = srv->getConnectedIdentity();
    log.push_back(who);
    std::cout << "[server] " << who << " connected\n";
//! [wait for client]

//! [echo loop]
    char buf[512];
    int n;
    while ((n = srv->get(buf, sizeof(buf) - 1)) > 0) {
      buf[n] = '\0';
      std::cout << "[server] echo to " << who << ": " << buf << "\n";
      srv->put(buf, (unsigned int)n);
    }
    std::cout << "[server] " << who << " disconnected\n";
//! [echo loop]
  }
}

// ---------------------------------------------------------------------------
// Client session
// ---------------------------------------------------------------------------

static void runClient(int port, const DemoCerts &dc,
                      const std::string & client_crt,
                      const std::string & client_key,
                      const std::vector<std::string> & msgs) {
//! [create client]
  // Each client presents its own cert; the server reads the CN as the identity.
  auto cli = SoDa::TCP::ClientSocket::make(
      "127.0.0.1", (size_t)port,
      dc.ca_crt,
      client_crt,
      client_key);
//! [create client]

  for (const auto & msg : msgs) {
//! [send and receive]
    cli->put(msg.c_str(), (unsigned int)msg.size() + 1);

    char resp[512] = {};
    int n = cli->get(resp, sizeof(resp) - 1);
    if (n > 0) std::cout << "[client] received: " << resp << "\n";
//! [send and receive]
  }
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main(int argc, char ** argv) {
  SoDa::Options cmd;
  int port;
  std::string cert_dir;
  cmd.add<int>(&port, "port", 'p', 19400, "TCP port for the demo server");
  cmd.add<std::string>(&cert_dir, "certdir", 'd', "/tmp/TCPSocketsExample_",
                       "Prefix for ephemeral demo cert files");
  if (!cmd.parse(argc, argv)) return 1;

  DemoCerts dc = makeDemoCerts(cert_dir);

  std::atomic<bool> server_ready{false};
  std::vector<std::string> client_log;

  std::thread srv_thread(runEchoServer, port, std::cref(dc),
                         std::ref(server_ready), std::ref(client_log), 2);

  while (!server_ready) usleep(5000);

//! [alice session]
  std::cout << "\n--- Alice connects ---\n";
  runClient(port, dc, dc.alice_crt, dc.alice_key, { "Hello from alice", "How are you?" });
//! [alice session]

//! [bob session]
  std::cout << "\n--- Bob connects ---\n";
  runClient(port, dc, dc.bob_crt, dc.bob_key, { "Hi, I am bob" });
//! [bob session]

  srv_thread.join();

  std::cout << "\n--- Summary ---\n";
  for (const auto & who : client_log)
    std::cout << "  connected: " << who << "\n";

  return 0;
}
