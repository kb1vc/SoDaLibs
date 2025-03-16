#include "../include/MailBox.hxx"
#include <memory>
#include "../include/Format.hxx"
#include "../include/Options.hxx"
#include "../include/Barrier.hxx"

#include <iostream>
#include <thread>
#include <functional>
#include <chrono>

std::mutex mtx;

class MyMsg {
public:
  MyMsg(int from, int v) : from(from), v(v) {
    std::lock_guard<std::mutex> lock(mtx);
    tot_active++;
  }

  ~MyMsg() {
    std::lock_guard<std::mutex> lock(mtx);    
    tot_active--;
  }

  static std::shared_ptr<MyMsg> makeMsg(int from, int v) {
    return std::make_shared<MyMsg>(from, v);
  }

  static unsigned int tot_active;
  int from, v;
};

unsigned int MyMsg::tot_active = 0;

int objMailBoxTest(SoDa::MailBoxPtr<MyMsg> mailbox_p, 
		   int num_msgs, int num_threads, 
		   int my_id, 
		   std::shared_ptr<SoDa::Barrier> barrier_p, 
		   int num_trials, 
		   bool no_echo) {

  //! [subscribe and wait]
  auto subs = mailbox_p->subscribe();
  barrier_p->wait();
  //! [subscribe and wait]  

  for(int tr = 0; tr < num_trials ; tr++) { 
    // push some messages
    //! [send messages]
    for(int i = 0; i < num_msgs; i++) {
      auto msg = MyMsg::makeMsg(my_id, i);
      if(no_echo) {
	mailbox_p->put(msg, subs);
      }
      else {
	mailbox_p->put(msg);
      }
    }
    //! [send messages]

    // now look for messages
    int totmsgs = num_msgs * num_threads;
    // let's sum the content of all the messages
    unsigned long msg_sum = 0;
    unsigned long sender_sum = 0; 
    unsigned long expected_msgs = num_msgs * num_threads;
    if(no_echo) expected_msgs -= num_msgs; 

    for(int i = 0; i < expected_msgs;) {
      //! [get message]
      auto p = mailbox_p->get(subs);
      if(p != nullptr) {
	i++;
	msg_sum += p->v;
	sender_sum += p->from;
      }
      //! [get message]    
    }

    unsigned long N = num_msgs - 1;
    unsigned long actual_threads = num_threads - (no_echo ? 1 : 0);
    unsigned long expected_sum = actual_threads * (N * (N+1))/2;
  
    if(expected_sum != msg_sum) {
      std::cerr << SoDa::Format("subscriber %0 got all %1 expected messages sum was %2 (should have been %4) sender sum was %3\n")
	.addI(my_id)
	.addI(totmsgs)
	.addU(msg_sum)
	.addU(sender_sum)
	.addU(expected_sum);
      return -1;
    }
  }

  std::shared_ptr<MyMsg> p; 
  while((p = mailbox_p->get(subs)) != nullptr) {
    std::cerr << SoDa::Format("subscriber %0 got extra message from subscriber %1 : %2\n")
      .addI(my_id)
      .addI(p->v);
      return -1; 
  }
  
  
  return mailbox_p->minReadyCount(); 
}

  
int testObjMessage(int msg_count, int num_threads, int num_trials, bool no_echo) {
  //! [create a mailbox]
  SoDa::MailBoxPtr<MyMsg> mailbox_p = SoDa::makeMailBox<MyMsg>("MessageMailbox");
  //! [create a mailbox]

  // Just testing the pointer conversion to make sure we can build
  // tables of mailboxes that do the right thing. 
  std::shared_ptr<SoDa::MailBoxBase> mb_p = mailbox_p;
  SoDa::MailBoxPtr<MyMsg> nmm_p = SoDa::MailBoxBase::convert<SoDa::MailBox<MyMsg>>(mb_p);

  //! [create a barrier]
  // each thread will wait at the barrier until everyone has finished
  // subscribing.
  auto barrier_p = SoDa::Barrier::make("subscription_barrier", num_threads);
  //! [create a barrier]  
  
  std::cerr << "Creating threads\n";
  
  std::list<std::thread *> threads;
  std::cerr << SoDa::Format("Spawning %0 threads\n").addI(num_threads);

  //! [create threads]
  for(int i = 0; i < num_threads; i++) {
    threads.push_back(new std::thread(objMailBoxTest,
				      mailbox_p,
				      msg_count, 
				      num_threads,
				      i,
				      barrier_p, 
				      num_trials,
				      no_echo));
  }
  //! [create threads]  
  std::cerr << SoDa::Format("Waiting to join threads\n");
  
  for(auto t : threads) {
    t->join();
  }

  std::cerr << "Joined all threads\n";

  return 0;
}

void testMBoxConversion() {
  SoDa::MailBoxPtr<MyMsg> mailbox_p = SoDa::makeMailBox<MyMsg>("MessageMailbox");

  // Just testing the pointer conversion to make sure we can build
  // tables of mailboxes that do the right thing. 
  std::shared_ptr<SoDa::MailBoxBase> mb_p = mailbox_p;
  SoDa::MailBoxPtr<MyMsg> nmm_p = SoDa::MailBoxBase::convert<SoDa::MailBox<MyMsg>>(mb_p);
  
  // now make sure we get a conversion failure if we try to convert to the
  // wrong kind of mailbox
  bool found_problem = false; 
  try {
    SoDa::MailBoxPtr<int> nmmp = SoDa::MailBoxBase::convert<SoDa::MailBox<int>>(mb_p);
  }
  catch (SoDa::MailBoxBase::Exception & e) {
    std::cerr << e.what() << "\n";
    found_problem = true;
  }

  if(!found_problem) {
    std::cerr << "testMBoxConversion: bad pointer conversion did not throw an exception\n";
    exit(-1);
  }
}

int main(int argc, char ** argv) {
  // create a mailbox
  SoDa::Options cmd;

  int msg_count, num_threads, num_trials; 
  bool no_echo; 
  cmd.add<int>(&msg_count, "msgs", 'm', 1, "Number of messages to send from each thread")
    .add<int>(&num_threads, "th", 't', 2, "Number of threads in test.")
    .add<int>(&num_trials, "trials", 'r', 1, "Number of trials to run.")
    .addP(&no_echo, "noecho", 'n', "When present, a thread will not \"see\" its own outbound messages.");
  

  if(!cmd.parse(argc, argv)) exit(-1);

  testMBoxConversion();
  
  // std::cerr << "test 1\n";
  // testVectorMsg(msg_count, num_threads);
  
  std::cerr << "test 2\n";
  testObjMessage(msg_count, num_threads, num_trials, no_echo);
  
  if(MyMsg::tot_active > 0) {
    std::cerr << "There may be a leak in allocating messages: " << 
      MyMsg::tot_active << " still outstanding.\n";
    exit(-1);
  }
}
