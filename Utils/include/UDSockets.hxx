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
 * @file UDSockets.hxx
 * @author Matthew H. Reilly (kb1vc)
 * @date June 3, 2026
 * @author Claude Sonnet 4.6 (Sparky) -- co-author
 *
 * @brief Unix domain stream sockets matching the SoDa::SocketBase interface.
 */

#pragma once

/**
 * @page SoDa_UD SoDa::UD — Unix domain stream sockets
 *
 * ## Overview
 *
 * `SoDa::UD` provides stream sockets over Unix domain (AF_UNIX) paths.
 * Communication is local to the machine; there is no network overhead and
 * no authentication layer.  Use `SoDa::TCP` if you need encryption or
 * mutual authentication.
 *
 * Both the server and client sockets are **non-blocking**.  `get()` returns
 * 0 immediately when no data is available (EAGAIN) rather than blocking.
 * Callers must poll in a loop.
 *
 * ## Non-blocking get() pattern
 *
 * The recommended helper for waiting on a reply:
 * \snippet UDSocketsExample.cxx pollGet
 *
 * ## Server setup
 *
 * The server binds to a filesystem path, listens, and accepts one client at
 * a time.  `isReady()` returns true as soon as `accept()` succeeds (even
 * before data arrives) and returns false when the client closes the
 * connection (detected via `MSG_PEEK` returning 0).
 *
 * #### Create the server socket
 * \snippet UDSocketsExample.cxx create server
 *
 * #### Wait for a client to connect
 * \snippet UDSocketsExample.cxx wait for client
 *
 * #### Read jobs and send replies
 * \snippet UDSocketsExample.cxx serve jobs
 *
 * #### Detect client disconnect before accepting the next one
 * \snippet UDSocketsExample.cxx detect disconnect
 *
 * ## Client setup
 *
 * The client connects to the same filesystem path.  `startup_timeout_count`
 * controls how many 1-second retries are attempted before giving up.
 *
 * #### Connect to the server
 * \snippet UDSocketsExample.cxx create client
 *
 * #### Submit a job and wait for the result
 * \snippet UDSocketsExample.cxx submit job
 *
 * ## Multi-client sequential use
 *
 * After a client disconnects, `isReady()` detects the EOF via `MSG_PEEK` and
 * resets the ready flag.  The server then calls `isReady()` again to accept
 * the next client — no restart required.  Poll `isReady()` until it returns
 * false to confirm the disconnect before the next client connects.
 *
 * #### Two sequential client sessions
 * \snippet UDSocketsExample.cxx first dispatcher
 * \snippet UDSocketsExample.cxx second dispatcher
 *
 * ## Limitations
 *
 * - No authentication or encryption — Unix filesystem permissions are the
 *   only access control.
 * - `get()` cannot distinguish EAGAIN (no data yet) from connection close
 *   (EOF); both return 0.  Callers that need to detect EOF must either
 *   use a timeout in their poll loop or call `isReady()` separately.
 * - Only one client is active at a time.
 */

#include "SocketBase.hxx"

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/un.h>
#include <unistd.h>

#include <memory>
#include <string>
#include <iostream>

namespace SoDa {
  /**
   * @brief Unix domain (AF_UNIX) socket classes.
   *
   * See @ref SoDa_UD for a full usage guide and examples.
   */
  namespace UD {

    class ServerSocket;
    typedef std::shared_ptr<SoDa::UD::ServerSocket> ServerSocketPtr;

    class ClientSocket;
    typedef std::shared_ptr<SoDa::UD::ClientSocket> ClientSocketPtr;

    /**
     * @class NetSocket
     * @brief Shared base providing non-blocking `put()` and `get()` over a
     *        connected Unix domain socket.
     *
     * Both sockets (the accepted `conn_socket` on the server and the connected
     * socket on the client) are set to `O_NONBLOCK`.  `get()` returns 0 when
     * no data is available (EAGAIN); callers must poll.  `put()` retries on
     * EAGAIN until all bytes are written.
     */
    class NetSocket : public SocketBase {
    public:
      NetSocket() : SocketBase() {
        timeout.tv_sec = 0;
        timeout.tv_usec = 5;
      }

      /**
       * @brief Write @p size bytes to the connected socket.
       *
       * If @p len_prefix is true a 4-byte little-endian length field is
       * prepended (the same wire format used by `get()`).  The call retries
       * internally on EAGAIN until all bytes are sent.
       *
       * @param ptr   pointer to the data to send.
       * @param size  number of bytes to send.
       * @param len_prefix  if true, prepend a 4-byte length prefix.
       * @return 0 on success, negative on error.
       */
      int put(const void * ptr, unsigned int size, bool len_prefix = true);

      /**
       * @brief Read up to @p size bytes from the connected socket.
       *
       * If @p len_prefix is true the actual byte count is read from the first
       * 4 bytes of the message; surplus bytes beyond @p size are drained.
       *
       * Because the socket is non-blocking, this call returns 0 immediately
       * when no data is available (EAGAIN).  It also returns 0 on connection
       * close (EOF); callers cannot distinguish the two cases from the return
       * value alone — use `isReady()` or a timed poll loop.
       *
       * @param ptr   buffer to fill.
       * @param size  capacity of the buffer in bytes.
       * @param len_prefix  if true, read the length prefix to determine the
       *                    actual message size.
       * @return number of bytes written into @p ptr, or 0 if no data is
       *         available or the connection is closed, or negative on error.
       */
      int get(void * ptr, unsigned int size, bool len_prefix = true);

      int server_socket; ///< listening fd (ServerSocket only)
      int conn_socket;   ///< connected fd after accept() / connect()
      int portnum;       ///< unused; retained for ABI compatibility

