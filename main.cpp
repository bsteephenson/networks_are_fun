
#include <iostream>
#include <thread>
#include <chrono>

#include "network.h"

using namespace std;

void machine1(Network<string>* network) {
	int i = 0;
	while(i < 1000) {
		cout << "Sending message:" << i << endl;
		network->send_message("machine2", "Message: " + to_string(i));
		// this_thread::sleep_for(chrono::milliseconds(10));
		i++;
	}
}

void machine2(Network<string>* network) {
	while(true) {
		cout << network->wait_for_message("machine2") << endl;
	}
}

int main() {

	Network<string> network(1, 50, 10);

	std::thread t1(machine1, &network);
	std::thread t2(machine2, &network);

	t1.join();
	t2.join();

}
