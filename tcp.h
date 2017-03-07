#ifndef TCP_H
#define TCP_H

#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "network.h"
#include <string>

struct TCP_Packet {
	std::string sender;
	int seq_num;
	int ack_num;
	bool urg;
	bool ack;
	bool psh;
	bool rst;
	bool syn;
	bool fin;
	std::string payload;

	TCP_Packet() {
		urg = false;
		ack = false;
		psh = false;
		rst = false;
		syn = false;
		fin = false;
	}
};

class TCP {
public:
	TCP(Network<TCP_Packet>* network, std::string machine);
	~TCP();
	std::vector<std::string> wait_for_message();
	void send_message(std::string recipient, std::vector<std::string> message);

private:
	Network<TCP_Packet>* network;
	std::string machine;

	TCP_Packet last_packet;
	bool have_packet;
	std::mutex mu;
	std::condition_variable recieved_packet;

	template<typename T> bool wait_for_relevant_packet(int attempts, TCP_Packet* p, T lambda);

	void listen_loop();
	std::thread listen_thread;

};

#endif