      struct sockaddr_un server_address; ///< address used by bind() / connect()
      struct sockaddr_un client_address; ///< address filled by accept()

      struct timeval timeout; ///< reserved; not currently used by get()/put()

    private:
      int loopWrite(int fd, const void * ptr, unsigned int nbytes);
    };


    /**
     * @class ServerSocket
     * @brief Unix domain stream server; accepts one client at a time.
     *
     * Binds to @p path, listens, and accepts connections via `isReady()`.
     * The socket file is created at construction and removed at destruction.
     *
     * `put()` and `get()` are overridden to gate writes on the ready state
     * and to reset ready on I/O errors.  See @ref SoDa_UD for the full
     * usage pattern including multi-client sequential use.
     *
     * @code
     * auto srv = SoDa::UD::ServerSocket::make("/tmp/my.sock");
     * while (!srv->isReady()) usleep(5000);   // wait for client
     * char buf[256];
     * int n;
     * while ((n = srv->get(buf, sizeof(buf))) > 0) { // poll for data
     *   srv->put(buf, n);                             // echo back
     * }
     * @endcode
     */
    class ServerSocket : public NetSocket {
    public:
      /**
       * @brief Bind to @p path and begin listening.
       *
       * Any existing socket file at @p path is unlinked first.  The listening
       * socket is set non-blocking so that `isReady()` never blocks.
       *
       * @param path  filesystem path for the Unix domain socket file.
       */
      ServerSocket(const std::string & path);

      /**
       * @brief Close sockets and unlink the socket file.
       */
      ~ServerSocket() {
        close(conn_socket);
        close(server_socket);
        unlink(mailbox_pathname.c_str());
        std::cerr << "Closing server socket [" << mailbox_pathname << "]\n";
      }

      /**
       * @brief Factory: construct and return a shared_ptr<ServerSocket>.
       * @param path  filesystem path for the Unix domain socket file.
       */
      static ServerSocketPtr make(const std::string & path) {
        return std::shared_ptr<ServerSocket>(new ServerSocket(path));
      }

      /**
       * @brief Check whether an authenticated client is connected and has
       *        data available.
       *
       * If not yet connected, attempts a non-blocking `accept()`.  If a
       * client is already connected, probes the socket with `MSG_PEEK |
       * MSG_DONTWAIT` to detect a clean disconnect (returns false and resets
       * ready when `recv()` returns 0 / EOF).
       *
       * @param required_len  passed to the internal `recv()` peek; ignored in
       *                      practice since peek returns on the first byte.
       * @return true if a client is connected (ready for I/O); false otherwise.
       */
      bool isReady(size_t required_len = 32);

      /**
       * @brief Read from the connected client; resets ready on error.
       *
       * Delegates to `NetSocket::get()` and sets `ready = false` if the
       * return value is negative (hard error).  Returns 0 for both EAGAIN and
       * connection close — poll with `isReady()` to distinguish them.
       */
      int get(void *ptr, unsigned int size, bool len_prefix = true) {
        int rv = NetSocket::get(ptr, size, len_prefix);
        if(rv < 0) ready = false;
        return rv;
      }

      /**
       * @brief Write to the connected client; calls `isReady()` if not ready.
       *
       * Returns 0 without sending if no client is connected and `isReady()`
       * does not find one.  Sets `ready = false` on write error.
       */
      int put(const void *ptr, unsigned int size, bool len_prefix = true) {
        if(!ready && !isReady()) {
          return 0;
        }
        int rv = NetSocket::put(ptr, size, len_prefix);
        if(rv < 0) ready = false;
        return rv;
      }

      /**
       * @brief Enable or disable verbose debug output from `isReady()`.
       * @param v  true to enable, false to disable.
       */
      void setDebug(bool v) {
        debug = v;
      }

    private:
      bool debug;
      bool ready;
      std::string mailbox_pathname;
    };


    /**
     * @class ClientSocket
     * @brief Unix domain stream client.
     *
     * Connects to a @c ServerSocket at the given filesystem path.  The
     * constructor retries `connect()` up to @p startup_timeout_count times
     * (1-second sleep between attempts) and calls `exit()` if all attempts
     * fail.
     *
     * Both `put()` and `get()` are inherited from `NetSocket`.  The socket is
     * set non-blocking; see @ref SoDa_UD for the polling pattern.
     *
     * @code
     * auto cli = SoDa::UD::ClientSocket::make("/tmp/my.sock");
     * cli->put("hello", 6);
     * char resp[64];
     * int n;
     * do { n = cli->get(resp, sizeof(resp)); } while (n == 0);
     * @endcode
     */
    class ClientSocket : public NetSocket {
    public:
      /**
       * @brief Connect to the server at @p path.
       *
       * @param path                  filesystem path of the server socket.
       * @param startup_timeout_count number of 1-second connect() retries
       *                              before calling `exit()` (default 1).
       */
      ClientSocket(const std::string & path, int startup_timeout_count = 1);

      /** @brief Close the connected socket. */
      ~ClientSocket() {
        close(conn_socket);
      }

      /**
       * @brief Factory: construct and return a shared_ptr<ClientSocket>.
       * @param path                  filesystem path of the server socket.
       * @param startup_timeout_count connect-retry limit (default 1).
       */
      static ClientSocketPtr make(const std::string & path,
                                  int startup_timeout_count = 1) {
        return std::shared_ptr<ClientSocket>(new ClientSocket(path, startup_timeout_count));
      }

    private:
      struct hostent * server;
      std::string mailbox_pathname;
    };
  }
}
