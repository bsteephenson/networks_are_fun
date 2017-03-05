#ifndef NETWORK_H
#define NETWORK_H

#include <map>
#include <vector>
#include <tuple>
#include <queue>
#include <mutex>

class Network {
public:
	Network() {
		time = 0;
	}
	void send_message(std::string recipient, std::string message);
	std::string wait_for_message(std::string recipient);
private:
	std::mutex m;
	std::map<std::string, std::priority_queue<std::tuple<int, std::string>, std::vector<std::tuple<int, std::string>>,  std::greater<std::tuple<int, std::string>>>> mailboxes;
	int time;
};

#endif