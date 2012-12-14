#ifndef NON_REENTRANT_POLICY_HPP
#define NON_REENTRANT_POLICY_HPP

// Defines a policy which does not start a thread but does not permit multiple threads to be in
// Receive methods simultaneously

#include <atomic>
#include <queue>
#include <mutex>
#include <vector>

#include "Agent.hpp"
namespace Agent
{

class NonReentrantPolicy
{
public:
	class QueuePoll
	{
	public:
		virtual void ExecuteSaved() = 0;
	};

	template <typename Message>
	class ReceiveImpl : public Receiver<Message>, public QueuePoll
	{
	public:
		ReceiveImpl(NonReentrantPolicy* policy):
			m_policy(policy),
			m_queue(),
			m_mutex()
		{
			policy->m_queues.push_back(this);
		}
		
		class AtomicWrapper
		{
		public:
			AtomicWrapper(std::atomic<int>& a) : m_atomic(a) { m_value = a++; }
			~AtomicWrapper() { --m_atomic; }
			const int OldValue() const { return m_value; }
		private:
			std::atomic<int>& m_atomic;
			int m_value;
		};

		virtual void Input(Message m)
		{
			AtomicWrapper a(m_policy->m_threadCount);
			if (a.OldValue() == 0) // We are the only thread active
			{
				this->Receive(std::move(m));
				m_policy->ProcessOutstanding();
			}
			else //We need to buffer the message for later processing
			{
				std::unique_lock<std::mutex> l(m_mutex);
				m_queue.push(std::move(m));
			}
		}

		virtual void ExecuteSaved()
		{
			while (!m_queue.empty())
			{
				Message m;
				{
					std::unique_lock<std::mutex> l(m_mutex);
					m = std::move(m_queue.front());
					m_queue.pop();
				}
				this->Receive(std::move(m));
			}
		}

	private:
		NonReentrantPolicy* m_policy;
		std::queue<Message> m_queue;
		std::mutex m_mutex;
	};
public:
	void ProcessOutstanding()
	{
		for (QueuePoll* q : m_queues)
		{
			q->ExecuteSaved();
		}
	}


	void Start() {}
	void Finish() {}
private:
	std::atomic<int> m_threadCount;
	std::vector<QueuePoll*> m_queues;
};

}

#endif
