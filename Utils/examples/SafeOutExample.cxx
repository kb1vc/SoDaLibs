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
 * @file SafeOutExample.cxx
 * @author Matt Reilly (kb1vc)
 * @date June 3, 2026
 * @author Claude Sonnet 4.6 (Sparky) -- co-author
 *
 * Demonstrates SoDa::SafeOut: urgency-filtered, mutex-protected output
 * from multiple concurrent threads.
 */

#include <SoDa/SafeOut.hxx>
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <string>

// ---------------------------------------------------------------------------
// Part 1: Urgency filtering
// ---------------------------------------------------------------------------

void filteringDemo() {
  std::cerr << "\n=== Urgency Filtering ===\n";

  //! [create threshold]
  // The threshold is a shared_ptr so the owner can change it at any time.
  // All SafeOut objects that hold the same pointer react immediately.
  auto threshold = std::make_shared<SoDa::SafeOut::Urgency>(SoDa::SafeOut::PROBLEMATIC);
  //! [create threshold]

  //! [create streams]
  // dire_out writes only the most urgent messages (DIRE <= PROBLEMATIC → prints).
  SoDa::SafeOut dire_out(std::cerr, threshold, SoDa::SafeOut::DIRE);

  // info_out writes routine status (INTERESTING > PROBLEMATIC → suppressed).
  SoDa::SafeOut info_out(std::cerr, threshold, SoDa::SafeOut::INTERESTING);

  // chatter writes noise (TRIVIAL > PROBLEMATIC → suppressed).
  SoDa::SafeOut chatter(std::cerr, threshold, SoDa::SafeOut::TRIVIAL);
  //! [create streams]

  //! [filtering in action]
  dire_out << std::string("DIRE:       something needs immediate attention  [visible]\n");
  info_out << std::string("INTERESTING: routine status update               [suppressed]\n");
  chatter  << std::string("TRIVIAL:    verbose internal detail               [suppressed]\n");
  //! [filtering in action]

  //! [raise threshold]
  // Open the floodgates — no SafeOut objects need to be touched.
  std::cerr << "Raising threshold to TRIVIAL...\n";
  *threshold = SoDa::SafeOut::TRIVIAL;

  dire_out << std::string("DIRE:       something needs immediate attention  [visible]\n");
  info_out << std::string("INTERESTING: routine status update               [visible]\n");
  chatter  << std::string("TRIVIAL:    verbose internal detail               [visible]\n");
  //! [raise threshold]

  //! [silence all]
  // Setting NEVER silences everything regardless of each stream's urgency_select.
  std::cerr << "Silencing all output (NEVER)...\n";
  *threshold = SoDa::SafeOut::NEVER;

  dire_out << std::string("DIRE:       (this will not appear)\n");
  info_out << std::string("INTERESTING: (this will not appear)\n");
  //! [silence all]

  std::cerr << "(all messages above were suppressed)\n";
}

// ---------------------------------------------------------------------------
// Part 2: setUrgency — changing a stream's own level
// ---------------------------------------------------------------------------

void setUrgencyDemo() {
  std::cerr << "\n=== setUrgency ===\n";

  auto threshold = std::make_shared<SoDa::SafeOut::Urgency>(SoDa::SafeOut::PROBLEMATIC);
  SoDa::SafeOut out(std::cerr, threshold, SoDa::SafeOut::WORRISOME);

  //! [setUrgency]
  out << std::string("WORRISOME: visible at PROBLEMATIC threshold\n");

  // Promote this stream to INTERESTING — now it is above the threshold.
  out.setUrgency(SoDa::SafeOut::INTERESTING);
  out << std::string("INTERESTING: suppressed at PROBLEMATIC threshold\n");

  // Demote back to DIRE — always visible.
  out.setUrgency(SoDa::SafeOut::DIRE);
  out << std::string("DIRE: visible again\n");
  //! [setUrgency]
}

// ---------------------------------------------------------------------------
// Part 3: Thread safety
// ---------------------------------------------------------------------------

//! [worker thread]
// Each worker assembles a complete line in one std::string and passes it in a
// single operator<< call.  Because the mutex covers that entire call, lines
// from different threads never interleave.
void workerThread(SoDa::SafeOut & log, int id, int iterations) {
  for (int i = 0; i < iterations; i++) {
    log << (std::string("[worker ") + std::to_string(id) +
            "] iteration " + std::to_string(i) + "\n");
  }
}
//! [worker thread]

void threadingDemo(int num_threads, int iterations) {
  std::cerr << "\n=== Thread Safety (" << num_threads
            << " threads x " << iterations << " iterations) ===\n";

  //! [shared log]
  auto threshold = std::make_shared<SoDa::SafeOut::Urgency>(SoDa::SafeOut::TRIVIAL);
  // All worker threads share this single SafeOut.  The internal mutex
  // serialises every operator<< call, so no two lines ever interleave.
  SoDa::SafeOut shared_log(std::cerr, threshold, SoDa::SafeOut::TRIVIAL);
  //! [shared log]

  //! [spawn threads]
  std::vector<std::thread> threads;
  threads.reserve(num_threads);
  for (int i = 0; i < num_threads; i++) {
    threads.emplace_back(workerThread, std::ref(shared_log), i, iterations);
  }
  for (auto & t : threads) t.join();
  //! [spawn threads]

  std::cerr << "All " << num_threads << " threads joined cleanly.\n";
}

// ---------------------------------------------------------------------------
// Part 4: Type operators
// ---------------------------------------------------------------------------

void typeDemo() {
  std::cerr << "\n=== Type Operators ===\n";

  auto threshold = std::make_shared<SoDa::SafeOut::Urgency>(SoDa::SafeOut::TRIVIAL);
  SoDa::SafeOut out(std::cerr, threshold, SoDa::SafeOut::TRIVIAL);

  //! [type operators]
  out << std::string("string:        hello\n");
  out << (unsigned long)42UL;   out << std::string("\n");
  out << (long)-7L;             out << std::string("\n");
  out << 3.14f;                 out << std::string("\n");
  out << true;                  out << std::string("\n");
  out << false;                 out << std::string("\n");
  //! [type operators]
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main() {
  filteringDemo();
  setUrgencyDemo();
  threadingDemo(4, 3);
  typeDemo();
  std::cerr << "\nDone.\n";
  return 0;
}
