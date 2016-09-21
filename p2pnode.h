#ifndef P2PNODE_H
#define P2PNODE_H

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

// Keeps track of and provides new peers with list of peers currently on the network
class P2PNode {
public:
    P2PNode(unsigned short port) : m_acceptor(m_ioService, tcp::endpoint(tcp::v4(), port)), m_socket(m_ioService) {}
    ~P2PNode();

    static const unsigned short DEFPORT            = 1337;
    static const unsigned short MAX_PEERS          = 64;
    static const unsigned short MAX_STRING_SIZE    = 128;
    static const unsigned short BYTE_SIZE          = 128;
    static const unsigned long PORTMIN             = 1024;
    static const unsigned long PORTMAX             = 65535;

	typedef enum _MessageType {
		ADDREQUEST,             // request remote peer to add self to list of peers
		REMREQUEST              // request remote peer to remove self from list of peers
	} MessageType;

	// Performs port-specific error checking and converts string to ushort
	static unsigned short strToPort(char* portStr) {
		unsigned long tmpPort = std::strtoul(portStr, NULL, 10);
		if (tmpPort >= PORTMIN && tmpPort <= PORTMAX && tmpPort <= USHRT_MAX) return static_cast<unsigned short>(tmpPort);
		else return 0;
    }

    // Returns address string from "address:port" format
    static std::string addressFromString(std::string peerString) {
        std::string delimiter = ":";
        return peerString.substr(0, peerString.find(delimiter));
    }

    // Returns port string from "address:port" format
    static std::string portFromString(std::string peerString) {
        std::string delimiter = ":";
        return peerString.substr(peerString.find(delimiter) + 1, peerString.size() - peerString.find(delimiter) - 1);
    }

    void handleConnection();                                    // Handler for a connection

    virtual void handleAddRequest();                            // Connected peer wants to join network
    void handleRemRequest();                                    // Connected peer wants to leave network

    std::string parseAddress();                                 // Read a peer's address from socket
    void sendPeersList();                                       // Send list of peers on network to the connected peer

    void addPeer(std::string peer);                             // If doesn't already exist, add peer address string to list
    void remPeer(std::string peer);                             // Remove peer address string from list

    void closeConnection();                                     // Closes connection on m_socket

    void printPeers();                                          // Pls use brain ty

    const std::vector<std::string>& getPeersList() const;
    std::string getAcceptorPort() const;
protected:
	boost::asio::io_service m_ioService;
	tcp::acceptor m_acceptor;
	tcp::socket m_socket;
    std::vector<std::string> m_peersList;
};

#endif // P2PNODE_H
