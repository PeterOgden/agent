Prototype for Agent Implementation
==================================

This library provides a very simple implementation of communicating agents in C++.
It is designed to be as typesafe as possible and make use of only standard C++ features.
Currently it supports defining classes that send and receive individual messages with some options for using threading.
It is designed as a prototype for an API and not for serious use.

Building
--------

No Makefile is provided as the functionality only exists in headers. 
To build the example program with GCC use `g++ -pthread --std=c++0x Example.cpp`.
This code requires a compiler which supports some C++11 features including threads, lambdas and variadic templates.

Using
-----

The example shows how to use the library.
Agents are created by deriving from AgentBase with the template parameters specifying the type and direction of messages understood.
Messages are received by creating a virtual function called `Receive` which takes a message type and returns void.
Messages are send by calling `Send` with the message and an appropriate receiver.
By default messages will received by the thread that sent them.
This can be changed using the `ThreadReceivePolicy` which creates a new thread for each agent instance which handles all received messages.
