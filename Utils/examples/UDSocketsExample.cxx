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
 * @file UDSocketsExample.cxx
 * @author Matt Reilly (kb1vc)
 * @date June 3, 2026
 * @author Claude Sonnet 4.6 (Sparky) -- co-author
 *
 * @brief Demonstrates SoDa::UD::ServerSocket and ClientSocket.
 *
 * This self-contained example uses Unix domain sockets to build a simple
 * work-queue: a dispatcher thread sends jobs to a worker thread over a UD
 * socket, and the worker replies with results.  It shows:
 *
 *  - Creating a server socket bound to a filesystem path.
 *  - Connecting a client to that path.
 *  - The non-blocking get() pattern: both sockets are non-blocking, so
 *    get() returns 0 on EAGAIN and the caller must poll.
 *  - Sequential multi-client use: the server detects a client disconnect
 *    via isReady() / MSG_PEEK and accepts the next client.
 *
 * ## Non-blocking I/O note
 *
 * UD sockets are non-blocking.  get() returns 0 when no data is available
 * yet (EAGAIN), not when the connection is closed.  Always poll get() in a
 * loop rather than assuming a single call will succeed.  The helper
 * pollGet() below shows the recommended pattern.
 */

#include <SoDa/UDSockets.hxx>
#include <SoDa/Options.hxx>

#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <cstring>

#include <unistd.h>

// ---------------------------------------------------------------------------
// Non-blocking get() helper
// ---------------------------------------------------------------------------

//! [pollGet]
static int pollGet(SoDa::UD::NetSocket & sock,
                   void * buf, unsigned int size,
                   int timeout_ms = 3000) {
  auto deadline = std::chrono::steady_clock::now()
                + std::chrono::milliseconds(timeout_ms);
  int n = 0;
  while (n == 0 && std::chrono::steady_clock::now() < deadline) {
    n = sock.get(buf, size);
    if (n == 0) usleep(5000);
  }
  return n;
}
//! [pollGet]

// ---------------------------------------------------------------------------
// Worker (server side)
// ---------------------------------------------------------------------------
// Accepts num_jobs jobs from num_clients clients.  Each job is a string;
// the worker replies with "done: <job>".

static void runWorker(const std::string & path,
                      std::atomic<bool> & ready,
                      int num_clients) {

//! [create server]
  auto srv = SoDa::UD::ServerSocket::make(path);
//! [create server]

  ready = true;

  for (int client = 0; client < num_clients; ++client) {

//! [wait for client]
    while (!srv->isReady()) usleep(5000);
    std::cout << "[worker] dispatcher connected\n";
//! [wait for client]

//! [serve jobs]
    char buf[512];
    int n;
    // 500 ms timeout: reply always arrives in microseconds on loopback;
    // timeout signals the dispatcher has sent its last job and disconnected.
    while ((n = pollGet(*srv, buf, sizeof(buf) - 1, 500)) > 0) {
      buf[n] = '\0';
      std::cout << "[worker] job: " << buf << "\n";

      // Build the reply and send it back.
      std::string reply = std::string("done: ") + buf;
      srv->put(reply.c_str(), (unsigned int)reply.size() + 1);
    }
//! [serve jobs]

    // isReady() returns false once the client closes the connection
    // (MSG_PEEK sees EOF).  Poll until the transition is confirmed so that
    // the next accept() in isReady() starts with a clean state.

//! [detect disconnect]
    auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
    while (srv->isReady() && std::chrono::steady_clock::now() < deadline) {
      usleep(5000);
    }
    std::cout << "[worker] dispatcher disconnected\n";
//! [detect disconnect]
  }
}

// ---------------------------------------------------------------------------
// Dispatcher (client side)
// ---------------------------------------------------------------------------
// Connects to the worker and submits a list of jobs, reading each result
// before sending the next.

static void runDispatcher(const std::string & path,
                          const std::string & name,
                          const std::vector<std::string> & jobs) {

//! [create client]
  auto cli = SoDa::UD::ClientSocket::make(path, 5 /* retry seconds */);
  std::cout << "[" << name << "] connected\n";
//! [create client]

  for (const auto & job : jobs) {

//! [submit job]
    cli->put(job.c_str(), (unsigned int)job.size() + 1);

    char result[512] = {};
    int n = pollGet(*cli, result, sizeof(result) - 1);
    if (n > 0) {
      std::cout << "[" << name << "] result: " << result << "\n";
    } else {
      std::cout << "[" << name << "] timed out waiting for result\n";
    }
//! [submit job]
  }

  // Destructor closes the connection; the worker's pollGet() returns 0.
  std::cout << "[" << name << "] done, disconnecting\n";
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main(int argc, char ** argv) {
  SoDa::Options cmd;
  std::string sock_path;
  cmd.add<std::string>(&sock_path, "path", 'p', "/tmp/UDSocketsExample",
                       "Filesystem path for the Unix domain socket");
  if (!cmd.parse(argc, argv)) return 1;

  std::atomic<bool> worker_ready{false};

  // Start the worker in a background thread.
  std::thread worker_thread(runWorker, sock_path,
                            std::ref(worker_ready),
                            2 /* two dispatchers */);

  while (!worker_ready) usleep(5000);
  usleep(20000); // let server reach listen()

//! [first dispatcher]
  std::cout << "\n--- Dispatcher A ---\n";
  runDispatcher(sock_path, "dispatcher-A",
                { "render frame 1", "render frame 2", "render frame 3" });
//! [first dispatcher]

  usleep(50000); // let worker detect A's disconnect before B connects

//! [second dispatcher]
  std::cout << "\n--- Dispatcher B ---\n";
  runDispatcher(sock_path, "dispatcher-B",
                { "compress archive", "upload result" });
//! [second dispatcher]

  worker_thread.join();

  std::cout << "\nAll jobs complete.\n";
  return 0;
}
