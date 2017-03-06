#ifndef TCP_H
#define TCP_H

#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>


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
	std::vector<std::string> payload;

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
	TCP(Network<TCP_Packet>* network);
	std::vector<std::string> wait_for_message();
	void send_message(std::string recipient, std::vector<std::string> message);
private:
	Network<TCP_Packet>* network;
	std::deque<std::tuple<std::string, std::string>> buffer;
	std::mutex mu;
	std::condition_variable buffer_not_empty;
	template<T> bool wait_for_relevant_packet(int attempts, T lambda);

};

#endif