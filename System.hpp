// Implementation for a System of communicating agents
// Constructs and wires up agents as specified in the template arguments


namespace Agent
{
// Structs used to describe elements of a system
template <typename Agent> class Component {};
template <typename FromAgent, typename ToAgent, typename Message> class Connection {};
template <typename Agent, typename Message> class Input {};
template <typename Agent, typename Message> class Output {};

// Component which calls a callback for each received message
template <typename Message>
class OutputComponent : public AgentBase<Messages<Receive<Message>>>
{
public:
	std::function<void (Message)> m_callback;
	void SetCallback(const std::function<void (Message)>& f)
	{
		m_callback = f;
	}
	virtual void Receive(Message m) { m_callback(std::move(m)); }
};

// General template for implementing each part of the system
template <typename Detail, typename System> class SystemImpl {};

// Instantiates a component and starts and stops the component
template <typename Agent, typename System>
class SystemImpl<Component<Agent>, System>
{
public:
	void Start() { m_agent.Start(); }
	void Finish() { m_agent.Finish(); }
	Agent m_agent;
};

// Connects two components together when constructed
template <typename FromAgent, typename ToAgent, typename Message, typename System>
class SystemImpl<Connection<FromAgent, ToAgent, Message>, System>
{
public:
	SystemImpl() {
		static_cast<SystemImpl<Component<FromAgent>, System>* const>(
			static_cast<System* const>(this))->m_agent.
			SetDefaultReceiver(
				&static_cast<SystemImpl<Component<ToAgent>, System>* const>(static_cast<System* const>(this))->m_agent);
	};
	void Start() {}
	void Finish() {}
};

// Passes through any created messages
template <typename Agent, typename Message, typename System>
class SystemImpl<Input<Agent, Message>, System>
{
public:
	void Input(Message m)
	{
		static_cast<SystemImpl<Component<Agent>, System>* const>(
			static_cast<System*const>(this))->m_agent.Input(std::move(m));
	}
	void Start() {}
	void Finish() {}
};

// Creates and wires up an OutputComponent
template <typename Agent, typename Message, typename System>
class SystemImpl<Output<Agent, Message>, System>
{
public:
	SystemImpl() {
		static_cast<SystemImpl<Component<Agent>,System>* const>(
			static_cast<System* const>(this))->m_agent.
			SetDefaultReceiver(&m_component);
	}
	void SetCallback(const std::function<void(Message)>& callback)
	{ m_component.SetCallback(callback); }

	void Start() {}
	void Finish() {}
private:
	OutputComponent<Message> m_component;
};

// System description.  Please note that Components must be
// declared before any connections
template <typename... Details>
class System : public SystemImpl<Details, System<Details...>>...
{
public:
	// TODO: Find a way of doing this which doesn't involve
	// explicit template looping
	template <typename... Ts> class StartLoop
	{
	public:
		static void Start(System<Details...>* s) {}
		static void Finish(System<Details...>* s) {}
	};

	template <typename T, typename... Ts>
	class StartLoop<T, Ts...>
	{
	public:
		static void Start(System<Details...>* s)
		{
			static_cast<SystemImpl<T, System<Details...>>*>(s)->Start();
			StartLoop<Ts...>::Start(s);
		}
		static void Finish(System<Details...>* s)
		{
			static_cast<SystemImpl<T, System<Details...>>*>(s)->Finish();
			StartLoop<Ts...>::Finish(s);
		}
	};


	void Start()
	{
		StartLoop<Details...>::Start(this);
	}
	void Finish() {
		StartLoop<Details...>::Finish(this);
	}
};


}

