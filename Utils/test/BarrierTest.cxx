#include "../include/Barrier.hxx"
#include <memory>
#include "../include/Format.hxx"
#include "../include/Options.hxx"

#include <iostream>
#include <thread>
#include <functional>
#include <chrono>
#include <random>

void threadBarrierTest(SoDa::BarrierPtr bar, int trial_count, int max_duration, int my_idx) {
  // make the random generator
  int seed = my_idx * 234525 + 23919;  
  std::default_random_engine generator(seed);
  std::uniform_int_distribution<unsigned int> distribution(1, max_duration);
  
  // do the trials
  try {
    for(int i = 0; i < trial_count; i++) {
      // pick a random time to wait.    
      auto rtime = distribution(generator);
      std::this_thread::sleep_for(std::chrono::microseconds(rtime));

      // now go to the barrier -- we set an absurdly short timeout
      bar->wait(std::chrono::milliseconds(100));
    }
  }
  catch (SoDa::Exception & e) {
    std::cerr << "Ooops, dang!: " << e.what() << "\n";
    return; 
  }
}

int testBarrier(int trial_count, int num_threads, int max_duration) {
  //! [create the barrier] 
  SoDa::BarrierPtr barrier_p = SoDa::makeBarrier("test barrier", num_threads);
  //! [create the barrier] 
  
  std::cerr << "Creating threads\n";
  
  std::list<std::thread *> threads;
  std::cerr << SoDa::Format("Spawning %0 threads\n").addI(num_threads);

  //! [create threads]
  for(int i = 0; i < num_threads; i++) {
    threads.push_back(new std::thread(threadBarrierTest,
				      barrier_p, 
				      trial_count, 
				      max_duration, 
				      i));

  }
  //! [create threads]
  
  std::cerr << SoDa::Format("Waiting to join threads\n");
  
  for(auto t : threads) {
    t->join();
  }

  std::cerr << "Joined all threads\n";

  return 0;
}

int main(int argc, char ** argv) {
  // create a mailbox
  SoDa::Options cmd;

  int trial_count, num_threads, max_duration;
  cmd.add<int>(&trial_count, "trials", 'r', 100, "Number of passes through the barrier")
    .add<int>(&num_threads, "threads", 't', 10, "Number of threads in test.")
    .add<int>(&max_duration, "maddur", 'm', 50, "Max time in us for the random wait between barriers");

  if(!cmd.parse(argc, argv)) exit(-1);

  testBarrier(trial_count, num_threads, max_duration);
  
}
