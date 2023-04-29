#include "stdafx.h"
#include <WinSock2.h>
#include "Address.h"
#include "Socket.h"


/*
* Static function to intitalize Socket capabilities
* Returns: bool - True if successfull
*/
bool Socket::InitializeSockets() {
	WSADATA wsa_data;
	return WSAStartup(MAKEWORD(2, 2), &wsa_data) == NOERROR;
}
/*
* Static function to cleanly shutdown Socket capabilities
* Returns: void
*/
void Socket::ShutdownSockets() {
	WSACleanup();
}

/*
* Creates a UDP socket
*/
Socket::Socket() {
	handle = static_cast<Uint32>( socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP) );
}

/*
* Binds a socket to a specified port number
* Returns: bool - True if successfull
*/
bool Socket::Open(Uint16 port) {
	sockaddr_in address;
	//Address family specified for IPv4
	address.sin_family = AF_INET;
	//The IP address of the localhost
	address.sin_addr.s_addr = INADDR_ANY;
	//port converted to network byte order (big endian)
	address.sin_port =
		htons((unsigned short)port);
	unsigned short short_port = port;
	if (bind(handle, (const sockaddr*)&address, sizeof(sockaddr_in)) != 0) {
		SIK_ERROR("Socket bind failed with : {}", WSAGetLastError());
		return false;
	}

	//Set the socket to non-blocking
	DWORD nonBlocking = 1;
	if (ioctlsocket(handle, FIONBIO, &nonBlocking) != 0) {
		SIK_ERROR("Socket ioctl call failed with : {}", WSAGetLastError());
		return false;
	}

	return true;
}

/*
* Closes an open socket
*/
void Socket::Close() {
	closesocket(handle);
}

/*
* Send some data to the specified destination address
* Returns: bool - True if successfull
*/
bool Socket::Send(const Address& destination, const void* data, int size) {
	//Create the required struct for the destination address
	sockaddr_in send_addr;
	send_addr.sin_family = AF_INET;
	send_addr.sin_port = htons(destination.GetPort());
	send_addr.sin_addr.s_addr = htonl(destination.GetAddress());

	int res = sendto(handle,
		(const char*)data, size, 0,
		(SOCKADDR*)&send_addr, sizeof(send_addr));

	if (res == SOCKET_ERROR) {
		SIK_ERROR("Sending datagram failed with : {}", WSAGetLastError());
		return false;
	}
	return true;
}

/*
* Method to check if there are any packets to read
* that were sent to the sockets port number and then
* read the contents of the packet.
* Stores the address of the sender in sender.
* Returns the number of bytes read per packet
*/
int Socket::Receive(Address& sender, void* buffer, int buffer_len) {
	sockaddr_in from_addr;
	int from_size = sizeof(from_addr);
	
	int bytes_received = recvfrom(handle, (char*)buffer, buffer_len, 0,
								 (SOCKADDR*)&from_addr, &from_size);
	if (bytes_received == SOCKET_ERROR) {
		//Don't worry about the non-blocking returns
		if (WSAGetLastError() == WSAEWOULDBLOCK) {
			return 0;
		}
		else {
			SIK_ERROR("Receiving datagram failed with : {}", WSAGetLastError());
			return SOCKET_ERROR;
		}
	}
	else if (bytes_received == 0)
		return bytes_received;

	//We received a packet containing data
	//Process the sender IP and port number
	unsigned long from_address = ntohl(from_addr.sin_addr.s_addr);
	sender.SetAddress(from_address);

	unsigned int from_port = ntohs(from_addr.sin_port);
	sender.SetPort(from_port);

	return bytes_received;
}


