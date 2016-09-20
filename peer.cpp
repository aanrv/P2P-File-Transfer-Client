#include "peer.h"

using boost::asio::ip::tcp;

Peer::~Peer() {

}

void Peer::joinNetwork(std::string server, std::string service) {
    try {
        tcp::socket tmpSocket(m_ioService);
        tcp::resolver::query query(server, service);
        tcp::resolver::iterator endpoint_iterator = m_resolver.resolve(query);

        boost::asio::connect(tmpSocket, endpoint_iterator); // connect to connection manager
        sendAddRequest(tmpSocket);                          // ask connection manager to add peer list
        retreivePeersList(tmpSocket);                       // read the current list of peers from connection manager
        connectToPeers();

    } catch(std::exception& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << "Peer::joinNetwork(): Make sure the connection manager is running." << std::endl;

        throw e;
    }
}

void Peer::sendAddRequest(tcp::socket& tmpSocket) {
    try {
        // send add request to peer
        char messageType = static_cast<char>(P2PNode::ADDREQUEST);
        boost::asio::write(tmpSocket, boost::asio::buffer(&messageType, 1));            // send message type

        // send listening port, peer will extract ip on its own
        std::string portString = std::to_string(m_acceptor.local_endpoint().port());
        char addressSize = static_cast<char>(portString.size());
        boost::asio::write(tmpSocket, boost::asio::buffer(&addressSize, 1));            // send size of port;
        boost::asio::write(tmpSocket, boost::asio::buffer(portString));                 // send port
    } catch (std::exception& e) {
        std::cerr << "Peer::sendAddRequest()" << e.what() << std::endl;
    }
}

void Peer::connectToPeers() {
    for (std::vector<std::string>::iterator it = m_peersList.begin(); it != m_peersList.end(); ++it) {
        std::string currentPeerAddress = P2PNode::addressFromString(*it);
        std::string currentPeerPort = P2PNode::portFromString(*it);

        try {
            tcp::socket tmpSocket(m_ioService);
            tcp::resolver::query query(currentPeerAddress, currentPeerPort);
            tcp::resolver::iterator endpoint_iterator = m_resolver.resolve(query);

            boost::asio::connect(tmpSocket, endpoint_iterator);
            sendAddRequest(tmpSocket);

        } catch (std::exception& e) {
            std::cerr << "Peer::connectToPeers(): " << e.what() << std::endl;
            throw e;
        }
    }
}

void Peer::handleAddRequest() {
    std::cout << "Peer: Add request" << std::endl;
    std::string addressString = parseAddress();	// parse address from socket
    addPeer(addressString);                     // add address to list
}

void Peer::retreivePeersList(tcp::socket &tmpSocket) {
    try {
        m_peersList.clear();
        // read first byte to see how many peers will be read
        boost::array<char, 1> peerCount;
        boost::asio::read(tmpSocket, boost::asio::buffer(peerCount), boost::asio::transfer_exactly(1));

        // read peerCount many addresses
        for (short i = 0; i < static_cast<short>(peerCount[0]); ++i) {
            // read first byte for size of string
            boost::array<char, 1> strSize;
            boost::asio::read(tmpSocket, boost::asio::buffer(strSize), boost::asio::transfer_exactly(1));

            boost::array<char, MAX_STRING_SIZE> buffer;
            boost::asio::read(tmpSocket, boost::asio::buffer(buffer), boost::asio::transfer_exactly(static_cast<size_t>(strSize[0])));
            std::string readString(buffer.begin(), buffer.begin() + static_cast<short>(strSize[0]));
            addPeer(readString);
        }
    } catch (std::exception& e) {
        std::cerr << "Peer::retreivePeersList(): " << e.what() << std::endl;
    }
}
