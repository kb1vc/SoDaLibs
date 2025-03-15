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
  SoDa::BarrierPtr barrier_p = SoDa::makeBarrier("test barrier", num_waiters);
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
