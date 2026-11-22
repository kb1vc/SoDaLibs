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
#include "Format.hxx"

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
#include <mutex>
#include <signal.h>

namespace SoDa {
namespace TCP {

// Maximum size of a single framed message. Any peer-supplied length prefix
// above this is treated as hostile and the read is aborted.
static constexpr size_t max_TCP_msg_size = 8u * 1024u * 1024u;

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

// Install the SIGPIPE-ignore handler exactly once per process. Writing to a
// closed TLS socket raises SIGPIPE; we want SSL_write to return -1 rather
// than the signal killing the process. Install from std::once so the library
// stops stomping on this handler every time a context is built.
static void ignore_sigpipe_once() {
  static std::once_flag flag;
  std::call_once(flag, []{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGPIPE, &sa, nullptr);
  });
}

static SSL_CTX *buildCTX(bool is_server,
                          const std::string & ca_cert_path,
                          const std::string & own_cert_path,
                          const std::string & own_key_path) {
  ignore_sigpipe_once();

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
  size_t rsize = size;

  if (len_prefix) {
    // Wire format is a 32-bit length (matches tlsPut). Promote to size_t
    // for all arithmetic so an rsize of 0x80000000+ can't poison signed math.
    unsigned int wire_rsize;
    int n = sslLoopRead(s, &wire_rsize, sizeof(wire_rsize));
    if (n <= 0) return n;
    rsize = wire_rsize;

    // Reject any peer-supplied frame larger than the hard cap. Even on a
    // TLS-authenticated session a compromised or buggy peer must not be able
    // to pin the server in a multi-gigabyte drain loop.
    if (rsize > max_TCP_msg_size) {
      std::cerr << Format("TCP::NetSocket::tlsGet: rejecting oversized frame (rsize=%0, max=%1)\n")
	.addU(rsize)
	.addU(max_TCP_msg_size);
      return -1;
    }
  }

  size_t to_copy = (rsize < size) ? rsize : size;
  int n = sslLoopRead(s, ptr, (int)to_copy);
  if (n <= 0) return n;

  // Drain surplus bytes that don't fit in the caller's buffer.
  size_t surplus = (rsize > size) ? (rsize - size) : 0;
  while (surplus > 0) {
    char sink[256];
    size_t chunk = (surplus > sizeof(sink)) ? sizeof(sink) : surplus;
    int r = sslLoopRead(s, sink, (int)chunk);
    if (r <= 0) break;
    surplus -= (size_t)r;
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
  server_socket = -1;

  ctx = makeServerCTX(ca_cert_path, server_cert_path, server_key_path);
  if (!ctx) {
    throw SocketError("failed to build TLS context (check CA/cert/key paths)");
  }

  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket < 0) {
    int e = errno;
    SSL_CTX_free(static_cast<SSL_CTX *>(ctx));
    throw SocketError(Format("socket() failed: %0").addS(strerror(e)).str());
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
    if (inet_pton(AF_INET, addr.c_str(), &sa.sin_addr) != 1) {
      close(server_socket);
      SSL_CTX_free(static_cast<SSL_CTX *>(ctx));
      throw SocketError(Format("bind address [%0] is not a valid IPv4 literal")
			.addS(addr).str());
    }
  }

  if (bind(server_socket, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
    int e = errno;
    close(server_socket);
    SSL_CTX_free(static_cast<SSL_CTX *>(ctx));
    throw SocketError(Format("bind() failed on %0:%1: %2")
		      .addS(addr)
		      .addU(port_num)
		      .addS(strerror(e)).str());
  }
  if (listen(server_socket, 1) < 0) {
    int e = errno;
    close(server_socket);
    SSL_CTX_free(static_cast<SSL_CTX *>(ctx));
    throw SocketError(Format("listen() failed: %0").addS(strerror(e)).str());
  }
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
  conn_socket = -1;

  ctx = makeClientCTX(ca_cert_path, client_cert_path, client_key_path);
  if (!ctx) {
    throw SocketError("failed to build TLS client context (check CA/cert/key paths)");
  }

  // Resolve addr via getaddrinfo (thread-safe, IPv6-aware) instead of the
  // deprecated gethostbyname. Try each returned address in turn, with up to
  // startup_timeout_count 1-second retries over the whole set.
  struct addrinfo hints;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family   = AF_INET;         // keep IPv4-only to match the rest of the module
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  std::string port_str = Format("%0").addU(port_num).str();
  struct addrinfo *res = nullptr;
  int gai = getaddrinfo(addr.c_str(), port_str.c_str(), &hints, &res);
  if (gai != 0 || !res) {
    SSL_CTX_free(static_cast<SSL_CTX *>(ctx));
    throw SocketError(Format("getaddrinfo(%0:%1) failed: %2")
		      .addS(addr)
		      .addU(port_num)
		      .addS(gai_strerror(gai)).str());
  }

  int stat = -1;
  for (int attempt = 0; attempt < startup_timeout_count && stat < 0; attempt++) {
    for (struct addrinfo *ai = res; ai != nullptr && stat < 0; ai = ai->ai_next) {
      if (conn_socket >= 0) close(conn_socket);
      conn_socket = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
      if (conn_socket < 0) continue;
      stat = connect(conn_socket, ai->ai_addr, ai->ai_addrlen);
    }
    if (stat < 0) sleep(1);
  }
  freeaddrinfo(res);

  if (stat < 0) {
    int e = errno;
    if (conn_socket >= 0) close(conn_socket);
    SSL_CTX_free(static_cast<SSL_CTX *>(ctx));
    throw SocketError(Format("connect() to %0:%1 failed: %2")
		      .addS(addr)
		      .addU(port_num)
		      .addS(strerror(e)).str());
  }

  SSL *s = SSL_new(static_cast<SSL_CTX *>(ctx));
  if (!s) {
    close(conn_socket);
    SSL_CTX_free(static_cast<SSL_CTX *>(ctx));
    throw SocketError("SSL_new() returned null (out of memory?)");
  }
  SSL_set_fd(s, conn_socket);

  // Pin the peer identity to the hostname (or IP literal) the caller asked
  // for. Without this, any cert signed by the trusted CA can impersonate any
  // server -- OpenSSL only verifies the chain by default, not that the
  // certificate was issued for the host we're talking to.
  {
    struct in_addr  v4;
    struct in6_addr v6;
    bool is_ip_literal = (inet_pton(AF_INET,  addr.c_str(), &v4) == 1)
                      || (inet_pton(AF_INET6, addr.c_str(), &v6) == 1);
    int ok = is_ip_literal
      ? X509_VERIFY_PARAM_set1_ip_asc(SSL_get0_param(s), addr.c_str())
      : SSL_set1_host(s, addr.c_str());
    if (ok != 1) {
      logSSLErrors("set peer identity");
      SSL_free(s);
      close(conn_socket);
      SSL_CTX_free(static_cast<SSL_CTX *>(ctx));
      throw SocketError(Format("can't pin peer identity to [%0]").addS(addr).str());
    }
  }

  if (SSL_connect(s) <= 0) {
    logSSLErrors("SSL_connect");
    SSL_free(s);
    close(conn_socket);
    SSL_CTX_free(static_cast<SSL_CTX *>(ctx));
    throw SocketError(Format("TLS handshake with %0:%1 failed")
		      .addS(addr)
		      .addU(port_num).str());
  }

  ssl = s;
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
