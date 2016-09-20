#include "p2pnode.h"

int main(int argc, char** argv) {
	// assign either provided port or default port
	unsigned short port = P2PNode::DEFPORT;
	if (argc == 2) {
		port = P2PNode::strToPort(argv[1]);
		if (port == 0) { std::cerr << "Invalid port." << std::endl; return EXIT_FAILURE; }
	}
	port = P2PNode::DEFPORT;	// remove once client can choose port for manager
	
	P2PNode connectionManager(port);
	std::cout << "Running on port " << port << "\n" << std::endl;

	for (;;) { connectionManager.handleConnection(); connectionManager.printPeers(); }
}

