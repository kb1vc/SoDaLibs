/*
 *  BSD 2-Clause License
 *
 *  Copyright (c) 2026, kb1vc
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice, this
 *     list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// Verify that an FFT allocation that cannot be satisfied throws
// SoDa::FFT::AllocFailed rather than crashing with a null-pointer deref.
//
// OOM is hard to trigger portably because Linux overcommit lets huge
// allocations succeed on paper. So we fork a child, cap its RLIMIT_AS a few
// megabytes above its current virtual-memory usage, then ask for an FFT that
// needs far more than that. fftwf_malloc goes through mmap/sbrk, both of
// which honour RLIMIT_AS, so NULL is returned deterministically and
// safe_fftwf_alloc throws.

#include "FFT.hxx"
#include <SoDa/Format.hxx>

#include <fstream>
#include <iostream>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

// Current virtual-memory size in bytes, read from /proc/self/statm. First
// field is the total number of pages mapped into our address space.
static size_t currentVirtualBytes() {
  std::ifstream statm("/proc/self/statm");
  size_t pages = 0;
  statm >> pages;
  long page_size = sysconf(_SC_PAGESIZE);
  return pages * (size_t)page_size;
}

// Child process: tighten RLIMIT_AS, then try to construct an oversized FFT.
// Exit codes mirror the outcome so the parent can tell pass from fail.
static void childRunCtorCase() {
  size_t cur = currentVirtualBytes();
  size_t headroom = 32u * 1024u * 1024u; // 32 MiB above current VM

  struct rlimit rl;
  if (getrlimit(RLIMIT_AS, &rl) != 0) _exit(10);
  rl.rlim_cur = cur + headroom;
  if (setrlimit(RLIMIT_AS, &rl) != 0) _exit(11);

  // Ask for 2^24 complex elements = 16 M * 8 B = 128 MiB per buffer. The
  // ctor allocates two of these, so ~256 MiB -- well above the 32 MiB cap.
  const unsigned int len = 1u << 24;

  try {
    auto fft = SoDa::FFT::make(len);
    // Reached only if fftwf_malloc somehow succeeded despite RLIMIT_AS.
    _exit(12);
  } catch (SoDa::FFT::AllocFailed & e) {
    // Expected. Avoid cerr allocation here (we are near the AS cap); use
    // write(2) directly on a stack buffer.
    const char *ok = "CAUGHT AllocFailed (ctor)\n";
    ssize_t _r = write(2, ok, 26); (void)_r;
    _exit(0);
  } catch (std::exception & e) {
    const char *msg = "FAIL: wrong exception type in ctor\n";
    ssize_t _r = write(2, msg, 34); (void)_r;
    _exit(13);
  } catch (...) {
    const char *msg = "FAIL: unknown exception in ctor\n";
    ssize_t _r = write(2, msg, 31); (void)_r;
    _exit(14);
  }
}

// Parent: fork, run child, interpret exit status.
static bool runCtorCase() {
  pid_t pid = fork();
  if (pid < 0) {
    std::cerr << "FAIL: fork() failed\n";
    return false;
  }
  if (pid == 0) {
    childRunCtorCase();
    _exit(99); // unreachable
  }

  int status = 0;
  if (waitpid(pid, &status, 0) < 0) {
    std::cerr << "FAIL: waitpid() failed\n";
    return false;
  }

  if (WIFSIGNALED(status)) {
    std::cerr << SoDa::Format("FAIL: ctor child died from signal %0 (likely null-deref -- AllocFailed not thrown)\n")
      .addI(WTERMSIG(status));
    return false;
  }
  if (!WIFEXITED(status)) {
    std::cerr << "FAIL: ctor child exited abnormally\n";
    return false;
  }
  int rc = WEXITSTATUS(status);
  if (rc == 0) return true;

  const char *why = "unknown";
  switch (rc) {
    case 10: why = "getrlimit failed";                          break;
    case 11: why = "setrlimit failed";                          break;
    case 12: why = "ctor returned normally (no throw)";         break;
    case 13: why = "ctor threw wrong exception type";           break;
    case 14: why = "ctor threw unknown exception";              break;
  }
  std::cerr << SoDa::Format("FAIL: ctor child exited with code %0 (%1)\n")
    .addI(rc)
    .addS(why);
  return false;
}

int main() {
  bool ok = runCtorCase();
  if (ok) {
    std::cout << "PASSED\n";
    return 0;
  }
  std::cout << "FAILED\n";
  return 1;
}
