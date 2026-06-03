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
 * @file TCPSockets.hxx
 * @author Matthew H. Reilly (kb1vc)
 * @date June 3, 2026
 * @author Claude Sonnet 4.6 (Sparky) -- co-author
 *
 * @brief TLS-secured TCP socket wrappers with X.509 mutual authentication.
 */

#pragma once

/**
 * @page SoDa_TCP SoDa::TCP — TLS-authenticated TCP sockets
 *
 * ## Overview
 *
 * `SoDa::TCP` provides stream sockets over TCP with TLS mutual authentication
 * using X.509 certificates.  Every connection is authenticated on both sides
 * and encrypted; no shared secrets are embedded in source code.
 *
 * | Property | Provided |
 * |---|---|
 * | Payload encryption (ECDHE + AES-GCM) | yes |
 * | Integrity (AEAD MAC) | yes |
 * | Server authentication | yes — via X.509 cert signed by trusted CA |
 * | Client authentication | yes — via X.509 cert signed by trusted CA |
 * | Replay protection | yes — TLS sequence numbers |
 * | Forward secrecy | yes — ECDHE key exchange |
 * | Key material in source | no |
 *
 * ## Certificate setup
 *
 * Both server and clients need three files each:
 *
 * | File | Contents |
 * |---|---|
 * | CA certificate | PEM — the trusted root used to verify the peer |
 * | Own certificate | PEM — signed by the CA, CN = the node's identity |
 * | Own private key | PEM — kept secret; chmod 600 |
 *
 * A minimal setup using the OpenSSL command-line tool:
 *
 * ```sh
 * # Generate a local CA (do once)
 * openssl ecparam -name prime256v1 -genkey -noout -out ca.key
 * openssl req -new -x509 -key ca.key -sha256 -days 3650 \
 *         -out ca.crt -subj "/CN=MyCA" \
 *         -addext "basicConstraints=CA:TRUE"
 *
 * # Generate a server cert (do once per server)
 * openssl ecparam -name prime256v1 -genkey -noout -out server.key
 * openssl req -new -key server.key -out server.csr -subj "/CN=server"
 * openssl x509 -req -in server.csr -CA ca.crt -CAkey ca.key \
 *         -CAcreateserial -days 3650 -sha256 -out server.crt
 *
 * # Generate a client cert (do once per client identity)
 * openssl ecparam -name prime256v1 -genkey -noout -out alice.key
 * openssl req -new -key alice.key -out alice.csr -subj "/CN=alice"
 * openssl x509 -req -in alice.csr -CA ca.crt -CAkey ca.key \
 *         -CAcreateserial -days 3650 -sha256 -out alice.crt
 * ```
 *
 * Store keys with `chmod 600`; CA and certificates may be world-readable.
 *
 * ## Server setup
 *
 * The server loads its own cert/key and the CA cert it will use to verify
 * clients.  Every connecting client must present a cert signed by that CA;
 * connections with an unrecognised or self-signed cert are rejected before
 * any payload is exchanged.  After a successful handshake,
 * `getConnectedIdentity()` returns the Common Name from the client's cert.
 *
 * #### Specify cert paths and create the server
 * \snippet TCPSocketsExample.cxx cert paths
 * \snippet TCPSocketsExample.cxx create server
 *
 * #### Wait for an authenticated client and read its identity
 * \snippet TCPSocketsExample.cxx wait for client
 *
 * #### Echo incoming messages back to the client
 * \snippet TCPSocketsExample.cxx echo loop
 *
 * ## Client setup
 *
 * The client loads its own cert/key and the CA cert it uses to verify the
 * server.  The TLS handshake is performed in the constructor; if it fails
 * the constructor calls `exit()`.
 *
 * #### Connect with a client certificate
 * \snippet TCPSocketsExample.cxx create client
 *
 * #### Send a message and read the echo
 * \snippet TCPSocketsExample.cxx send and receive
 *
 * ## Multi-client sequential use
 *
 * After a client disconnects (TLS close_notify), `ServerSocket::get()`
 * returns 0 and resets the ready flag.  The server calls `isReady()` again
 * to accept the next client — no restart required.
 *
 * \snippet TCPSocketsExample.cxx alice session
 * \snippet TCPSocketsExample.cxx bob session
 *
 * ## Limitations
 *
 * - Only one client is active at a time; the server is single-threaded.
 * - OpenSSL headers are excluded from this public header; TLS state is
 *   stored as opaque `void*` members so consumers need not link OpenSSL.
 * - Certificate revocation (CRL/OCSP) is not implemented.
 */

