#pragma once
#include "Test.h"

#include "Engine/Address.h"
#include "Engine/Socket.h"
#include "Engine/InputAction.h"

class PacketSendRecvTest : public Test {
public:
	virtual ~PacketSendRecvTest() = default;
	/*
	* Sets up the Packet test.
	* Initializes the Socket
	* Returns: void
	*/
	void Setup(EngineExport * _p_engine_export_struct) override;

	/*
	* Runs the Packet test
	* Steps:
	* 1) Send a packet in an input action is performed
	* 2) If a packet is received then print out the contents
	* Returns: void
	*/
	void Run() override;

	/*
	* Runs the teardown
	* Closes the Socket and De-intializes windows sockets
	* Returns: void
	*/
	void Teardown() override;
private:
	Socket socket;
	Address recv_addr;
	InputAction input_action{"default"};
	int recv_counter;
	std::vector<char> recv_buffer;
};

