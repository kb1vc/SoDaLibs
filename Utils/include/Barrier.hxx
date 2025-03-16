#pragma once
#include <string>
#include <memory>
#include <mutex>
#include <condition_variable>
#include "NoCopy.hxx"
#include "Exception.hxx"
#include "Format.hxx"

/*
  BSD 2-Clause License

  Copyright (c) 2022, Matt Reilly - kb1vc
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
 * @file Barrier.hxx
 * @author Matt Reilly (kb1vc)
 * @date Feb 21, 2022
 */

/**
 * @page SoDa::Barrier Barrier: A simple std::thread barrier 
 *
 *
 * It can't get much simpler. There's an example in examples/BarrierExample.cxx
 *
 * Here's what happens in the example:
 *
 * Create a barrier. 
 * 
 * The barrier is initialized to expect "num_threads + 1" or six
 * threads to wait on the barrier. (In this example, one for each
 * thread we create, and one for the parent thread.) All threads will
 * wait at the barrier until all of the waiters have called `wait`.
 * 
 * \snippet BarrierExample.cxx  create the barrier
 * 
 * Pass it to all of the threads. 
 * 
 * \snippet BarrierExample.cxx create threads
 * 
 * In this example, we pause one second before creating each
 * thread. (There are some messages that the full example prints to
 * show that things are happening.) The `sleepOneSecond` is *not* necessary. 
 * 
 * Each of the threads can then wait at the barrier, and stay there
 * until all six waiters have arrived. In this first wait, the threads
 * will wait forever, if necessary. This is often a reasonable thing
 * to do, as timeouts are almost always a maintenance and debug issue.
 * 
 * \snippet BarrierExample.cxx wait at the barrier
 *
 * But some applications need to do something when bad things
 * happen. A thread can wait, but set a timeout. If the timeout
 * expires, the `wait` method will throw a SoDa::Barrier::Timeout exception at the
 * waiter that timed out. All other waiters will get a SoDa::Barrier::Corrupt
 * exception to let them know that the barrier is broken.
 *
 * Here's how to wait, but bail out after one minute. (That's a little
 * impatient, in my book. But if you are writing a flight control
 * system, one minute might be a little long.)

 * > Hey! Why are you using
 * > this code in a flight control system? Sure the BSD license allows
 * > it, but are you nuts? This is written by a guy in his spare time,
 * > working on an old linux box. Sure, I've worked hard on this, but
 * > not hard enough to say that it is something you should bet
 * > someone's life on. Get a grip.
 *
 *  In any case, here's how to wait,
 * but timeout if waiting lasts longer than a minute.
 * 
 * \snippet BarrierExample.cxx wait at the barrier with timeout
 * 
 * That's it. The barrier will get cleaned up when everybody lets the
 * barrier pointer go out of scope, or reassigns it. (Underneath the
 * covers BarrierPtr is a std::shared_ptr<Barrier>.)  But that has an
 * important aspect to it. If the barrier is set up to wait for N
 * threads, then N threads have to call wait before any caller can go
 * forward.
 * 
 * The devious among you will have noticed that the barrier doesn't
 * care *who* is waiting, just that it is the right number. And
 * BarrierPtr values can be passed around and shared. So it is
 * conceivable that a barrier created for 8 waiters might be known to
 * 10 threads. If 10 threads call wait, 8 of them will clear the
 * barrier, and the other two will have to wait until six more call
 * wait. So it isn't a good thing to do.  
 * 
 * We could fix that with a subscription mechanism, like with the
 * SoDa::MailBox class. But the flexibility might be worth the
 * danger. We'll find out, won't we?
 * 
 */

/**
 * @namespace SoDa
 * 
 * Not much else is spelled that way, so we're probably not going to
 * have too many collisions with code written by other people.
 *
 * SoDa is the namespace I use in code like
 * <a href="https://kb1vc.github.io/SoDaRadio/">SoDaRadio</a> (The S
 * D and R stand for Software Defined Radio.  The o,a,d,i,o don't
 * stand for anything in particular.)  But this code has nothing to do
 * with software defined radios or any of that stuff.
 */
namespace SoDa {

  
  
  /**
   * @class Barrier
   * @brief All threads connected with a barrier will wait on calling
   * "wait". All will be awakened when all have called wait. 
   *
   */

