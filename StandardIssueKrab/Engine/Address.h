#pragma once
class Address {
public:
	Address() = default;
	
	/*
	* Create an Ipaddress from 4 unit8 values
	*/
	Address(Uint8 a,
		Uint8 b,
		Uint8 c,
		Uint8 d,
		Uint16 _port);

	/*
	* Create an IP address from a full 32bit Uint
	*/
	Address(Uint32 _ip_addr, Uint16 _port);

	//Getter for IP address
	Uint32 GetAddress() const;
	//Getter for port number
	Uint16 GetPort() const;

	//Setter for IP address
	void SetAddress(Uint32 ip_addr);
	//Setter for port
	void SetPort(Uint16 port);
private:
	Uint32 ip_addr;
	Uint16 port;
};

