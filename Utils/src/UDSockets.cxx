/*
  Copyright (c) 2012, 2025, 2026 Matthew H. Reilly (kb1vc)
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

#include "UDSockets.hxx"

#include <iostream>
#include <unistd.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <algorithm>
#include <string.h>
#include <SoDa/Format.hxx>

namespace SoDa {
  // Maximum size of a single framed message. Any peer-supplied length prefix
  // above this is treated as hostile and the read is aborted.
  static constexpr unsigned int max_UD_msg_size = 8u * 1024u * 1024u;

  // Copy `path` into `addr.sun_path`, always NUL-terminated. Throws
  // UD::SocketError if `path` is too long to fit in sun_path (including the
  // trailing NUL).
  static void set_sun_path(struct sockaddr_un & addr, const std::string & path) {
    if (path.size() >= sizeof(addr.sun_path)) {
      throw UD::SocketError(Format("socket path too long (%0 >= %1 bytes): %2")
			    .addU(path.size())
			    .addU(sizeof(addr.sun_path))
			    .addS(path)
			    .str());
    }
    memcpy(addr.sun_path, path.c_str(), path.size());
    addr.sun_path[path.size()] = '\0';
  }

  // Unlink a stale socket file only if it is actually a Unix domain socket
  // and owned by our effective uid. Guards against a symlink race in which an
  // attacker swaps `path` for a victim file (or directory entry) between our
  // check and our bind.
  static void safe_unlink_socket(const std::string & path) {
    struct stat st;
    if (lstat(path.c_str(), &st) < 0) {
      // Nothing there (or no permission to see). bind() will report any real
      // problem.
      return;
    }
    if (!S_ISSOCK(st.st_mode)) {
      std::cerr << Format("UD: refusing to unlink %0: not a socket (mode=%1)\n")
	.addS(path)
	.addU(st.st_mode, 'o');
      return;
    }
    if (st.st_uid != geteuid()) {
      std::cerr << Format("UD: refusing to unlink %0: owned by uid %1 (we are uid %2)\n")
	.addS(path)
	.addU(st.st_uid)
	.addU(geteuid());
      return;
    }
    unlink(path.c_str());
  }
  UD::ServerSocket::ServerSocket(const std::string & path)
  {
    int stat;
    // create the socket.
    server_socket = socket(AF_UNIX, SOCK_STREAM, 0);

    // init the debug flag
    debug = false;

    if(server_socket < 0) {
      throw UD::SocketError(Format("failed to create server socket for [%0]: %1")
			    .addS(path)
			    .addS(strerror(errno))
			    .str());
    }

    int x = fcntl(server_socket, F_GETFL, 0);
    fcntl(server_socket, F_SETFL, x | O_NONBLOCK);

    // setup the server address
    bzero((char*) &server_address, sizeof(server_address));
    server_address.sun_family = AF_UNIX;
    set_sun_path(server_address, path);
    safe_unlink_socket(path);
    int len = sizeof(server_address); // strlen(server_address.sun_path) + sizeof(server_address.sun_family);

    mailbox_pathname = path;

    // now bind it
    if (bind(server_socket, (struct sockaddr *) &server_address, len) < 0) {
      int e = errno;
      close(server_socket);
      throw UD::SocketError(Format("bind failed on [%0]: %1")
			    .addS(path)
			    .addS(strerror(e))
			    .str());
    }

    // now let the world know that we're ready for one and only one connection.
    stat = listen(server_socket, 1);
    if(stat < 0) {
      int e = errno;
      close(server_socket);
      throw UD::SocketError(Format("listen failed on [%0]: %1")
			    .addS(path)
			    .addS(strerror(e))
			    .str());
    }

    // mark the socket as "not ready" for input -- it needs to accept first.
    ready = false;
  }

  UD::ServerSocket::~ServerSocket()
  {
    close(conn_socket);
    close(server_socket);
    // Only unlink the socket file if it still looks like ours -- guards
    // against a late-breaking symlink swap against an unrelated file.
    safe_unlink_socket(mailbox_pathname);
  }

  UD::ClientSocket::ClientSocket(const std::string & path, int startup_timeout_count)
  {
    int retry_count;
    conn_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if(conn_socket < 0) {
      throw UD::SocketError(Format("failed to create client socket on [%0]: %1")
			    .addS(path)
			    .addS(strerror(errno))
			    .str());
    }

    mailbox_pathname = path;

    bzero((char*) &server_address, sizeof(server_address));
    server_address.sun_family = AF_UNIX;
    set_sun_path(server_address, path);
    int len = sizeof(server_address);

    int stat;
    for(retry_count = 0; retry_count < startup_timeout_count; retry_count++) {
      stat = connect(conn_socket, (struct sockaddr *) &server_address, len);
      if(stat >= 0) break;
      else {
	// we should wait a little while before we give up.
	sleep(1);
      }
    }

    if(stat < 0) {
      int e = errno;
      close(conn_socket);
      throw UD::SocketError(Format("client couldn't connect to UNIX socket [%0]: %1")
			    .addS(path)
			    .addS(strerror(e))
			    .str());
    }

    int x = fcntl(conn_socket, F_GETFL, 0);
    fcntl(conn_socket, F_SETFL, x | O_NONBLOCK);
  }

  bool UD::ServerSocket::isReady(size_t required_len)
  {
    // Clamp required_len to the local scratch buffer. The kernel is told to
    // peek up to required_len bytes; without this clamp a caller supplying a
    // value greater than sizeof(buf) would smash the stack.
    char buf[32];
    if (required_len > sizeof(buf)) required_len = sizeof(buf);

    if(ready) {
      // is the socket still open?
      if(recv(conn_socket, buf, required_len, MSG_PEEK | MSG_DONTWAIT) == 0) {
	// connection has been closed. -- though this never seems to trigger
	close(conn_socket);
	ready = false;
	return false;
      }
      return true;
    }
    else {
      socklen_t ca_len = sizeof(client_address);
      // note that we've set the server_socket to non-block, so if nobody is here,
      // we should get an EAGAIN or EWOULDBLOCK.
      int ns = accept(server_socket, (struct sockaddr *) & client_address, &ca_len);

      if(ns < 0) {
	ready = false;
      }
      else {
	conn_socket = ns;
	int x = fcntl(conn_socket, F_GETFL, 0);
	int stat = fcntl(conn_socket, F_SETFL, x | O_NONBLOCK);
	ready = (recv(conn_socket, buf, required_len, MSG_PEEK | MSG_DONTWAIT) != 0);
      }
    }
    return ready;
  }

  int UD::NetSocket::loopWrite(int fd, const void * ptr, unsigned int nbytes)
  {
    char * bptr = (char*) ptr;
    int left = nbytes;
    int stat;
    while(left > 0) {
      stat = write(fd, bptr, left);
      if(stat < 0) {
	if((errno == EWOULDBLOCK) || (errno == EAGAIN)) {
	  continue; 
	}
	else {
	  return stat; 
	}
      }
      else {
	left -= stat;
	bptr += stat; 
      }
    }
    return 0; 
  }

  int UD::NetSocket::put(const void * ptr, unsigned int size, bool len_prefix)
  {
    // we always put a buffer of bytes, preceded by a count of bytes to be sent.
    int stat;
  
    if(len_prefix) {
      stat = loopWrite(conn_socket, &size, sizeof(unsigned int));
      if(stat < 0) return stat; 
    }

    stat = loopWrite(conn_socket, ptr, size);

    return stat; 
  }

  int UD::NetSocket::get(void * ptr, unsigned int size, bool len_prefix)
  {
    int stat;
    size_t rsize = size;
    unsigned int ret_size = 0;
    size_t left = size;

    if(len_prefix) {
      // Wire format is a 32-bit length. Read into an unsigned int (matches
      // put(), which writes sizeof(unsigned int) bytes), then promote to
      // size_t for all subsequent arithmetic.
      unsigned int wire_rsize;
      stat = read(conn_socket, &wire_rsize, sizeof(wire_rsize));
      if(stat <= 0) {
	if((errno == EWOULDBLOCK) || (errno == EAGAIN)) {
	  return 0;
	}
	else {
	  perror("Oops -- UD::NetSocket::get -- ");
	  return stat;
	}
      }
      rsize = wire_rsize;

      // Reject any peer-supplied frame larger than the hard cap. Without this
      // a malicious peer could make us drain gigabytes or overrun the caller's
      // buffer via integer confusion.
      if(rsize > max_UD_msg_size) {
	std::cerr << Format("UD::NetSocket::get: rejecting oversized frame (rsize=%0, max=%1)\n")
	  .addU(rsize)
	  .addU(max_UD_msg_size);
	return -1;
      }

      // Copy no more bytes than the caller's buffer can hold; drain the rest
      // below.
      left = (rsize < size) ? rsize : size;
    }

    char * bptr = (char*) ptr;
    while(left > 0) {
      int ls = read(conn_socket, bptr, left);
      if(ls < 0) {
	if((errno == EWOULDBLOCK) || (errno == EAGAIN)) {
	  continue;
	}
	else {
	  perror("Ooops -- read buffer continued");
	  return ls;
	}
      }
      else {
	left -= ls;
	bptr += ls;
	ret_size += ls;
      }
    }

    if(len_prefix && rsize > size) {
      char dmy[100];
      size_t drain_left = rsize - size;
      while(drain_left != 0) {
	int ls;
	size_t chunk = (drain_left > sizeof(dmy)) ? sizeof(dmy) : drain_left;
	ls = read(conn_socket, dmy, chunk);
	if(ls < 0) {
	  if((errno == EWOULDBLOCK) || (errno == EAGAIN)) {
	    continue;
	  }
	  else {
	    perror("Ooops -- read buffer continued");
	    return ls;
	  }
	}
	drain_left -= (size_t)ls;
      }
    }

    return ret_size;

  }
}
