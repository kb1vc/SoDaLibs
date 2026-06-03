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
 * @file UDSocketsTest.cxx
 * @author Matt Reilly (kb1vc)
 * @date June 3, 2026
 * @author Claude Sonnet 4.6 (Sparky) -- co-author
 *
 * Tests for SoDa::UD::ServerSocket and ClientSocket.
 *
 * Test 1 — basic roundtrip: client sends a message, server echoes it back,
 *          content is verified on both ends.
 *
 * Test 2 — multi-client sequential: two clients connect one after the other.
 *          The server detects the first client's disconnect via isReady()
 *          (MSG_PEEK returns 0 on EOF) and then accepts the second client.
 *          The message from each client is verified.
 *
 * Notes on UD socket behaviour:
 *   - Both server and client sockets are non-blocking; get() returns 0 on
 *     EAGAIN, so callers must poll.
 *   - isReady() on ServerSocket uses MSG_PEEK | MSG_DONTWAIT: it returns
 *     false only when the peer closes the connection (recv() == 0).  It
 *     returns true as soon as accept() succeeds, even before data arrives.
 *   - Disconnect between clients is detected by polling isReady() until it
 *     returns false (EOF seen on MSG_PEEK), then waiting for the next
 *     accept() to succeed.
 */

#include "../include/UDSockets.hxx"
#include "../include/Options.hxx"

#include <iostream>
#include <string>
#include <thread>
#include <atomic>
#include <chrono>
#include <cstring>

#include <unistd.h>

static bool global_pass = true;

static void check(bool cond, const std::string & msg) {
  if (!cond) {
    std::cerr << "FAIL: " << msg << "\n";
    global_pass = false;
  }
}

// Poll get() until bytes arrive or deadline passes.
static int pollGet(SoDa::UD::NetSocket & sock, void * buf, unsigned int size,
                   int timeout_ms = 2000) {
  auto deadline = std::chrono::steady_clock::now()
                + std::chrono::milliseconds(timeout_ms);
  int n = 0;
  while (n == 0 && std::chrono::steady_clock::now() < deadline) {
    n = sock.get(buf, size);
    if (n == 0) usleep(5000);
  }
  return n;
}

// ---------------------------------------------------------------------------
// Test 1: Basic roundtrip
// ---------------------------------------------------------------------------
static void testRoundtrip(const std::string & path) {
  std::atomic<bool> server_up{false};
  std::string server_received;

  std::thread srv_thread([&]() {
    auto srv = SoDa::UD::ServerSocket::make(path);
    server_up = true;

    while (!srv->isReady()) usleep(5000);

    char buf[256] = {};
    int n = pollGet(*srv, buf, sizeof(buf) - 1);
    check(n > 0, "roundtrip: server got bytes");
    server_received = std::string(buf);  // null-terminated; buf[n-1] == '\0'

    srv->put(buf, (unsigned int)n);
  });

  while (!server_up) usleep(5000);
  usleep(20000); // let server reach listen()

  {
    auto cli = SoDa::UD::ClientSocket::make(path, 5);
    const char *msg = "Hello, UD!";
    unsigned int msglen = (unsigned int)strlen(msg) + 1;
    cli->put(msg, msglen);

    char resp[256] = {};
    int n = pollGet(*cli, resp, sizeof(resp) - 1);
    check(n > 0, "roundtrip: client got response");
    check(std::string(resp) == msg, "roundtrip: content matches");
  }

  srv_thread.join();
  check(server_received == "Hello, UD!", "roundtrip: server received correct content");
}

// ---------------------------------------------------------------------------
// Test 2: Multi-client sequential
// ---------------------------------------------------------------------------
// The server accepts two clients back-to-back.  After reading the first
// client's message the server polls isReady() until false (disconnect), then
// waits for the second client.
static void testMultiClient(const std::string & path) {
  std::atomic<bool> server_up{false};
  std::string msg1, msg2;

  std::thread srv_thread([&]() {
    auto srv = SoDa::UD::ServerSocket::make(path);
    server_up = true;

    // --- Client 1 ---
    while (!srv->isReady()) usleep(5000);

    char buf[256] = {};
    int n = pollGet(*srv, buf, sizeof(buf) - 1);
    check(n > 0, "multiClient: server got bytes from client 1");
    msg1 = std::string(buf);

    // Poll isReady() until client 1 disconnects (MSG_PEEK returns 0 on EOF).
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
    while (srv->isReady() && std::chrono::steady_clock::now() < deadline) {
      usleep(5000);
    }
    check(!srv->isReady(), "multiClient: server detected client 1 disconnect");

    // --- Client 2 ---
    deadline = std::chrono::steady_clock::now() + std::chrono::seconds(3);
    while (!srv->isReady() && std::chrono::steady_clock::now() < deadline) {
      usleep(5000);
    }
    check(srv->isReady(), "multiClient: server accepted client 2");

    char buf2[256] = {};
    n = pollGet(*srv, buf2, sizeof(buf2) - 1);
    check(n > 0, "multiClient: server got bytes from client 2");
    msg2 = std::string(buf2);
  });

  while (!server_up) usleep(5000);
  usleep(20000);

  // Client 1
  {
    auto cli = SoDa::UD::ClientSocket::make(path, 5);
    cli->put("from alice", 11);
    // Destructor: close(conn_socket) — server's MSG_PEEK will see EOF.
  }

  // Small gap to let the server detect the disconnect before client 2 connects.
  usleep(50000);

  // Client 2
  {
    auto cli = SoDa::UD::ClientSocket::make(path, 5);
    cli->put("from bob", 9);
  }

  srv_thread.join();

  check(msg1 == "from alice", "multiClient: received correct message from client 1");
  check(msg2 == "from bob",   "multiClient: received correct message from client 2");
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main(int argc, char ** argv) {
  SoDa::Options cmd;
  std::string base_path;
  cmd.add<std::string>(&base_path, "path", 'p', "/tmp/UDSocketsTest",
                       "Base path for Unix domain socket files");
  if (!cmd.parse(argc, argv)) return 1;

  testRoundtrip(base_path + "_roundtrip");
  testMultiClient(base_path + "_multi");

  if (global_pass) {
    std::cerr << "PASS\n";
  } else {
    std::cerr << "FAIL\n";
  }
  return global_pass ? 0 : 1;
}
