#include <iostream>
#include <cassert>

#include "Agent.hpp"
#include "System.hpp"

#include "NonReentrantPolicy.hpp"

using namespace Agent;

struct InputMessage
{
	int value;
};

struct ProcessingMessage
{
	int next;
	int accum;
};

struct OutputMessage
{
	int value;
};

class FactorialAgent : public AgentBase<
	Messages<Receive<InputMessage>,
	Receive<ProcessingMessage>,
	Send<ProcessingMessage>,
	Send<OutputMessage>>,
	NonReentrantPolicy>
{
public:
	FactorialAgent() : reentrantCount(0) {}
private:
	int reentrantCount;
	void Receive(InputMessage m)
	{
		++reentrantCount;
		ProcessingMessage pm = {m.value, 1};
		Send(pm);
		assert(reentrantCount == 1);
		--reentrantCount;
	}
	void Receive(ProcessingMessage pm)
	{
		++reentrantCount;
		if (pm.next == 0)
		{
			OutputMessage om = {pm.accum};
			Send(om);
		}
		else
		{
			pm.accum = pm.accum * pm.next;
			--pm.next;
			Send(pm);
		}
		assert(reentrantCount == 1);
		--reentrantCount;
	}
};

typedef System<Component<FactorialAgent>,
	Connection<FactorialAgent,FactorialAgent,ProcessingMessage>,
	Input<FactorialAgent,InputMessage>,
	Output<FactorialAgent,OutputMessage>> FactorialSystem;

int main()
{
	FactorialSystem s;
	s.Start();
	s.SetCallback([] (OutputMessage m) {std::cout << m.value << std::endl;});

	InputMessage m = {5};
	s.Input(m);

	s.Finish();
}
