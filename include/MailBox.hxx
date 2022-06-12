#pragma once
#include <string>
#include <map>
#include <queue>
#include <memory>
#include <mutex>
#include "Format.hxx"
#include "Exception.hxx"
#include "NoCopy.hxx"
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
 * @file MailBox.hxx
 * @author Matt Reilly (kb1vc)
 * @date Feb 21, 2022
 */

/**
 * @page SoDa::MailBox A simple thread-to-thread communication class. 
 *
 * This was derived from classes originally developed for SoDaRadio.
 * The advent of smart pointer support in C++ made the old
 * implementation look pretty crusty. Earlier implementations also
 * assumed the existence of a buffer allocator, to recycle created
 * objects. This didn't prove to be all that useful. In the case of a
 * large std::vector, for instance, the buffer allocator was *less*
 * efficient and speedy than just creating each vector when needed.
 * 
 * The multithread model here assumes a set of threads sharing a common
 * address space. Threads may put messages into a mailbox that are then
 * readable by any other thread that subscribes to the mailbox.  This is
 * a bit of a mixed metaphor, but imagine messages as magazines, threads
 * as subscribers, and mailboxes as the thing to which the magazines get
 * delivered.
 * 
 *  (Long ago, there were "publishers" who gathered up information on
 * a common topic, or for readers with a common interest. This stuff
 * would be written down and then printed on "paper." Many sheets of
 * paper would be stuck together, almost like a book -- but
 * thinner. (For an example of a "book" see a museum near you. It
 * might be called *the something* Library.) These not-quite-books
 * would be carried to each person who paid for it, *subscribers* on a
 * regular basis. They would be left at each subscriber's house kinda like
 * Amazon's *subscribe and save* thing.)
 *
 * The SoDa::MailBox class provides a mailbox that accepts and delivers
 * messages of a single type (named when the mailbox is created). Each
 * message sent will be received by *all* subscribers, even the originating
 * subscriber. 
 *
 * For an example, take a look at MailBoxTest.cxx. 
 * 
 * After usign the SoDa::Options class to parse the command line, 
 * the example creates a mailbox and a std::shared_ptr to it (mailbox_p).
 * In this example, all messages for this mailbox are vectors of ints.  
 * 
 * \snippet MailBoxTest.cxx create a mailbox
 * 
 * The example actually creates a '''shared_ptr''' to the mailbox. It
 * is a *really* bad idea to pass a pointer to an object created on
 * the stack to a thread. It is possible that the object might go out
 * of scope in its creator before the thread is done with
 * it. Allocating a MailBox with '''new''' and passing a pointer to
 * the resulting heap allocated object is much safer. Using a
 * '''shared_ptr''' will ensure that the MailBox isn't destroyed until
 * all threads and other pointer users have "let it go."
 * 
 * Then it creates a set of threads, using the C++ thread class. 
 * Each thread executes a single function called "objMailBoxTest."
 * The function is passed a pointer to the mailbox, a number of messages
 * to send, and the total number of threads participating in the test. 
 * 
 * \snippet MailBoxTest.cxx create threads
 *
 * Each thread subscribes to the mailbox, and then waits for all the other
 * threads to subscribe. (Each thread waits at a barrier:)
 *
 * \snippet MailBoxTest.cxx subscribe and wait
 *
 * In this example, each thread puts '''num_msgs''' messages into the
 * mailbox.
 * 
 * \snippet MailBoxTest.cxx send messages
 *
 * Once each thread has sent its messages, it will get, in turn, each
 * message that was posted to the mailbox. All threads will receive
 * All messages, even their own.
 * 
 * \snippet MailBoxTest.cxx get message
 * 
 * The call to '''get''' is non-blocking. If there are no messages in
 * the mailbox, '''get''' will return a nullptr.  Also, see that the
 * '''get''' method takes a "subscriber id." This is the id that was
 * returned by the '''subscribe''' method. Calling get with a
 * different subscriber id will screw things up, in all likelihood.
 * 
 * The '''get''' method returns a shared pointer to a message. The
 * shared pointer is important, as when its value changes or it goes
 * out of scope, the reference count for the message is
 * decreased. When the last subscriber has read and disposed of the
 * message (by setting the pointer to nullptr or just letting it go
	    * out of scope) the storage for the message will be reclaimed (the
									   * message will be destroyed.)
									   * 
									   * Because of this mechanism, if one of the subscribers to a mailbox
									   * stops reading the mail (calling '''get''') that subscriber's
									   * mailbox will grow, filling with unread messages. The other
									   * subscribers will proceed unimpeeded, but the waiting messages take
									   * up storage and can't be freed until the laggard subscriber catches
									   * up.
									   *
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
   * @brief Catch this when you don't care why the MailBox threw an exception
   */
  class MailBoxException : public SoDa::Exception {
  public:
    MailBoxException(std::string name, const std::string & problem) :
      SoDa::Exception("SoDa::MailBox[" + name + "] " + problem) {
    }
  };

  
  /**
   * @brief The subscriber ID was invalid. 
   */
  class MissingSubscriber : public MailBoxException {
  public:
    MissingSubscriber(std::string name, const std::string & operation, int sub_id) :
      MailBoxException(name, "::" + operation + " Subscriber ID " + std::to_string(sub_id) + " not found.") {
    }
  };

  class SubscriptionMismatch : public MailBoxException {
  public:
    SubscriptionMismatch(const std::string & should_be
			 , const std::string & was) : 
      MailBoxException(should_be, SoDa::Format("caller specified the wrong mailbox")
		       .addS(was).str()) {
    }
  }; 
  
  /**
   * @class MailBox<T>
   * @brief Accept messages and distribute them to multiple mailboxes
   *
   * @tparam T Type of message that will be found in this mailbox
   */
  template<typename T>
  class MailBox : public NoCopy {
  public:
    /**
     * @brief Create a mailbox. All mailboxes can put and get 
     * messages from the mailbox. 
     *
     * Messages are assumed to be std::shared_ptr as a message pointer
     * will be released from the message queue when a subscriber takes
     * posesion of it.
     * 
     * Each subscriber gets  message queue.  It is up to the subscriber
     * to "read the mail"
     */
    MailBox(std::string name) : name(name) {
      subscription_counter = 0; 
    }

    ~MailBox() {
      for(auto & s : message_queues) {
	while(!s.second.empty()) s.second.pop();
      }
      message_queues.clear();
    }

  protected:  
    class SubscriptionCl {
    public:
      SubscriptionCl(MailBox<T> * mbox, int idx) {
	this_mbox = mbox; 
	subscriber_index = idx; 
      }

      ~SubscriptionCl() {
	this_mbox->unsubscribe(subscriber_index);
      }

      int getIndex(MailBox<T> * mbox) {
	if(mbox != this_mbox) {
	  throw SubscriptionMismatch(mbox->getName(), this_mbox->getName()); 
	}
	else {
	  return subscriber_index; 
	}
      }
      /// selects the message queue
      int subscriber_index;
      /// double check that we're referencing the right mailbox
      MailBox<T> * this_mbox; 
    };

  public:
    typedef std::unique_ptr<SubscriptionCl> Subscription; 
    
    /**
     * @brief Subscribe the caller to a mailbox.  There may be 
     * multiple subscribers to the same mailbox.  A copy of each
     * message will be reserved for each caller. 
     * 
     * @returns a smart pointer to a subscriber object. 
     */
    Subscription subscribe() {
      std::lock_guard<std::mutex> lock(mtx);	      
      // what will the subscriber number be? 
      Subscription ret(new SubscriptionCl(this,
					  subscription_counter));

      // make a subscriber queue
      message_queues[subscription_counter] = std::queue<std::shared_ptr<T>>();
      
      subscription_counter++;
      return ret;
    }

    /**
     * @brief What is the name of this mailbox? 
     * @return the name.
     */
    const std::string & getName() const { return name; }

    /**
     * Get an object out of the mailbox for this subscriber
     * 
     * @param subs each user of a mailbox must have subscribed to the mailbox. 
     * (I know, we're mixing metaphors here... sigh.)
     * @returns The oldest object in the subscriber's mailbox. 
     */
    std::shared_ptr<T> get(Subscription & subs) {
      std::lock_guard<std::mutex> lock(mtx);	      
      auto mqueue = getQueue(subs); 
      if(mqueue.empty()) {
	return nullptr;
      }
      else {
	auto ret = mqueue.front();
	mqueue.pop();
	return ret;
      }
    }

    /**
     * @brief Place a message in every subscriber's mailbox
     *
     * @param msg The message to be sent to every subscriber. Note that this 
     * is passed by value -- each subscriber will get a copy.  Shared pointers
     * work just fine. 
     */
    void put(std::shared_ptr<T> msg) {
      std::lock_guard<std::mutex> lock(mtx);            
      for(auto & q : message_queues) {
	q.second.push(msg);
      }
    }

    /**
     * @brief Empty the subscriber's mailbox
     *
     * @param subs -- identifies the subscription we're clearing
     */
    void clear(Subscription & subs) {
      std::lock_guard<std::mutex> lock(mtx);      
      auto mqueue = getQueue(subs);
      while(!mqueue.empty()) mqueue.pop_front();
    }

    void unsubscribe(int subid) {
      std::lock_guard<std::mutex> lock(mtx);
      auto mqueue = getQueue(subid);
      while(!mqueue.empty()) mqueue.pop();
      // now remove our entry from the message queues.
      message_queues.erase(subid);
    }
  protected:
    std::string name;
    std::map<int, std::queue<std::shared_ptr<T>>> message_queues; 
    int subscription_counter; 

    std::queue<std::shared_ptr<T>> & getQueue(int idx) {
      if(message_queues.count(idx) == 0) {
	throw MissingSubscriber(getName(), "get()", idx);
      }
      return message_queues[idx];       
    }
    
    std::queue<std::shared_ptr<T>> & getQueue(Subscription & subs) {
      int idx = subs->getIndex(this);
      return getQueue(idx);
    }

    // mutual exclusion stuff
    std::mutex mtx; 



  };
  


  /**
   * @brief Make a mailbox and return a shared pointer to it. 
   *
   * This is a safer way of creating mailboxes, as it ensures that
   * the mailbox will live even after it "goes out of scope" in the
   * thing that created the mailbox and spawned the threads that use
   * it. Sure we could rely on good behavior, but why? 
   *
   * @param mname Name of the mailbox. 
   * @returns shared pointer to a Mailbox object
   */ 
    template<typename T>
    std::shared_ptr<MailBox<T>> makeMailBox(const std::string & mname) {
      return std::make_shared<MailBox<T>>(mname);
    }
  
}



    
