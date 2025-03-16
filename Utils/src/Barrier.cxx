#include "Barrier.hxx"

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


namespace SoDa {
  Barrier::Barrier(const std::string & name, unsigned int num_waiters) : 
    name(name), num_waiters(num_waiters) {
    barrier_count = 0; 
    waiting = 0; 
    corrupted = false; 
  }

  void Barrier::wait(unsigned long timeout_ms) {
    if(timeout_ms == 0) {
      wait(std::chrono::seconds(3600 * 24 * 1000));
    }
    else {
      wait(std::chrono::milliseconds(timeout_ms));    
    }
    return;
  }

  // guided, informed, enlightened by
  // http::/github.com/kirksaunders/barrier  Nice work. 
  void Barrier::wait(const std::chrono::duration<long, std::milli> & timeout) {
    if(timeout.count() == 0) {
      return wait(std::chrono::seconds(3600 * 24 * 1000));
    }
    
    // bump the waiting count
    auto old_barrier_count = barrier_count; 
    {
      std::unique_lock<std::mutex> lock(mtx);
      
      waiting++; 
      if(waiting == num_waiters) {
	// reset -- nobody will be waiting anymore
	waiting = 0;
	barrier_count++; // we've all cleared another barrier
	cv.notify_all();
      }
      else {
	// we've got to wait.  not that cv.wait releases our lock
	// and does the right thing wrt the race between bumping
	// the barrier_count and getting to the point where we're
	// testing things. 
	bool is_ok = cv.wait_for(lock, 
				 timeout,
				 [old_barrier_count, this]() {
				   return old_barrier_count != barrier_count; 
				 });
	
	// if the barrier hasn't advanced, then we timed out.
	if (!is_ok) {
	  // that's really bad, as we've got a broken barrier.
	  // If we return it will be truly broken, and everyone will
	  // hang. Though it is likely that everyone is hung already.
	  bool old_corrupt = corrupted; 
	  if(old_corrupt) {
	    throw Barrier::Corrupt(*this);
	  }
	  else {
	    // we're the first one to timout. 
	    corrupted = true; 
	    // set the barrier count to infinity, and wake everyone up
	    barrier_count = 0xffffffff;
	    cv.notify_all();
	    throw Barrier::Timeout(*this, timeout);
	  }
	}
      }
    }
  }

  BarrierPtr Barrier::make(const std::string & name, unsigned int num_waiters) {
    return std::make_shared<Barrier>(name, num_waiters);
  }
  
}
