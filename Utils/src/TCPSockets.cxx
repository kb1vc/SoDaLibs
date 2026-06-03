/*
  Copyright (c) 2026 Matthew H. Reilly (kb1vc)
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

  Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
  Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in
  the documentation and/or other materials provided with the
  distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/**
 * @file TCPSockets.cxx
 * @author Matthew H. Reilly (kb1vc)
 * @date June 3, 2026
 * @author Claude Sonnet 4.6 (Sparky) -- co-author
 */

#include "TCPSockets.hxx"

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <signal.h>

namespace SoDa {
namespace TCP {

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

static void logSSLErrors(const char *tag) {
  unsigned long e;
  while ((e = ERR_get_error()) != 0) {
    std::cerr << "TCPSockets [" << tag << "]: "
              << ERR_error_string(e, nullptr) << "\n";
  }
}

// Retry SSL_write on WANT_WRITE/WANT_READ; return bytes written or -1.
static int sslLoopWrite(SSL *ssl, const void *ptr, int len) {
  const char *p = static_cast<const char *>(ptr);
  int left = len;
  while (left > 0) {
    int n = SSL_write(ssl, p, left);
    if (n > 0) { p += n; left -= n; continue; }
    int err = SSL_get_error(ssl, n);
    if (err == SSL_ERROR_WANT_WRITE || err == SSL_ERROR_WANT_READ) continue;
    logSSLErrors("SSL_write");
    return -1;
  }
  return len;
}

// Retry SSL_read on WANT_READ/WANT_WRITE; return bytes read, 0 on clean
// close, or -1 on error.
static int sslLoopRead(SSL *ssl, void *ptr, int len) {
  while (true) {
    int n = SSL_read(ssl, ptr, len);
    if (n > 0) return n;
    int err = SSL_get_error(ssl, n);
    if (err == SSL_ERROR_ZERO_RETURN) return 0;
    if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) continue;
    logSSLErrors("SSL_read");
    return -1;
  }
}

// ---------------------------------------------------------------------------
// NetSocket — context builders
// ---------------------------------------------------------------------------

static SSL_CTX *buildCTX(bool is_server,
                          const std::string & ca_cert_path,
                          const std::string & own_cert_path,
                          const std::string & own_key_path) {
  // Writing to a closed TLS socket raises SIGPIPE; ignore it so SSL_write
  // returns -1 instead of killing the process.
  signal(SIGPIPE, SIG_IGN);

  SSL_CTX *ctx = SSL_CTX_new(is_server ? TLS_server_method()
                                       : TLS_client_method());
  if (!ctx) { logSSLErrors("SSL_CTX_new"); return nullptr; }

  SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);

  // Require and verify peer certificate.
  int verify_flags = SSL_VERIFY_PEER;
  if (is_server) verify_flags |= SSL_VERIFY_FAIL_IF_NO_PEER_CERT;
  SSL_CTX_set_verify(ctx, verify_flags, nullptr);

  if (!SSL_CTX_load_verify_locations(ctx, ca_cert_path.c_str(), nullptr)) {
    logSSLErrors("load CA cert");
    SSL_CTX_free(ctx); return nullptr;
  }

  if (!SSL_CTX_use_certificate_file(ctx, own_cert_path.c_str(), SSL_FILETYPE_PEM)) {
    logSSLErrors("load own cert");
    SSL_CTX_free(ctx); return nullptr;
  }

  if (!SSL_CTX_use_PrivateKey_file(ctx, own_key_path.c_str(), SSL_FILETYPE_PEM)) {
    logSSLErrors("load own key");
    SSL_CTX_free(ctx); return nullptr;
  }

  if (!SSL_CTX_check_private_key(ctx)) {
    logSSLErrors("check private key");
    SSL_CTX_free(ctx); return nullptr;
  }

  return ctx;
}

void *NetSocket::makeServerCTX(const std::string & ca_cert_path,
                                const std::string & server_cert_path,
                                const std::string & server_key_path) {
  return buildCTX(true, ca_cert_path, server_cert_path, server_key_path);
}

void *NetSocket::makeClientCTX(const std::string & ca_cert_path,
                                const std::string & client_cert_path,
                                const std::string & client_key_path) {
  return buildCTX(false, ca_cert_path, client_cert_path, client_key_path);
}

