#ifndef NETWORK_H
#define NETWORK_H

#include <map>
#include <vector>
#include <tuple>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>

template<class T>
class CompareTuples {
public:
	bool operator()(std::tuple<int, T> t1, std::tuple<int, T> t2) {
		return std::get<0>(t1) > std::get<0>(t2);		
	}
};

template<class T>
class Network {
public:
	// tick_rate is milliseconds it takes to increment the clock
	// drop_rate is the percent of messages dropped
	// max_delay is the max number of ticks a message takes to arrive
	Network(int tick_rate, int drop_rate, int max_delay);
	~Network();
	void send_message(std::string recipient, T message);
	bool wait_for_message(std::string recipient, int timeout, T* message);
private:
	std::mutex m;
	std::condition_variable time_ticked;
	int time;
	int drop_rate;
	int max_delay;

	bool stop;

	std::thread time_increment_thread;

	std::map<std::string, std::priority_queue<std::tuple<int, T>, std::vector<std::tuple<int, T>>,  CompareTuples<T>>> mailboxes;
};

#endif