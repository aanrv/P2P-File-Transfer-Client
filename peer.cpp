#include "peer.h"
#include <boost/filesystem/path.hpp>

using boost::asio::ip::tcp;

Peer::~Peer() {
}

void Peer::joinNetwork() {
    try {
        tcp::socket tmpSocket(m_ioService);
        tcp::resolver::query query(m_connectionManagerAddress, m_connectionManagerPort);
        tcp::resolver::iterator endpoint_iterator = m_resolver.resolve(query);

        boost::asio::connect(tmpSocket, endpoint_iterator); // connect to connection manager
        sendAddRequest(tmpSocket);                          // ask connection manager to add peer list
        retrievePeersList(tmpSocket);                       // read the current list of peers from connection manager
        retrieveFilesList(tmpSocket);                       // read the current list of files available for download
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

void Peer::retrievePeersList(tcp::socket &tmpSocket) {
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
        std::cerr << "Peer::retrievePeersList(): " << e.what() << std::endl;
    }
}

void Peer::retrieveFilesList(tcp::socket& tmpSocket) {
    try {
        m_availableFilesList.clear();
        // read first byte to see how many peers will be read
        boost::array<char, 1> fileCount;
        boost::asio::read(tmpSocket, boost::asio::buffer(fileCount), boost::asio::transfer_exactly(1));

        // read peerCount many addresses
        for (short i = 0; i < static_cast<short>(fileCount[0]); ++i) {
            // read first byte for size of file string
            boost::array<char, 1> strSize;
            boost::asio::read(tmpSocket, boost::asio::buffer(strSize), boost::asio::transfer_exactly(1));
            // read file str
            boost::array<char, MAX_STRING_SIZE> buffer;
            boost::asio::read(tmpSocket, boost::asio::buffer(buffer), boost::asio::transfer_exactly(static_cast<size_t>(strSize[0])));
            std::string readString(buffer.begin(), buffer.begin() + static_cast<short>(strSize[0]));

            // read first byte for size of addr string
            boost::array<char, 1> addrSize;
            boost::asio::read(tmpSocket, boost::asio::buffer(addrSize), boost::asio::transfer_exactly(1));
            // read addr str
            boost::array<char, MAX_STRING_SIZE> addrBuffer;
            boost::asio::read(tmpSocket, boost::asio::buffer(addrBuffer), boost::asio::transfer_exactly(static_cast<size_t>(addrSize[0])));
            std::string readAddrString(addrBuffer.begin(), addrBuffer.begin() + static_cast<short>(addrSize[0]));

            addAvailableFile(readString, readAddrString);
        }
    } catch (std::exception& e) {
        std::cerr << "Peer::retrieveFilesList(): " << e.what() << std::endl;
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
        }
    }
}

void Peer::leaveNetwork() {
    try {
        tcp::socket tmpSocket(m_ioService);
        tcp::resolver::query query(m_connectionManagerAddress, m_connectionManagerPort);
        tcp::resolver::iterator endpoint_iterator = m_resolver.resolve(query);

        try {
            boost::asio::connect(tmpSocket, endpoint_iterator); // connect to connection manager
            sendRemRequest(tmpSocket);                          // ask connection manager to remove from peer list
        } catch (std::exception& e) {
            std::cerr << "Peer::leaveNetwork(): Unable to send remove request to connection manager." << std::endl;
        }

        disconnectFromPeers();

    } catch(std::exception& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << "Peer::leaveNetwork(): Make sure the connection manager is running." << std::endl;
    }
}

void Peer::sendRemRequest(tcp::socket& tmpSocket) {
    try {
        // write remove request to socket
        char messageType = static_cast<char>(P2PNode::REMREQUEST);
        boost::asio::write(tmpSocket, boost::asio::buffer(&messageType, 1));

        // send listening port, peer will extract ip on its own
        std::string portString = getAcceptorPort();
        char addressSize = static_cast<char>(portString.size());
        boost::asio::write(tmpSocket, boost::asio::buffer(&addressSize, 1));            // send size of port;
        boost::asio::write(tmpSocket, boost::asio::buffer(portString));                 // send port
    } catch (std::exception& e) {
        std::cerr << "Peer::sendRemRequest()" << e.what() << std::endl;
    }
}

