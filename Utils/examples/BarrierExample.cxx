/*
BSD 2-Clause License

Copyright (c) 2021,2025 Matt Reilly - kb1vc
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

#include <SoDa/Barrier.hxx>
#include <thread>
#include <chrono>

void sleepOneSecond() {
  std::this_thread::sleep_for(std::chrono::seconds(1));
}

void barrierExample(SoDa::BarrierPtr barrier_p, int my_id) {
  std::cerr << my_id;
  //! [wait at the barrier]
  // wait at the barrier, forever if we need to. 
  barrier_p->wait();
  //! [wait at the barrier]
  //! [wait at the barrier with timeout]
  // wait at the barrier, but if we wait for more than 10 minutes, bail out. 
  barrier_p->wait(std::chrono::minutes(1));
  //! [wait at the barrier with timeout]
  std::cerr << my_id;  
}

int main(int argc, char ** argv) {
  // demostrate a simple barrier

  //! [create the barrier] 
  int num_threads = 5; // we'll create five threads
  // each thread will wait at the barrier, and so will this main thread. 
  int num_waiters = num_threads + 1;
  SoDa::BarrierPtr barrier_p = SoDa::Barrier::make("test barrier", num_waiters);
  //! [create the barrier] 

  //! [create threads]
  std::list<std::thread *> threads;
  for(int thread_id = 0; thread_id < num_threads; thread_id++) {
    sleepOneSecond();
    threads.push_back(new std::thread(barrierExample, 
				      barrier_p, 
				      thread_id));
  }
  //! [create threads]

  sleepOneSecond();
  std::cerr << " wait at the barrier\n";
    
  //! [main waits too]
  // wait twice, just like the example threads
  barrier_p->wait();
  barrier_p->wait();  
  //! [main waits too]


  //! [wait for completion]
  for(auto t : threads) {
    t->join();
  }
  //! [wait for completion]

  std::cerr << " clear the barrier\n";
  
  exit(0);
}
