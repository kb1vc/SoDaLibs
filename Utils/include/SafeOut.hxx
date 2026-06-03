#pragma once
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
 * @file SafeOut.hxx
 * @author Matt Reilly (kb1vc)
 * @date June 2, 2026
 * @author Claude Sonnet 4.6 (Sparky) -- co-author
 */

#include <string>
#include <iostream>
#include <memory>
#include <mutex>

namespace SoDa {

/**
 * @page SoDa_SafeOut SoDa::SafeOut — Mutex-protected output stream
 *
 * ## The Problem
 *
 * In a multithreaded application, concurrent writes to `std::cout` or any
 * shared `std::ostream` interleave unpredictably.  Even a single `<<` chain
 * like
 * ```cpp
 * std::cout << "value = " << x << "\n";
 * ```
 * is three separate calls, and another thread can inject output between any
 * of them.  The result is a jumbled mess on the terminal.
 *
 * ## The Solution
 *
 * SoDa::SafeOut wraps an output stream with a static `std::mutex`.  Every
 * individual `operator<<` call acquires the mutex for the duration of that
 * single write, so no two SafeOut writes ever overlap.  To emit a complete
 * line atomically, assemble it into a `std::string` first and write it in
 * one call:
 * ```cpp
 * safeout << (std::string("value = ") + std::to_string(x) + "\n");
 * ```
 *
 * ## Urgency Filtering
 *
 * SafeOut includes a lightweight urgency/verbosity filter so that different
 * parts of a program can share a single run-time verbosity knob without
 * each needing its own `if (verbose) ...` guard.
 *
 * Two values interact at each write:
 *
 * - **urgency_select** — the importance level assigned to *this* SafeOut
 *   instance.  Low numbers are more urgent: DIRE (1) < WORRISOME (2) <
 *   PROBLEMATIC (3) < INTERESTING (4) < TRIVIAL (5).  NEVER (0) suppresses
 *   output unconditionally from this stream.
 *
 * - **urgency_threshold** — a `shared_ptr<Urgency>` whose pointee is the
 *   current run-time verbosity ceiling.  A write is emitted only when
 *   `urgency_select <= *urgency_threshold`.  Because it is a pointer, the
 *   owner can raise or lower verbosity at any time and all SafeOut objects
 *   sharing that pointer respond immediately.
 *
 * ### Thread-safety caveat on threshold updates
 *
 * `SafeOut` reads `*urgency_threshold` under `safeout_mutex`.  However, the
 * *owner's* writes to `*urgency_threshold` (e.g. `*threshold = TRIVIAL`) are
 * not coordinated with that mutex — the owner has no access to it.  This is
 * technically a data race under the C++ memory model, and is therefore
 * formally undefined behaviour.
 *
 * In practice the race is benign on every architecture this code targets:
 * single-word loads and stores of a trivially-copyable enum are naturally
 * atomic on x86-64 and AArch64, so no torn read can occur.  The worst
 * observable effect is that a message near a threshold change is printed or
 * suppressed based on a value that is one assignment old.
 *
 * The formally correct remedy is to change `UrgencyPtr` to
 * `std::shared_ptr<std::atomic<Urgency>>` and update callers accordingly;
 * that is a breaking API change and has been deferred.
 *
 * ### Examples
 *
 * #### Creating a threshold and streams
 * \snippet SafeOutExample.cxx create threshold
 * \snippet SafeOutExample.cxx create streams
 *
 * #### Filtering in action
 * \snippet SafeOutExample.cxx filtering in action
 *
 * #### Raising the threshold at run time
 * \snippet SafeOutExample.cxx raise threshold
 *
 * #### Silencing all output
 * \snippet SafeOutExample.cxx silence all
 *
 * #### Changing a stream's urgency level with setUrgency()
 * \snippet SafeOutExample.cxx setUrgency
 *
 * #### Safe concurrent output from multiple threads
 * \snippet SafeOutExample.cxx worker thread
 * \snippet SafeOutExample.cxx shared log
 * \snippet SafeOutExample.cxx spawn threads
 *
 * #### Writing non-string types
 * \snippet SafeOutExample.cxx type operators
 */

  /**
   * @class SafeOut
   * @brief Mutex-protected output stream with urgency-based filtering.
   *
   * SafeOut serialises writes to a shared `std::ostream` using a process-wide
   * static mutex, preventing interleaved output from concurrent threads.
   * An urgency filter lets the caller suppress low-priority messages at
   * run time by adjusting a shared threshold pointer without touching any
   * SafeOut object directly.
   *
   * All SafeOut instances in the process share a single `std::mutex`, so the
   * guarantee holds even when multiple SafeOut objects point at the same
   * underlying stream.
   */
  class SafeOut {
  public:

    /**
     * @brief Urgency levels for output filtering.
     *
     * Levels are ordered from most urgent (DIRE) to least urgent (TRIVIAL).
     * A message is written only when its urgency_select value is <=
     * the current *urgency_threshold.  NEVER suppresses output from a
     * stream unconditionally regardless of the threshold.
     */
    enum Urgency {
      NEVER      = 0, ///< never write to output stream, regardless of threshold
      DIRE       = 1, ///< write if threshold is DIRE or higher
      WORRISOME  = 2, ///< write if threshold is WORRISOME or higher
      PROBLEMATIC = 3,///< write if threshold is PROBLEMATIC or higher
      INTERESTING = 4, ///< write if threshold is INTERESTING or higher
      TRIVIAL    = 5  ///< write regardless of threshold (unless threshold is NEVER)
    };

