#pragma once

class Socket {
public:
	/*
	* Static function to intitalize Socket capabilities
	* Returns: bool - True if successfull
	*/
	static bool InitializeSockets();

	/*
	* Static function to cleanly shutdown Socket capabilities
	* Returns: void
	*/
	static void ShutdownSockets();

	/*
	* Creates a UDP socket
	*/
	Socket();

	~Socket() = default;

	/*
	* Binds a socket to a specified port number
	* Returns: bool - True if successfull
	*/
	bool Open(Uint16 port);

	void Close();

	bool IsOpen() const;

	/*
	* Send some data to the specified destination address
	* Returns: bool - True if successfull
	*/
	bool Send(const Address& destination, const void* data,
		int size);

	/*
	* Method to check if there are any packets to read 
	* that were sent to the sockets port number and then
	* read the contents of the packet.
	* Returns the number of bytes read per packet
	*/
	int Receive(Address& sender, void* buffer, int buffer_len);
private:
	Uint32 handle;
};

