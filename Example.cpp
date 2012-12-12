// This is a simple example program showing two Agents interacting on different
// threads.  It is designed to show briefly what the API looks like for agents

#include <iostream>

#include "Agent.hpp"
#include "ThreadReceivePolicy.hpp"
#include "System.hpp"

// An empty struct to use as a message
struct Message1 {};

using namespace Agent;

// This agent receives a message on a different thread and prints when that message is received
class ReceivingAgent : 
	public AgentBase< // Required base class
		Messages<Receive<Message1>> // Messages and directions understood
		,ThreadReceivePolicy>	// Policy to use for implementing receive
{
private:
	// Receive method for the specified message.  Will only be called on a
	// thread owned by an instance of this class
	void Receive(Message1 m)
	{
		std::cout << "Message Received" << std::endl;
	}
};

// This agent provides an interface for sending a message
class SendingAgent : 
	public AgentBase< // Required base class
		Messages<Send<Message1>>> // Messages understood
{
public:
	// Publically visible interface for sending a message
	void ForceSend(ReceivingAgent& a) {
		Send(Message1(), a);
	}
};

struct AddMessage
{
	int a,b;
};

struct ResultMessage
{
	int result;
};

// Adds two numbers
// cheap and so can be done on the receiving thread
class Adder : public AgentBase<
	Messages<Receive<AddMessage>,
		 Send<ResultMessage>>>
{
	void Receive(AddMessage m)
	{
		ResultMessage r;
		r.result = m.a + m.b;
		Send(r);
	}
};

// Multiplies by a constant factor
// Expensive so uses a separate thread
template <int Factor>
class ConstantMultiply : public AgentBase<
	Messages<Receive<ResultMessage>,
		 Send<ResultMessage>>,
	ThreadReceivePolicy>
{
	void Receive(ResultMessage m)
	{
		m.result *= Factor;
		Send(m);
	}
};

// Define a System with an add and a multiply
typedef System<Component<Adder>,
	Component<ConstantMultiply<2>>,
	Connection<Adder, ConstantMultiply<2>, ResultMessage>,
	Input<Adder, AddMessage>,
	Output<ConstantMultiply<2>, ResultMessage>> AddSystem;

int main()
{
	// ======= System Example =========
	// Instantiate the system
	AddSystem s;

	// Print the result of the computation
	s.SetCallback([] (ResultMessage r) {std::cout << r.result << std::endl; });

	// Start the worker threads
	s.Start();

	AddMessage m;
	m.a = 2;
	m.b = 3;

	// Send the Message	
	s.Input(m);

	// ======== Manual Example ==========

	// Instansiate the two agents
	ReceivingAgent a1;
	SendingAgent a2;

	// Start the thread belonging to a1
	a1.Start();

	// Send the message
	a2.ForceSend(a1);

	// Wait for the message to be received
	sleep(1);

	// Shutdown the thread belonging to a1 and the system
	s.Finish();
	a1.Finish();
	return 0;
}

