/*
BSD 2-Clause License

Copyright (c) 2021, 2022, 2025 Matt Reilly - kb1vc
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
 * @file SafeOutTest.cxx
 * @author Matt Reilly (kb1vc)
 * @date June 2, 2026
 * @author Claude Sonnet 4.6 (Sparky) -- co-author
 */

#include "../include/SafeOut.hxx"
#include "../include/Options.hxx"

#include <iostream>
#include <sstream>
#include <thread>
#include <vector>
#include <string>
#include <regex>

static bool global_pass = true;

static void check(bool cond, const std::string & msg) {
  if (!cond) {
    std::cerr << "FAIL: " << msg << "\n";
    global_pass = false;
  }
}

// Each thread writes complete single-string lines via one operator<< call.
// If the mutex works, no two writes interleave within a single call, so every
// line in the output must exactly match our pattern.
static void writerThread(SoDa::SafeOut & out, int thread_id, int num_messages) {
  for (int i = 0; i < num_messages; i++) {
    out << (std::string("T") + std::to_string(thread_id) +
            " M" + std::to_string(i) + "\n");
  }
}

static void testThreadSafety(int num_threads, int num_messages) {
  auto threshold = std::make_shared<SoDa::SafeOut::Urgency>(SoDa::SafeOut::TRIVIAL);
  std::ostringstream oss;
  SoDa::SafeOut safeout(oss, threshold, SoDa::SafeOut::TRIVIAL);

  std::vector<std::thread> threads;
  threads.reserve(num_threads);
  for (int i = 0; i < num_threads; i++) {
    threads.emplace_back(writerThread, std::ref(safeout), i, num_messages);
  }
  for (auto & t : threads) t.join();

  // Every line must be exactly "T<N> M<M>" — any deviation means interleaving.
  std::istringstream reader(oss.str());
  std::string line;
  int line_count = 0;
  std::regex line_pat("T[0-9]+ M[0-9]+");

  while (std::getline(reader, line)) {
    line_count++;
    check(std::regex_match(line, line_pat),
          std::string("Garbled line from concurrent writes: [") + line + "]");
  }

  int expected = num_threads * num_messages;
  check(line_count == expected,
        std::string("Expected ") + std::to_string(expected) +
        " lines, got " + std::to_string(line_count));
}

static void testUrgencyFiltering() {
  auto threshold = std::make_shared<SoDa::SafeOut::Urgency>(SoDa::SafeOut::PROBLEMATIC);
  std::ostringstream oss;

  // WORRISOME(2) <= PROBLEMATIC(3): should print
  SoDa::SafeOut low_out(oss, threshold, SoDa::SafeOut::WORRISOME);
  // INTERESTING(4) > PROBLEMATIC(3): should be suppressed
  SoDa::SafeOut high_out(oss, threshold, SoDa::SafeOut::INTERESTING);

  low_out << std::string("visible\n");
  high_out << std::string("suppressed\n");

  std::string output = oss.str();
  check(output.find("visible") != std::string::npos,
        "WORRISOME message should appear when threshold is PROBLEMATIC");
  check(output.find("suppressed") == std::string::npos,
        "INTERESTING message should be suppressed when threshold is PROBLEMATIC");

  // Raise threshold to TRIVIAL: INTERESTING should now print
  *threshold = SoDa::SafeOut::TRIVIAL;
  high_out << std::string("now_visible\n");

  output = oss.str();
  check(output.find("now_visible") != std::string::npos,
        "INTERESTING message should appear after threshold raised to TRIVIAL");

  // Lower threshold to NEVER: nothing should print
  *threshold = SoDa::SafeOut::NEVER;
  low_out << std::string("silenced\n");

  output = oss.str();
  check(output.find("silenced") == std::string::npos,
        "Nothing should print when threshold is NEVER");
}

