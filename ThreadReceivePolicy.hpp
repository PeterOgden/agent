// Provides an implementation for a Thread-based receive policy where each instance of
// an agent has its own thread for handing received messages.

#ifndef THREAD_RECEIVE_POLICY_HPP
#define THREAD_RECEIVE_POLICY_HPP

#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>

#include "Agent.hpp"

namespace Agent
{

// This causes all receive events to happen on a new thread.
// The thread is started when "Start" is called and stopped
// when "Finish" is called There

class ThreadReceivePolicy
{
public:
	// Class to allow queues to be polled without knowing the type of the message
	class QueuePoll
	{
	public:
		virtual bool ExecuteSaved() = 0;
	};

	// Required implementation of receive to meet the requirement of a ReceivePolicy
	// This implementation uses per-queue messages but a single lock for the instance
	// This is not particularly efficient but serves as a proof of concept
	template <typename Message>
	class ReceiveImpl : public Receiver<Message>, public QueuePoll
	{
	public:
		ReceiveImpl(ThreadReceivePolicy* policy):
			m_queue(),
			m_policy(policy)
		{
			m_policy->m_queues.push_back(this);
		}

		// Places the message on the receiving queue
		virtual void Input(Message m)
		{
			std::unique_lock<std::mutex> l(m_policy->m_mutex);
			m_queue.push(std::move(m));
			m_policy->m_cv.notify_all();
		}

		// Processes a single saved message. Returns true if there are
		// more messages left to process. Ought to consider releasing the
		// lock otherwise cycles may cause deadlocks.
		virtual bool ExecuteSaved()
		{
			if (!m_queue.empty())
			{
				this->Receive(m_queue.front());
				m_queue.pop();
			}
			return !m_queue.empty();
		}
	private:
		std::queue<Message> m_queue;
		ThreadReceivePolicy* m_policy;
	};

	// Starts the processing thread
	void Start()
	{
		m_thread = std::thread([=] () { this->Entry(); });
	}

	// Stops the processing thread
	void Finish()
	{
		//std::unique_lock<std::mutex> l(m_mutex);
		m_exit = true;
		m_cv.notify_all();
		m_thread.join();
	}

private:
	// Returns true if anything is left to execute
	bool ExecuteSaved() {
		bool left = false;
		for (QueuePoll* q: m_queues)
		{
			left |= q->ExecuteSaved();
		}
	}


protected:
	// Returns false when the thread has been signalled to exit;
	bool WaitForEvent()
	{
		std::unique_lock<std::mutex> l(m_mutex);
		while (ExecuteSaved());
		m_cv.wait(l);
		while (ExecuteSaved());
		return !m_exit;
	}
private:
	std::thread m_thread;
	std::condition_variable m_cv;
	std::mutex m_mutex;
	bool m_exit;
	std::vector<QueuePoll*> m_queues;

	// Default implementation of the entry function can be overridden
	// by users if required
	virtual void Entry() {
		while (WaitForEvent());
	}
};

}
#endif
