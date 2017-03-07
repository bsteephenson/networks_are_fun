
#include "tcp.h"
#include <stdlib.h>
#include <iostream>


using namespace std;

TCP::TCP(Network<TCP_Packet>* network, string machine)
	: network(network), have_packet(false), machine(machine),
	  listen_thread([this](){ this->listen_loop(); }) {}

TCP::~TCP() {
	listen_thread.join();
}

vector<string> TCP::wait_for_message() {
	
	TCP_Packet client_syn_packet;
	TCP_Packet syn_ack_packet;

	vector<string> messages;

	auto wait_for_syn_lambda = [&](TCP_Packet p){
		return (p.syn && !p.ack);
	};

	while (wait_for_relevant_packet(INT_MAX, &client_syn_packet, wait_for_syn_lambda)) {

		cout << "2. Server received init" << endl;

		int my_seq_num = rand(); // in real life a timestamp
		syn_ack_packet.syn = true;
		syn_ack_packet.ack = true;
		syn_ack_packet.ack_num = client_syn_packet.seq_num;
		syn_ack_packet.seq_num = my_seq_num;
		syn_ack_packet.sender = machine;

		string client_machine = client_syn_packet.sender;

		network->send_message(client_machine, syn_ack_packet);

		if (!wait_for_relevant_packet(5000, NULL, [&](TCP_Packet p) {
				return (!p.syn && p.ack_num == my_seq_num && p.sender == client_machine);
			})) {
			cout << "Server timed out waiting for ack for its init ack" << endl;
			continue;
		}

		cout << "4. Server connection established" << endl;


		// At this point we've estalished a connection!
		// (Ignored the prior packet because coding laziness + the client will send it again anyways)
		
		TCP_Packet packet;
		int timeout = INT_MAX;
		while (wait_for_relevant_packet(timeout, &packet, [&](TCP_Packet p){ return !p.syn && p.sender == client_machine; })) {

			cout << "6. Server got a packet" << endl;

			// if this is a packet we've never seen before, update my_seq_num
			if (packet.ack_num == my_seq_num) {
				my_seq_num++;
				if (!packet.fin) {
					messages.push_back(packet.payload);
				}
			}

			TCP_Packet response;
			response.ack = true;
			response.ack_num = packet.seq_num;
			response.seq_num = packet.ack_num + 1;
			response.sender = machine;
			network->send_message(packet.sender, response);			


			if (packet.fin) {
				// lower the timeout so the client gets the chance to send another fin in case it didn't get my ack
				timeout = 42;
				cout << "9. Server recieved FIN" << endl;
			}

		}
		return messages;
	}

}

void TCP::send_message(string recipient, vector<string> messages) {


	TCP_Packet init_syn_packet;
	TCP_Packet response;

	init_syn_packet.syn = true;
	init_syn_packet.ack = false;
	init_syn_packet.sender = machine;

	int my_seq_num = rand(); // in real life this would be the timestamp
	init_syn_packet.seq_num = my_seq_num;

	do {
		cout << "1. Client attempt init" << endl;
		network->send_message(recipient, init_syn_packet);

	} while (!wait_for_relevant_packet(50, &response, [&](TCP_Packet q){
		cout << "Client got ack:" << q.ack_num << endl;
		return (q.syn && q.ack && (q.ack_num == init_syn_packet.seq_num) && q.sender == recipient);
	}));

	cout << "3. Client received ack for init" << endl;

	my_seq_num++;

	for (string s : messages) {
		TCP_Packet packet;
		packet.ack = true;
		packet.ack_num = response.seq_num;
		packet.seq_num = my_seq_num;
		packet.payload = s;
		packet.sender = machine;
		

		do {
			cout << "5. Client attempt sent packet" << endl;
			network->send_message(recipient, packet);
		} while (!wait_for_relevant_packet(500, &response, [&](TCP_Packet p){
			return (!p.syn && p.ack_num == packet.seq_num && p.sender == recipient);
		}));

		cout << "7. Client successfully sent packet" << endl;


		my_seq_num++;
	}

	// End the conection

	TCP_Packet packet;
	packet.ack = true;
	packet.ack_num = response.seq_num;
	packet.seq_num = my_seq_num;
	packet.fin = true;
	packet.sender = machine;
	
	int attempts = 10;
	do {
		attempts--;
		cout << "8. Client sends server FIN" << endl;
		network->send_message(recipient, packet);
	} while (attempts > 0 && !wait_for_relevant_packet(50, &response, [&](TCP_Packet p){
		return (!p.syn && p.ack_num == packet.seq_num && p.sender == recipient);
	}));

	if (attempts > 0) {
		cout << "10. Client recieved ACK for FIN" << endl;
	}
	else {
		cout << "10.b. Client never recieved an ACK for FIN but quit anyways" << endl;
	}


}

template<typename T>
bool TCP::wait_for_relevant_packet(int attempts, TCP_Packet* p, T lambda) {
	
	while(attempts > 0) {

		unique_lock<mutex> lock(mu);
		
		if (!have_packet) {
			recieved_packet.wait(lock);
		}



		if (have_packet && lambda(last_packet)) {
			have_packet = false;
			if (p != NULL) {
				*p = last_packet;			
			}
			return true;
		}

		else {
			attempts--;
		}
	}

	return false;

}

void TCP::listen_loop() {
	while (true) {
		TCP_Packet packet;
		if(network->wait_for_message(machine, 1, &packet)) {
			mu.lock();
			last_packet = packet;
			have_packet = true;
			mu.unlock();
		}

		recieved_packet.notify_all();

	}
}