#include "SocketBase.hxx"

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <memory>
#include <string>
#include <iostream>

namespace SoDa {
  namespace TCP {

    class ServerSocket;
    typedef std::shared_ptr<SoDa::TCP::ServerSocket> ServerSocketPtr;

    class ClientSocket;
    typedef std::shared_ptr<SoDa::TCP::ClientSocket> ClientSocketPtr;


    /**
     * @class NetSocket
     * @brief Shared base holding TLS state and the put/get helpers used by
     *        ServerSocket and ClientSocket.
     *
     * OpenSSL SSL* and SSL_CTX* objects are stored as @c void* to keep
     * OpenSSL headers out of this public header.
     */
    class NetSocket : public SocketBase {
    protected:
      int server_socket; ///< listening fd (ServerSocket only; -1 on ClientSocket)
      int conn_socket;   ///< connected fd after accept() / connect()

      void *ssl; ///< opaque SSL*     — valid after handshake
      void *ctx; ///< opaque SSL_CTX* — valid after constructor

      /**
       * @brief Build a TLS server context that requires and verifies a client
       *        certificate signed by the given CA.
       * @param ca_cert_path     PEM file of the trusted CA certificate.
       * @param server_cert_path PEM file of this server's certificate.
       * @param server_key_path  PEM file of this server's private key.
       * @return opaque SSL_CTX*, or nullptr on failure.
       */
      void *makeServerCTX(const std::string & ca_cert_path,
                          const std::string & server_cert_path,
                          const std::string & server_key_path);

      /**
       * @brief Build a TLS client context that presents a client certificate
       *        and verifies the server certificate against the given CA.
       * @param ca_cert_path     PEM file of the trusted CA certificate.
       * @param client_cert_path PEM file of this client's certificate.
       * @param client_key_path  PEM file of this client's private key.
       * @return opaque SSL_CTX*, or nullptr on failure.
       */
      void *makeClientCTX(const std::string & ca_cert_path,
                          const std::string & client_cert_path,
                          const std::string & client_key_path);

      /**
       * @brief Extract the Common Name from the peer's X.509 certificate.
       *
       * Valid only after a successful TLS handshake.  Returns an empty string
       * if no peer certificate is available.
       */
      std::string getPeerCN();

      /**
       * @brief Write @p size bytes through the TLS session.
       * @return 0 on success, -1 on error.
       */
      int tlsPut(const void *ptr, unsigned int size, bool len_prefix);

      /**
       * @brief Read bytes through the TLS session.
       * @return bytes copied into @p ptr, or ≤0 on error / connection close.
       */
      int tlsGet(void *ptr, unsigned int size, bool len_prefix);
    };


    /**
     * @class ServerSocket
     * @brief TLS mutual-auth TCP server; identifies each client by cert CN.
     *
     * The server loads a CA certificate and its own cert/key at construction.
     * Every connecting client must present a certificate signed by that CA.
     * After a successful handshake, `getConnectedIdentity()` returns the
     * client's Common Name.  Only one client is active at a time; after a
     * client disconnects `isReady()` accepts the next one.
     *
     * @code
     * auto srv = SoDa::TCP::ServerSocket::make(
     *     "0.0.0.0", 9000, "ca.crt", "server.crt", "server.key");
     * while (!srv->isReady()) { usleep(10000); }
     * std::cerr << "Connected: " << srv->getConnectedIdentity() << "\n";
     * @endcode
     */
    class ServerSocket : public NetSocket {
    public:
      /**
       * @brief Bind, listen, and load server TLS credentials.
       *
       * @param addr            bind address — empty string or "*" means INADDR_ANY.
       * @param port_num        TCP port to listen on.
       * @param ca_cert_path    PEM file of the CA used to verify client certs.
       * @param server_cert_path PEM file of this server's certificate.
       * @param server_key_path PEM file of this server's private key (chmod 600).
       */
      ServerSocket(const std::string & addr, size_t port_num,
                   const std::string & ca_cert_path,
                   const std::string & server_cert_path,
                   const std::string & server_key_path);