std::string NetSocket::getPeerCN() {
  X509 *peer = SSL_get_peer_certificate(static_cast<SSL *>(ssl));
  if (!peer) return "";
  char cn[256] = {};
  X509_NAME_get_text_by_NID(X509_get_subject_name(peer),
                             NID_commonName, cn, (int)sizeof(cn) - 1);
  X509_free(peer);
  return std::string(cn);
}

// ---------------------------------------------------------------------------
// NetSocket — put / get
// ---------------------------------------------------------------------------

int NetSocket::tlsPut(const void *ptr, unsigned int size, bool len_prefix) {
  SSL *s = static_cast<SSL *>(ssl);
  if (len_prefix) {
    if (sslLoopWrite(s, &size, sizeof(unsigned int)) < 0) return -1;
  }
  return sslLoopWrite(s, ptr, (int)size) < 0 ? -1 : 0;
}

int NetSocket::tlsGet(void *ptr, unsigned int size, bool len_prefix) {
  SSL *s = static_cast<SSL *>(ssl);
  unsigned int rsize = size;

  if (len_prefix) {
    int n = sslLoopRead(s, &rsize, sizeof(unsigned int));
    if (n <= 0) return n;
  }

  unsigned int to_copy = (rsize < size) ? rsize : size;
  int n = sslLoopRead(s, ptr, (int)to_copy);
  if (n <= 0) return n;

  // Drain surplus bytes that don't fit in the caller's buffer.
  unsigned int surplus = (rsize > size) ? (rsize - size) : 0;
  while (surplus > 0) {
    char sink[256];
    unsigned int chunk = (surplus > sizeof(sink)) ? (unsigned int)sizeof(sink) : surplus;
    int r = sslLoopRead(s, sink, (int)chunk);
    if (r <= 0) break;
    surplus -= (unsigned int)r;
  }

  return n;
}

// ---------------------------------------------------------------------------
// ServerSocket
// ---------------------------------------------------------------------------

ServerSocket::ServerSocket(const std::string & addr, size_t port_num,
                           const std::string & ca_cert_path,
                           const std::string & server_cert_path,
                           const std::string & server_key_path)
  : ready(false) {
  ssl = nullptr;
  conn_socket = -1;

  ctx = makeServerCTX(ca_cert_path, server_cert_path, server_key_path);
  if (!ctx) {
    std::cerr << "TCP::ServerSocket: failed to build TLS context\n";
    exit(-1);
  }

  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket < 0) {
    std::cerr << "TCP::ServerSocket: socket() failed\n";
    exit(-1);
  }

  int opt = 1;
  setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  // Non-blocking so isReady() never blocks on accept().
  int flags = fcntl(server_socket, F_GETFL, 0);
  fcntl(server_socket, F_SETFL, flags | O_NONBLOCK);

  struct sockaddr_in sa;
  memset(&sa, 0, sizeof(sa));
  sa.sin_family = AF_INET;
  sa.sin_port   = htons((uint16_t)port_num);
  if (addr.empty() || addr == "*") {
    sa.sin_addr.s_addr = INADDR_ANY;
  } else {
    inet_pton(AF_INET, addr.c_str(), &sa.sin_addr);
  }

  if (bind(server_socket, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
    std::cerr << "TCP::ServerSocket: bind() failed on "
              << addr << ":" << port_num << "\n";
    exit(-1);
  }
  if (listen(server_socket, 1) < 0) {
    std::cerr << "TCP::ServerSocket: listen() failed\n";
    exit(-1);
  }

  std::cerr << "TCP::ServerSocket: listening on "
            << (addr.empty() ? "0.0.0.0" : addr) << ":" << port_num << "\n";
}

ServerSocket::~ServerSocket() {
  // ssl and conn_socket may already be nullptr/-1 if get() detected a
  // disconnect and cleaned up; the guards here prevent double-free.
  if (ssl) { SSL_shutdown(static_cast<SSL *>(ssl)); SSL_free(static_cast<SSL *>(ssl)); }
  if (conn_socket >= 0)   close(conn_socket);
  if (server_socket >= 0) close(server_socket);
  if (ctx) SSL_CTX_free(static_cast<SSL_CTX *>(ctx));
}

