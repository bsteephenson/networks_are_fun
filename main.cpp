
#include <iostream>
#include <thread>
#include <chrono>

#include "network.h"

using namespace std;

void machine1(Network* network) {
	int i = 0;
	while(true) {
		cout << "Sending message:" << i << endl;
		network->send_message("machine2", "Message: " + to_string(i));
		this_thread::sleep_for(chrono::milliseconds(100));
		i++;
	}
}

void machine2(Network* network) {
	while(true) {
		cout << network->wait_for_message("machine2") << endl;
	}
}

int main() {

	Network network;

	std::thread t1(machine1, &network);
	std::thread t2(machine2, &network);

	t1.join();
	t2.join();

}