      ~ServerSocket();

      static ServerSocketPtr make(const std::string & addr, size_t port_num,
                                  const std::string & ca_cert_path,
                                  const std::string & server_cert_path,
                                  const std::string & server_key_path) {
        return std::make_shared<ServerSocket>(addr, port_num,
                                             ca_cert_path,
                                             server_cert_path,
                                             server_key_path);
      }

      /**
       * @brief Non-blocking check for an authenticated client.
       *
       * Attempts a non-blocking `accept()`.  If a TCP connection is pending,
       * performs a blocking TLS handshake.  The handshake verifies the client
       * certificate against the CA; any cert not signed by the trusted CA
       * causes the handshake to fail and the connection to be closed.
       *
       * @return true once a TLS-authenticated client is connected.
       */
      bool isReady(size_t required_len = 32) override;

      /**
       * @brief Return the Common Name of the currently-connected client's cert.
       *
       * Valid only after `isReady()` returns true.  Returns an empty string
       * if no client is connected.
       */
      const std::string & getConnectedIdentity() const { return connected_identity; }

      int get(void *ptr, unsigned int size, bool len_prefix = true) override;
      int put(const void *ptr, unsigned int size, bool len_prefix = true) override;

    private:
      bool ready;
      std::string connected_identity; ///< CN from the current client's cert
    };


    /**
     * @class ClientSocket
     * @brief TLS mutual-auth TCP client.
     *
     * Presents a client certificate during the TLS handshake and verifies the
     * server certificate against the trusted CA.  The handshake is performed
     * in the constructor; if it fails (cert not trusted, server unreachable)
     * the constructor calls `exit()`.
     *
     * @code
     * auto cli = SoDa::TCP::ClientSocket::make(
     *     "192.168.1.10", 9000, "ca.crt", "alice.crt", "alice.key");
     * cli->put(buf, len);
     * @endcode
     */
    class ClientSocket : public NetSocket {
    public:
      /**
       * @brief Connect and perform mutual TLS authentication.
       *
       * Retries `connect()` up to @p startup_timeout_count times (1-second
       * sleep between attempts) before giving up.  The TLS handshake is
       * performed once; failure calls `exit()`.
       *
       * @param addr                  server hostname or dotted-decimal IP.
       * @param port_num              server TCP port.
       * @param ca_cert_path          PEM file of the CA used to verify the server.
       * @param client_cert_path      PEM file of this client's certificate.
       * @param client_key_path       PEM file of this client's private key.
       * @param startup_timeout_count connect-retry limit (default 1).
       */
      ClientSocket(const std::string & addr, size_t port_num,
                   const std::string & ca_cert_path,
                   const std::string & client_cert_path,
                   const std::string & client_key_path,
                   int startup_timeout_count = 1);

      ~ClientSocket();

      static ClientSocketPtr make(const std::string & addr, size_t port_num,
                                  const std::string & ca_cert_path,
                                  const std::string & client_cert_path,
                                  const std::string & client_key_path,
                                  int startup_timeout_count = 1) {
        return std::make_shared<ClientSocket>(addr, port_num,
                                             ca_cert_path,
                                             client_cert_path,
                                             client_key_path,
                                             startup_timeout_count);
      }

      int get(void *ptr, unsigned int size, bool len_prefix = true) override;
      int put(const void *ptr, unsigned int size, bool len_prefix = true) override;
    };

  } // namespace TCP
} // namespace SoDa
