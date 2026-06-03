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
 * @file SafeOut.cxx
 * @author Matt Reilly (kb1vc)
 * @date June 2, 2026
 * @author Claude Sonnet 4.6 (Sparky) -- co-author
 */

#include "SafeOut.hxx"

namespace SoDa {

  std::mutex SafeOut::safeout_mutex;

  SafeOut::SafeOut(UrgencyPtr urgency_threshold, Urgency urgency_select)
    : urgency(urgency_select), urgency_threshold_ptr(urgency_threshold),
      os_ptr(&std::cout) {
  }

  SafeOut::SafeOut(std::ostream & os, UrgencyPtr urgency_threshold, Urgency urgency_select)
    : urgency(urgency_select), urgency_threshold_ptr(urgency_threshold),
      os_ptr(&os) {
  }

  bool SafeOut::shouldWrite() const {
    // Called only while safeout_mutex is held, so urgency and
    // *urgency_threshold_ptr are read under the lock.
    if (!urgency_threshold_ptr) return false;
    return (urgency != NEVER) && (urgency <= *urgency_threshold_ptr);
  }

  SafeOut & SafeOut::operator<<(const std::string & str) {
    std::lock_guard<std::mutex> lock(safeout_mutex);
    if (shouldWrite()) *os_ptr << str;
    return *this;
  }

  SafeOut & SafeOut::operator<<(unsigned long v) {
    std::lock_guard<std::mutex> lock(safeout_mutex);
    if (shouldWrite()) *os_ptr << v;
    return *this;
  }

  SafeOut & SafeOut::operator<<(long v) {
    std::lock_guard<std::mutex> lock(safeout_mutex);
    if (shouldWrite()) *os_ptr << v;
    return *this;
  }

  SafeOut & SafeOut::operator<<(float v) {
    std::lock_guard<std::mutex> lock(safeout_mutex);
    if (shouldWrite()) *os_ptr << v;
    return *this;
  }

  SafeOut & SafeOut::operator<<(bool v) {
    std::lock_guard<std::mutex> lock(safeout_mutex);
    if (shouldWrite()) *os_ptr << (v ? "true" : "false");
    return *this;
  }

  SafeOut & SafeOut::setUrgency(Urgency select) {
    std::lock_guard<std::mutex> lock(safeout_mutex);
    urgency = select;
    return *this;
  }

}
