
#include <iostream>
#include <thread>
#include <chrono>

#include "network.h"
#include "tcp.h"
#include <vector>

using namespace std;

void machine1(Network<TCP_Packet>* network) {
	int i = 0;
	// while(i < 1000) {
	// 	cout << "Sending message:" << i << endl;
	// 	network->send_message("machine2", "Message: " + to_string(i));
	// 	// this_thread::sleep_for(chrono::milliseconds(10));
	// 	i++;
	// }

	TCP conn(network, "machine1");
	conn.send_message("machine2", {"Dunder", "Mifflin", "this", "is", "Kevin"});

}

void machine2(Network<TCP_Packet>* network) {
	
	TCP conn(network, "machine2");
	vector<string> m = conn.wait_for_message();
	for (string s : m) {
		cout << s << endl;
	}
}

int main() {

	Network<TCP_Packet> network(1, 50, 50);

	std::thread t1(machine1, &network);
	std::thread t2(machine2, &network);

	t1.join();
	t2.join();

}
