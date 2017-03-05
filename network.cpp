
#include "network.h"
#include <algorithm>
#include <chrono>
#include <thread>
#include <iostream>

using namespace std;

void Network::send_message(string recipient, string message) {
	int r = rand() % 100;

	if (r > 75) {
		return;
	}

	this->m.lock();
	int t = this->time + r;
	this->mailboxes[recipient].push(make_tuple(t, message));
	this->time++;
	this->m.unlock();
}

string Network::wait_for_message(string recipient) {
	string message;
	thread t([&](){
		while(true) {
			this->m.lock();
			if (!this->mailboxes[recipient].empty()) {

				auto num_message = this->mailboxes[recipient].top();
				if (get<0>(num_message) <= this->time) {
					message = get<1>(num_message);
					this->mailboxes[recipient].pop();
					this->m.unlock();
					return;
				}
			}

			this->time++;

			this->m.unlock();
			this_thread::sleep_for(chrono::milliseconds(10));

		}
	});
	t.join();
	return message;
}