bool ServerSocket::isReady(size_t) {
  if (ready) return true;

  // Free any leftover session from the previous client before accepting a
  // new one (avoids both a memory leak and a double-close on the old fd).
  if (ssl) {
    SSL_free(static_cast<SSL *>(ssl));
    ssl = nullptr;
  }
  if (conn_socket >= 0) {
    close(conn_socket);
    conn_socket = -1;
  }

  struct sockaddr_in client_addr;
  socklen_t ca_len = sizeof(client_addr);
  int ns = accept(server_socket, (struct sockaddr *)&client_addr, &ca_len);
  if (ns < 0) return false;

  SSL *s = SSL_new(static_cast<SSL_CTX *>(ctx));
  SSL_set_fd(s, ns);

  if (SSL_accept(s) <= 0) {
    logSSLErrors("SSL_accept");
    std::cerr << "TCP::ServerSocket: TLS handshake failed (untrusted client cert)\n";
    SSL_free(s);
    close(ns);
    return false;
  }

  ssl         = s;
  conn_socket = ns;
  connected_identity = getPeerCN();

  std::cerr << "TCP::ServerSocket: client [" << connected_identity
            << "] connected (cipher: " << SSL_get_cipher_name(s) << ")\n";

  ready = true;
  return true;
}

int ServerSocket::get(void *ptr, unsigned int size, bool len_prefix) {
  int rv = tlsGet(ptr, size, len_prefix);
  if (rv <= 0) {
    // Tear down the TLS session so isReady() can accept the next client
    // without leaking the SSL object or the connected file descriptor.
    if (ssl) {
      SSL_shutdown(static_cast<SSL *>(ssl));
      SSL_free(static_cast<SSL *>(ssl));
      ssl = nullptr;
    }
    if (conn_socket >= 0) {
      close(conn_socket);
      conn_socket = -1;
    }
    ready = false;
    connected_identity.clear();
  }
  return rv;
}

int ServerSocket::put(const void *ptr, unsigned int size, bool len_prefix) {
  return tlsPut(ptr, size, len_prefix);
}

// ---------------------------------------------------------------------------
// ClientSocket
// ---------------------------------------------------------------------------

ClientSocket::ClientSocket(const std::string & addr, size_t port_num,
                           const std::string & ca_cert_path,
                           const std::string & client_cert_path,
                           const std::string & client_key_path,
                           int startup_timeout_count) {
  ssl = nullptr;
  server_socket = -1;

  ctx = makeClientCTX(ca_cert_path, client_cert_path, client_key_path);
  if (!ctx) {
    std::cerr << "TCP::ClientSocket: failed to build TLS context\n";
    exit(-1);
  }

  conn_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (conn_socket < 0) {
    std::cerr << "TCP::ClientSocket: socket() failed\n";
    exit(-1);
  }

  struct sockaddr_in sa;
  memset(&sa, 0, sizeof(sa));
  sa.sin_family = AF_INET;
  sa.sin_port   = htons((uint16_t)port_num);

  struct hostent *host = gethostbyname(addr.c_str());
  if (host) {
    memcpy(&sa.sin_addr, host->h_addr_list[0], (size_t)host->h_length);
  } else {
    inet_pton(AF_INET, addr.c_str(), &sa.sin_addr);
  }

  int stat = -1;
  for (int i = 0; i < startup_timeout_count; i++) {
    stat = connect(conn_socket, (struct sockaddr *)&sa, sizeof(sa));
    if (stat >= 0) break;
    sleep(1);
  }
  if (stat < 0) {
    std::cerr << "TCP::ClientSocket: connect() to "
              << addr << ":" << port_num << " failed\n";
    exit(-1);
  }

  SSL *s = SSL_new(static_cast<SSL_CTX *>(ctx));
  SSL_set_fd(s, conn_socket);

  if (SSL_connect(s) <= 0) {
    logSSLErrors("SSL_connect");
    std::cerr << "TCP::ClientSocket: TLS handshake failed\n";
    SSL_free(s);
    exit(-1);
  }

  ssl = s;

  std::cerr << "TCP::ClientSocket: connected to " << addr << ":" << port_num
            << " (cipher: " << SSL_get_cipher_name(s) << ")\n";
}

ClientSocket::~ClientSocket() {
  if (ssl) {
    SSL_shutdown(static_cast<SSL *>(ssl));
    SSL_free(static_cast<SSL *>(ssl));
  }
  if (conn_socket >= 0) close(conn_socket);
  if (ctx) SSL_CTX_free(static_cast<SSL_CTX *>(ctx));
}

int ClientSocket::get(void *ptr, unsigned int size, bool len_prefix) {
  return tlsGet(ptr, size, len_prefix);
}

int ClientSocket::put(const void *ptr, unsigned int size, bool len_prefix) {
  return tlsPut(ptr, size, len_prefix);
}

} // namespace TCP
} // namespace SoDa
