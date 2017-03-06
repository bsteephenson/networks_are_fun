
#include "network.h"
#include <algorithm>
#include <chrono>
#include <thread>
#include <iostream>

using namespace std;

template<class T>
Network<T>::Network(int tick_rate, int drop_rate, int max_delay) :
	time(0),
	drop_rate(drop_rate),
	max_delay(max_delay),
	time_increment_thread([this, tick_rate](){
		while(true) {
			this->m.lock();
			this->time++;
			this->time_ticked.notify_all();
			this->m.unlock();
			
			this_thread::sleep_for(chrono::milliseconds(tick_rate));
		}
	})
{}

template<class T>
Network<T>::~Network() {
	this->time_increment_thread.join();
}

template<class T>
void Network<T>::send_message(string recipient, T message) {
	int r = rand() % 100;
	int delay = rand() % max_delay;

	if (r < drop_rate) {
		return;
	}

	this->m.lock();
	int t = this->time + delay;
	this->mailboxes[recipient].push(make_tuple(t, message));
	this->time++;
	this->m.unlock();
}

template<class T>
string Network<T>::wait_for_message(string recipient) {
	while(true) {
		unique_lock<mutex> lock(m);
		time_ticked.wait(lock);
		// if my mailbox isnt empty
		if (!this->mailboxes[recipient].empty()) {
			// get the message with the lowest timestamp
			auto num_message = this->mailboxes[recipient].top();
			// if that message's timestamp is before now, it has "arrived"
			if (get<0>(num_message) <= this->time) {
				T message = get<1>(num_message);
				this->mailboxes[recipient].pop();
				this->m.unlock();
				return message;
			}
		}
	}
}

template class Network<string>;

