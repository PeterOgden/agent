// This is a simple example program showing two Agents interacting on different
// threads.  It is designed to show briefly what the API looks like for agents

#include <iostream>

#include "Agent.hpp"
#include "ThreadReceivePolicy.hpp"

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

int main()
{
	// Instansiate the two agents
	ReceivingAgent a1;
	SendingAgent a2;

	// Start the thread belonging to a1
	a1.Start();

	// Send the message
	a2.ForceSend(a1);

	// Wait for the message to be received
	sleep(1);

	// Shutdown the thread belonging to a1
	a1.Finish();
	return 0;
}