  class Barrier;
  
  typedef std::shared_ptr<Barrier> BarrierPtr;  
  
  class Barrier : public NoCopy {
  public:
    /**
     * @brief constructor 
     * @param name Name of the barrier
     * @param num_waiters number of threads that will wait at this barrier.
     */    
    Barrier(const std::string & name, unsigned int num_waiters);

    /**
     * @brief a local Exception class so that users can do a catch like this: 
     * 
     * `catch(SoDa::Exception & e)`
     * 
     * or 
     * 
     * `catch(SoDa::Barrier::Exception & e)`
     * 
     * or 
     * 
     * `catch(SoDa::Barrier::Timeout & e)`
     * 
     * depending on how particular we want to be. 
     */
    class Exception : public SoDa::Exception {
    public:
      Exception(const std::string & err) : SoDa::Exception(err) { }
    }; 

    /**
     * @brief A waiter decided that too long was too long. If a waiter
     * "times out" it will throw a Timeout exception. This is
     * important as it indicates that something is desparately wrong.
     */
    class Timeout : public Exception {
    public:      
      Timeout(Barrier & b, 
	      const std::chrono::duration<long, std::milli> & timeout) :
	Exception(SoDa::Format("Barrier %0 timed out after %1 ms. Current state is dangerous\n")
		  .addS(b.name)
		  .addU(timeout.count()).str()) {
      }
    }; 

    /**
     * @brief When a wait throws a Timeout exception, we know that the barrier
     * has now completely come unglued. Waiters will get stuck in the barrier. 
     * When the first waiter times-out, it will set the ```corrupted``` flag and 
     * wake the other waiters up. The first waiter will throw Timeout while
     * the rest will throw Corrupt.
     * 
     * The Corrupt exception is 
     */
    class Corrupt : public Exception {
    public:
      Corrupt(Barrier & b) :
	Exception(SoDa::Format("Barrier %0 is corrupted.\n")
		  .addS(b.name).str()) {

      }
    }; 

    /**
     *
     * @brief wait for all waiters to arrive at the barrier. 
     *
     * @param timeout_ms thrown an exception if "timeout_ms" has
     * expired without all reaching the barrier.
     * 
     * If timeout_ms is zero, the timeout will be set to about 1000
     * days.  This is not equal to std::chrono::duration::max() as,
     * for some reason, condition_value::wait_for will trigger
     * immediately. I think it is treating count() as a signed
     * integer.. dunno.
     * 
     * @throws Barrier::Timeout if we reach the timeout limit
     * @throws Barrier::Corrupt if the barrier state is corrupted. 
     */
    void wait(unsigned long timeout_ms = 0);

    /**
     *
     * @brief wait for all waiters to arrive at the barrier. 
     *
     * The timeout should be chosen carefully. If any one of the threads
     * at a barrier "times out" then all threads will eventually time out. 
     *
     * @param timeout wait will throw an exception if "timeout" has
     * expired without all reaching the barrier.  Timeout is specified
     * as a chrono::duration. 
     * 
     * If timeout is zero, the timeout will be set to about 1000
     * days.  This is not equal to std::chrono::duration::max() as,
     * for some reason, condition_value::wait_for will trigger
     * immediately. I think it is treating count() as a signed
     * integer.. dunno.
     * 
     * before the timeout, false otherwise.
     */
    void wait(const std::chrono::duration<long, std::milli> & timeout); 

    /**
     * @brief Make a barrier and return a shared pointer to it. 
     *
     * This is a safer way of creating barrier, as it ensures that
     * the barrier will live even after it "goes out of scope" in the
     * thing that created the it and spawned the threads that use
     * it. Sure we could rely on good behavior, but why? 
     *
     * @param name Name of the barrier
     * @param num_waiters number of threads that will wait at this barrier.
     * @returns shared pointer to a barrier object
     */ 
    static BarrierPtr make(const std::string & name, unsigned int num_waiters);
    
  private:
    std::string name;
    unsigned int num_waiters;
    unsigned int waiting;
    unsigned int barrier_count; 

    bool corrupted; 

    // mutex and condition variable
    std::mutex mtx; 
    std::condition_variable cv; 
  };


}



    