void Peer::disconnectFromPeers() {
    for (std::vector<std::string>::iterator it = m_peersList.begin(); it != m_peersList.end(); ++it) {
        std::string currentPeerAddress = P2PNode::addressFromString(*it);
        std::string currentPeerPort = P2PNode::portFromString(*it);

        try {
            tcp::socket tmpSocket(m_ioService);
            tcp::resolver::query query(currentPeerAddress, currentPeerPort);
            tcp::resolver::iterator endpoint_iterator = m_resolver.resolve(query);

            boost::asio::connect(tmpSocket, endpoint_iterator);
            sendRemRequest(tmpSocket);

        } catch (std::exception& e) {
            std::cerr << "Peer::connectToPeers(): " << e.what() << std::endl;
        }
    }
}

void Peer::addShareFile(std::string filepath) {
    // make sure the file's *name* doesn't already exist in shared or available
    std::string filename = boost::filesystem::path(filepath).filename().string();

    for (std::vector<std::string>::const_iterator it = m_sharedFilesList.begin(); it != m_sharedFilesList.end(); ++it) {
        boost::filesystem::path curr(*it);
        std::string currentFilename = curr.filename().string();
        if (currentFilename == filename) throw std::invalid_argument("Peer::addShareFile(): File \"" + filename + "\" is already shared.");
    }

    for (auto const &it : m_availableFilesList) {
        if (it.first == filename) throw std::invalid_argument("Peer::addShareFile(): File \"" + filename + "\" is already available.");
    }

    // send add file requests to clients and conncetion manager
    try {
        tcp::socket tmpSocket(m_ioService);
        tcp::resolver::query query(m_connectionManagerAddress, m_connectionManagerPort);
        tcp::resolver::iterator endpoint_iterator = m_resolver.resolve(query);

        boost::asio::connect(tmpSocket, endpoint_iterator);
        sendAddFileRequest(tmpSocket, filename, getAcceptorPort());

    } catch (std::exception& e) {
        std::cerr << "Peer::connectToPeers(): Unable to connect to connection manager. " << e.what() << std::endl;
        throw e;
    }

    for (std::vector<std::string>::const_iterator it = m_peersList.begin(); it != m_peersList.end(); ++it) {
        std::string currentPeerAddress = P2PNode::addressFromString(*it);
        std::string currentPeerPort = P2PNode::portFromString(*it);

        try {
            tcp::socket tmpSocket(m_ioService);
            tcp::resolver::query query(currentPeerAddress, currentPeerPort);
            tcp::resolver::iterator endpoint_iterator = m_resolver.resolve(query);

            boost::asio::connect(tmpSocket, endpoint_iterator);
            sendAddFileRequest(tmpSocket, filename, getAcceptorPort());

        } catch (std::exception& e) {
            std::cerr << "Peer::connectToPeers(): " << e.what() << std::endl;
        }
    }
    m_sharedFilesList.push_back(filepath);  // add to your list
}

void Peer::sendAddFileRequest(tcp::socket &tmpSocket, std::string filePath, std::string port) {
    try {
        std::string filename = boost::filesystem::path(filePath).filename().string();
        // send message type
        char messageType = static_cast<char>(Peer::ADDFILEREQUEST);
        boost::asio::write(tmpSocket, boost::asio::buffer(&messageType, 1));

        // send corresponding port
        char portSize = static_cast<char>(port.size());
        boost::asio::write(tmpSocket, boost::asio::buffer(&portSize, 1));
        boost::asio::write(tmpSocket, boost::asio::buffer(port));

        // send filename
        if (filename.size() > P2PNode::MAX_STRING_SIZE) throw std::length_error("Filename too large. Sorry m8, totally my fault.");
        char fnSize = static_cast<char>(filename.size());
        boost::asio::write(tmpSocket, boost::asio::buffer(&fnSize, 1));
        boost::asio::write(tmpSocket, boost::asio::buffer(filename));

    } catch (std::exception& e) {
        std::cerr << "Peer::sendAddFileRequest()" << e.what() << std::endl;
    }
}

void Peer::handleAddRequest() {
    std::string addressString = parseAddress();     // parse address:port acceptor from socket
    addPeer(addressString);                         // add address to list
}

const std::vector<std::string>& Peer::getSharedFilesList() const {
    return m_sharedFilesList;
}

void Peer::printSharedFiles() const {
    std::cout << "Shared Files:" << std::endl;
    std::vector<std::string>::const_iterator it;
    for (it = m_sharedFilesList.begin(); it != m_sharedFilesList.end(); ++it) {
        std::cout << *it << "\n";
    }
    std::cout << std::endl;
}

void Peer::setConnectionManagerAddress(std::string s) {
    m_connectionManagerAddress = s;
}

void Peer::setConnectionManagerPort(std::string s) {
    m_connectionManagerPort = s;
}