    /**
     * @brief Shared pointer to an Urgency value.
     *
     * Passing the threshold as a `shared_ptr` lets the owner change the
     * verbosity ceiling at run time; all SafeOut objects that hold the same
     * pointer respond to the change immediately.
     *
     * @warning The owner's writes to `*UrgencyPtr` are not synchronised with
     * `safeout_mutex`, creating a formal data race under the C++ memory model.
     * This is benign on all supported architectures (single-word enum stores
     * are naturally atomic on x86-64 and AArch64), but is not
     * standards-guaranteed.  The formally correct type would be
     * `std::shared_ptr<std::atomic<Urgency>>`; that change has been deferred
     * because it would break the existing API.
     */
    typedef std::shared_ptr<Urgency> UrgencyPtr;

    /**
     * @brief Construct a SafeOut that writes to `std::cout`.
     *
     * Output is produced only when `urgency_select <= *urgency_threshold`.
     * The mutex is acquired for each individual `operator<<` call.
     *
     * @param urgency_threshold shared pointer to the run-time verbosity
     *        ceiling.  The owner may write to `*urgency_threshold` at any
     *        time to change which messages are printed.
     * @param urgency_select importance level assigned to this stream.
     *        Messages are emitted only when this value does not exceed the
     *        current threshold.  Defaults to PROBLEMATIC.
     */
    SafeOut(UrgencyPtr urgency_threshold,
	    Urgency urgency_select = PROBLEMATIC);

    /**
     * @brief Construct a SafeOut that writes to the given output stream.
     *
     * Output is produced only when `urgency_select <= *urgency_threshold`.
     * The mutex is acquired for each individual `operator<<` call.
     *
     * @param os destination output stream (e.g. `std::cerr`, a file stream).
     *        The caller must ensure the stream outlives this SafeOut object.
     * @param urgency_threshold shared pointer to the run-time verbosity
     *        ceiling.  The owner may write to `*urgency_threshold` at any
     *        time to change which messages are printed.
     * @param urgency_select importance level assigned to this stream.
     *        Messages are emitted only when this value does not exceed the
     *        current threshold.  Defaults to PROBLEMATIC.
     */
    SafeOut(std::ostream & os, UrgencyPtr urgency_threshold,
	    Urgency urgency_select = PROBLEMATIC);

    /**
     * @brief Write a string to the stream.
     *
     * The call is a no-op when `shouldWrite()` returns false.  Otherwise
     * the mutex is acquired for the duration of the write.  To emit an
     * atomic multi-token line, assemble a single `std::string` and pass it
     * in one call.
     *
     * @param str string to write
     * @return reference to this SafeOut for chaining
     */
    SafeOut & operator<<(const std::string & str);

    /**
     * @brief Write an unsigned long integer to the stream.
     *
     * @param v value to write
     * @return reference to this SafeOut for chaining
     */
    SafeOut & operator<<(unsigned long v);

    /**
     * @brief Write a signed long integer to the stream.
     *
     * @param v value to write
     * @return reference to this SafeOut for chaining
     */
    SafeOut & operator<<(long v);

    /**
     * @brief Write a float to the stream.
     *
     * @param v value to write
     * @return reference to this SafeOut for chaining
     */
    SafeOut & operator<<(float v);

    /**
     * @brief Write a boolean to the stream as the string "true" or "false".
     *
     * @param v value to write
     * @return reference to this SafeOut for chaining
     */
    SafeOut & operator<<(bool v);

    /**
     * @brief Change the urgency level assigned to this stream (sticky).
     *
     * After this call, subsequent writes are filtered against the new
     * urgency_select value.  The urgency_threshold pointer is not affected.
     *
     * @param select new urgency level for this stream
     * @return reference to this SafeOut for chaining
     */
    SafeOut & setUrgency(Urgency select);

  protected:

    /**
     * @brief Return true if a write should proceed.
     *
     * Must be called while `safeout_mutex` is held so that reads of
     * `urgency` and `*urgency_threshold_ptr` are synchronised with
     * concurrent writes from `setUrgency()` and the threshold owner.
     * Returns false immediately if `urgency_threshold_ptr` is null.
     *
     * @return true if urgency_threshold_ptr is non-null, urgency is not
     *         NEVER, and `urgency <= *urgency_threshold_ptr`
     */
    bool shouldWrite() const;

    /// Urgency level assigned to this stream instance (urgency_select).
    Urgency urgency;

    /// Shared pointer to the run-time verbosity ceiling.
    /// @note Read under safeout_mutex; owner writes are unsynchronised —
    ///       see UrgencyPtr for the formal data-race caveat.
    UrgencyPtr urgency_threshold_ptr;

    /// Pointer to the destination output stream.
    std::ostream * os_ptr;

    /// Process-wide mutex shared by all SafeOut instances.
    static std::mutex safeout_mutex;
  };
}
