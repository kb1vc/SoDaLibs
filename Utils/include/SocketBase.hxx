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
#pragma once

#include <memory>
#include <string>
#include <iostream>

namespace SoDa {

  class SocketBase {
  public:
    /**
     * @class SocketBase
     *
     * @brief All SoDa Sockets (UD, TCP, etc...)  must provide put and
     * get and isReady
     */
    SocketBase() { }

    /**
     * @brief put "size" bytes from the buffer pointed to by ptr to the socket.
     *
     * @param ptr a byte pointer to the buffer
     * @param size the number of bytes to transfer
     * @param len_prefix: if true, write "size" as an unsigned int to the
     * first 4 bytes of the outbound packet.
     * @return the number of bytes written to the socket.
     *
     * This call will block until all "size" bytes have been written to the
     * socket. 
     */
    virtual int put(const void * ptr, unsigned int size, bool len_prefix = true) = 0;

    /**
     * @brief put "size" bytes from the buffer pointed to by ptr to the socket.
     *
     * @param ptr a byte pointer to the buffer to be filled. 
     * @param size the number of bytes to read from the socket.
     * @param len_prefix: If true, read the size of the buffer from
     * first 4 bytes of the arriving packet. Use this as the size
     * count, overriding the "size" parameter.
     * @return the number of bytes read from the socket.
     * 
     * This call will block until all "size" bytes have been written to the
     * socket. 
     */
    virtual int get(void * ptr, unsigned int size, bool len_prefix = true) = 0;


    /**
     * @brief Avoid blocking @c put operations by testing for the availability
     * of outbound buffer space. This is an optimization. Returning "true" is
     * always safe.
     *
     * @param required_len the number of bytes we desire to send to the socket
     * @return false if there is not enough space, true otherwise. False should
     * only be returned if we know the outbound buffer size is inadequate.
     * If put is called after isReady returns false, the put call may block.
     * If put is called after isReady returns true, and the Socket implements
     * isReady, the put call will not block. Otherwise the put call may block. 
     */
    virtual bool isReady(size_t required_len = 32) { return true; }
  };

}

