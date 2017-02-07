#include <iostream>
#include "p2pnode.h"

int main(int argc, char** argv) {
	unsigned short port = argc == 2 ? P2PNode::strToPort(argv[1]) : P2PNode::DEFPORT;
	if (port == 0) { std::cerr << "Invalid port." << std::endl; return EXIT_FAILURE; }
	
	P2PNode connectionManager(port);
	std::cout << "Running on port " << port << "\n" << std::endl;

	for (;;) connectionManager.handleConnection();
}

