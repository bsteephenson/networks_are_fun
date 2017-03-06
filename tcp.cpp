
#include <tcp.h>

using namespace std;

enum Sending_State
{
	CLOSED, // intial and final states
	SYN_SENT, // after having sent a syn to the server
	ESTABLISHED, // after the server sends back an acknowledgement for the syn
	FIN_WAIT1, // after having sent a fin to the server
	FIN_WAIT2, // after the server sends back an acknowledgement for the fin
	TIME_WAIT, // waiting for the server to send me a fin
};

TCP::TCP(Network<TCP_Packet>* network) : network(network) {}

string TCP::wait_for_message() {
	unique_lock<mutex> lock(mu);
		
	Sending_State state = CLOSED;

	while (buffer.empty()) {
		buffer_not_empty.wait(lock);

		if (buffer.empty()) {
			continue;
		}

	}
}

void TCP::send_message(string recipient, string message) {

	TCP_Packet syn_ack_packet;

	auto got_syn_ack_check = [](TCP_Packet p){
		return (p.syn && p.ack);
	};

	do {
		// todo send a message
		TCP_Packet p;
		p.syn = true;
		p.seq = 40000;
		network->send_message(recipient, p);

	} while(wait_for_relevant_packet(5, &syn_ack_packet, got_syn_ack_check));



}

template<T>
bool TCP::wait_for_relevant_packet(int attempts, TCP_Packet* p, T lambda) {
	
	while(attempts > 0) {

		unique_lock<mutex> lock(mu);
		
		while (buffer.empty()) {
			buffer_not_empty.wait(lock);

			if (buffer.empty()) {
				continue;
			}
			else {
				break;
			}

		}

		if (T(buffer.front())) {
			*p = buffer.front();
			buffer.pop_front();
			return true;
		}
		else {
			attempts--;			
		}

	}

}