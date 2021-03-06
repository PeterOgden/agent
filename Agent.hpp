// Main header for Agent implementations
// Contains the Agent base class and simple non-threaded send and receive policies

#ifndef AGENT_HPP
#define AGENT_HPP

#include <utility>

namespace Agent
{

// Send and Receive Tag Types for Messages
template <typename Message> class Send {};
template <typename Message> class Receive {};

// Receiving Base Class
// Provides an interface for Policy implementations
template <typename Message> class Receiver {
public:
	virtual ~Receiver() {};
	// Is called by a sender and implemented by a policy
	// implementation
	virtual void Input(Message m) = 0;
	// Is implemented by the user of the class
	virtual void Receive(Message m) = 0;
};

// Sending Base Class
// Provides an interface for Policy implementations
template <typename Message> class Sender {
public:
	virtual ~Sender() {}
	// Called by the user and implemented by the policy
	virtual void Send(Message m, Receiver<Message>& r) = 0;
	virtual void Send(Message m) = 0;
};

//Simple Sending Policy, Fowards things directly
class SimpleSendPolicy
{
public:
	template <typename Message>
	class SendImpl : public Sender<Message>
	{
	public:
		virtual void Send(Message m, Receiver<Message>& r)
		{
			r.Input(std::move(m));
		}
		virtual void Send(Message m) { Send(std::move(m), *m_receiver); }

		void SetDefaultReceiver(Receiver<Message>* r) { m_receiver = r; }
	private:
		Receiver<Message>* m_receiver;
	};
};

//Simple Receiving Policy, Calls directly
class SimpleReceivePolicy
{
public:
	template <typename Message>
	class ReceiveImpl : public Receiver<Message>
	{
	public:
		ReceiveImpl(SimpleReceivePolicy*) {}
		virtual void Input(Message m)
		{
			this->Receive(std::move(m));
		}
	};
	// Allow other policies to start and stop threads
	void Start() {}
	void Finish() {}
};

// Implementation for Agents
template <typename MessageType,
	 typename ReceivePolicy,
	 typename SendPolicy>
class AgentImpl {};

// Implementation of Send functionality
template <typename Message, typename ReceivePolicy, typename SendPolicy>
class AgentImpl<Send<Message>,ReceivePolicy, SendPolicy> : 
	public SendPolicy::template SendImpl<Message>
{
public:
	AgentImpl(ReceivePolicy* rp, SendPolicy* sp) {};
};

// Implementation of Receive functionality
template <typename Message, typename ReceivePolicy, typename SendPolicy>
class AgentImpl<Receive<Message>,ReceivePolicy, SendPolicy> :
	public ReceivePolicy::template ReceiveImpl<Message>
{
public: 
	AgentImpl(ReceivePolicy* rp, SendPolicy* sp):
		ReceivePolicy::template ReceiveImpl<Message>(rp)
	{};
};

// Message Listing Classes

// Tag Type for messages for deriving from
template <typename... Ts>
class Messages {
public:
	template <typename ReceivePolicy, typename SendPolicy>
	class AgentBase : public ReceivePolicy, public SendPolicy, public AgentImpl<Ts, ReceivePolicy, SendPolicy>...
	{
	public:
		AgentBase():
			ReceivePolicy(),
			SendPolicy(),
			AgentImpl<Ts, ReceivePolicy, SendPolicy>(this, this)...
		{
		}
		template <typename MessageType>
		void Send(MessageType m)
		{
			static_cast<Sender<MessageType>*const>(this)->
				Send(std::move(m));
		}

		template <typename MessageType>
		void Send(MessageType m, Receiver<MessageType>& r)
		{
			static_cast<Sender<MessageType>*const>(this)->
				Send(std::move(m), r);
		}
		template <typename MessageType>
		void SetDefaultReceiver(Receiver<MessageType>* r)
		{
			static_cast<SimpleSendPolicy::template SendImpl<MessageType>*const>(this)->
				SetDefaultReceiver(r);
		}
		template <typename MessageType>
		void Input(MessageType m)
		{
			static_cast<Receiver<MessageType>*const>(this)->
				Input(std::move(m));
		}
	};
};

// Agent Base class for users to derive from
template <typename Messages,
	 typename ReceivePolicy = SimpleReceivePolicy,
	 typename SendPolicy = SimpleSendPolicy>
class AgentBase : public Messages::template AgentBase<ReceivePolicy, SendPolicy>
{
};



}
#endif
