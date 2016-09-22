#ifndef P2PNODE_H
#define P2PNODE_H

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

// Keeps track of and provides new peers with list of peers currently on the network
class P2PNode {
public:
    static const unsigned short DEFPORT            = 1337;
    static const unsigned short MAX_PEERS          = 64;
    static const unsigned short MAX_STRING_SIZE    = 128;
    static const unsigned short BYTE_SIZE          = 128;
    static const unsigned long PORTMIN             = 1024;
    static const unsigned long PORTMAX             = 65535;

    P2PNode(unsigned short port) : m_acceptor(m_ioService, tcp::endpoint(tcp::v4(), port)), m_socket(m_ioService) {}

    typedef enum _MessageType {
        ADDREQUEST,             // request remote peer to add self to list of peers
        REMREQUEST,             // request remote peer to remove self from list of peers
        ADDFILEREQUEST,         // notify peers of a new file you are sharing
        REMFILEREQUEST          // notifu peers to remove a previously shared file from list
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

    void handleConnection();                                                // Handler for a connection

    void printPeers() const;                                                // Pls use brain ty
    void printAvailableFiles() const;

    const std::vector<std::string>& getPeersList() const;
    const std::map<std::string, std::string>& getAvailableList() const;
    std::string getAcceptorPort() const;

protected:
    std::string parseAddress();                                             // Parses peer addr from socket, given the next format is portSize,portString

    virtual void handleAddRequest();                                        // Connected peer wants to join network
    void sendPeersList();                                                   // Send list of peers on network to the connected peer
    void addPeer(std::string peer);                                         // If doesn't already exist, add peer address string to list

    void handleRemRequest();                                                // Connected peer wants to leave network
    void remPeerFiles(std::string peer);                                    // Removes all files that were made available by peer
    void remPeer(std::string peer);                                         // Remove peer address string from list

    void handleAddFileRequest();                                            // Connected peer wants to share a file with network
    void addAvailableFile(std::string filename, std::string address);       // Adds filename available for downloay `address`

    void sendFilesList();                                                   // Sends list of files available for download to connecting peer

	boost::asio::io_service m_ioService;
	tcp::acceptor m_acceptor;
	tcp::socket m_socket;
    std::vector<std::string> m_peersList;                                   // List of peers on network
    std::map<std::string, std::string> m_availableFilesList;                // List of files available for download by other peers
};

#endif // P2PNODE_H