static void testSetUrgency() {
  auto threshold = std::make_shared<SoDa::SafeOut::Urgency>(SoDa::SafeOut::PROBLEMATIC);
  std::ostringstream oss;
  // Start at WORRISOME(2) <= PROBLEMATIC(3): prints
  SoDa::SafeOut out(oss, threshold, SoDa::SafeOut::WORRISOME);

  out << std::string("before\n");
  out.setUrgency(SoDa::SafeOut::INTERESTING);  // 4 > 3: now suppressed
  out << std::string("after\n");

  std::string output = oss.str();
  check(output.find("before") != std::string::npos,
        "Message before setUrgency should appear");
  check(output.find("after") == std::string::npos,
        "Message after setUrgency to INTERESTING should be suppressed");

  // Raise select back to DIRE(1): prints again
  out.setUrgency(SoDa::SafeOut::DIRE);
  out << std::string("restored\n");

  output = oss.str();
  check(output.find("restored") != std::string::npos,
        "Message after setUrgency back to DIRE should appear");
}

static void testTypeOperators() {
  auto threshold = std::make_shared<SoDa::SafeOut::Urgency>(SoDa::SafeOut::TRIVIAL);
  std::ostringstream oss;
  SoDa::SafeOut out(oss, threshold, SoDa::SafeOut::TRIVIAL);

  out << (unsigned long)12345UL;
  out << std::string(" ");
  out << (long)-67L;
  out << std::string(" ");
  out << 1.5f;
  out << std::string(" ");
  out << true;
  out << std::string(" ");
  out << false;

  std::string output = oss.str();
  check(output.find("12345") != std::string::npos,  "unsigned long operator<<");
  check(output.find("-67")   != std::string::npos,  "long operator<<");
  check(output.find("1.5")   != std::string::npos,  "float operator<<");
  check(output.find("true")  != std::string::npos,  "bool true operator<<");
  check(output.find("false") != std::string::npos,  "bool false operator<<");
}

// Several threads simultaneously raise and lower the shared threshold while
// writers are active. The test just verifies no crash or deadlock occurs; we
// can't predict exactly which messages appear, but none should be garbled.
static void testDynamicThreshold(int num_threads, int num_messages) {
  auto threshold = std::make_shared<SoDa::SafeOut::Urgency>(SoDa::SafeOut::PROBLEMATIC);
  std::ostringstream oss;
  SoDa::SafeOut safeout(oss, threshold, SoDa::SafeOut::TRIVIAL);

  // Threshold-toggling thread
  std::thread toggler([&threshold, num_messages]() {
    SoDa::SafeOut::Urgency levels[] = {
      SoDa::SafeOut::NEVER,
      SoDa::SafeOut::DIRE,
      SoDa::SafeOut::WORRISOME,
      SoDa::SafeOut::PROBLEMATIC,
      SoDa::SafeOut::INTERESTING,
      SoDa::SafeOut::TRIVIAL
    };
    for (int i = 0; i < num_messages; i++) {
      *threshold = levels[i % 6];
    }
  });

  std::vector<std::thread> writers;
  writers.reserve(num_threads);
  for (int i = 0; i < num_threads; i++) {
    writers.emplace_back(writerThread, std::ref(safeout), i, num_messages);
  }

  toggler.join();
  for (auto & t : writers) t.join();

  // Only lines matching the expected pattern should appear (no garbling).
  std::istringstream reader(oss.str());
  std::string line;
  std::regex line_pat("T[0-9]+ M[0-9]+");
  while (std::getline(reader, line)) {
    check(std::regex_match(line, line_pat),
          std::string("Garbled line during dynamic threshold test: [") + line + "]");
  }
}

int main(int argc, char ** argv) {
  SoDa::Options cmd;
  int num_threads, num_messages;
  cmd.add<int>(&num_threads, "threads", 't', 8,    "Number of writer threads")
     .add<int>(&num_messages, "messages", 'm', 500, "Messages per thread");

  if (!cmd.parse(argc, argv)) return 1;

  testThreadSafety(num_threads, num_messages);
  testUrgencyFiltering();
  testSetUrgency();
  testTypeOperators();
  testDynamicThreshold(num_threads, num_messages);

  if (global_pass) {
    std::cerr << "PASS\n";
  } else {
    std::cerr << "FAIL\n";
  }

  return global_pass ? 0 : 1;
}
