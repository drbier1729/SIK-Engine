#include "stdafx.h"
#include "PacketSendRecvTest.h"
#include "Engine/InputManager.h"

void PacketSendRecvTest::Setup(EngineExport* _p_engine_export_struct) {
	Socket::InitializeSockets();

	recv_addr = Address(127, 0, 0, 1, 30000);
	SIK_INFO("Opening a socket with port number : {}", recv_addr.GetPort());
    socket = Socket();
	socket.Open(recv_addr.GetPort());

    recv_counter = 0;
    
    p_input_manager = _p_engine_export_struct->p_engine_input_manager;

    SetRunning();
}
/*
* Runs the Packet test
* Steps:
* 1) Send a packet in an input action is performed
* 2) If a packet is received then add to buffer
* 3) When buffer reaches 5 packets then unpack and process
* Returns: void
*/
void PacketSendRecvTest::Run() {
	const char up_data[] = "UP";
    const char down_data[] = "DOWN";
    const char left_data[] = "LEFT";
    const char right_data[] = "RIGHT";
    const char end_data[] = "END";

    if (input_action.IsActionTriggered(InputAction::Actions::UP)) {
        SIK_INFO("Sending UP packet");
        socket.Send(recv_addr, up_data, sizeof(up_data));
    }
	    
    if (input_action.IsActionTriggered(InputAction::Actions::DOWN)) {
        SIK_INFO("Sending DOWN packet");
        socket.Send(recv_addr, down_data, sizeof(down_data));
    }
        
    if (input_action.IsActionTriggered(InputAction::Actions::RIGHT)) {
        SIK_INFO("Sending RIGHT packet");
        socket.Send(recv_addr, right_data, sizeof(right_data));
    }
        
    if (input_action.IsActionTriggered(InputAction::Actions::LEFT)) {
        SIK_INFO("Sending LEFT packet");
        socket.Send(recv_addr, left_data, sizeof(left_data));
    }
        
    if (input_action.IsActionTriggered(InputAction::Actions::ACTION_SELECT)) {
        SIK_INFO("Sending END packet");
        socket.Send(recv_addr, end_data, sizeof(end_data));
    }

    Address sender;
    unsigned char buffer[8] = {0};
    int buf_size = sizeof(buffer);
    int bytes_read = socket.Receive(sender, buffer, sizeof(buffer));
    if (bytes_read > 0) {
        for (Uint8 i = 0; i < sizeof(buffer); i++) {
            recv_buffer.push_back(buffer[i]);
        }
        recv_buffer.push_back('_');
        recv_counter++;
    }
    
    if (recv_counter == 5) {
        recv_counter = 0;
        SIK_INFO("Reached 5 messages. Unpacking the buffer");
        String output_string;
        for (const auto& c : recv_buffer) {
            output_string += c;
        }
        SIK_INFO("Received data : {}", output_string);

        if (output_string.find("END") != std::string::npos) {
            SIK_INFO("Received END message. Ending test.");
            SetPassed();
            return;
        }
        recv_buffer.clear();
    }
    
    SetRunning();
}

/*
* Runs the teardown
* Closes the Socket and De-intializes windows sockets
* Returns: void
*/
void PacketSendRecvTest::Teardown() {
    socket.Close();
    Socket::ShutdownSockets();
}
