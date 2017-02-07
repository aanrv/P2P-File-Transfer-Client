#include "peer.h"
#include <boost/array.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem/path.hpp>
#include <fstream>

using boost::asio::ip::tcp;

Peer::~Peer() {
}

void Peer::handleConnection() {
    // wait for a peer to connect
    // use shared_ptr for tcp::socket to keep alive for use with download thread
    boost::shared_ptr<tcp::socket> tmpSocket;
    tmpSocket.reset(new tcp::socket(m_ioService));
    //tcp::socket tmpSocket(m_ioService);
    m_acceptor.accept(*tmpSocket);
    std::cout << "Received connection." << std::endl;

    // determine request type
    char requestType;
    boost::asio::read(*tmpSocket, boost::asio::buffer(&requestType, 1));

    // handle request
    if (requestType == ADDREQUEST) handleAddRequest(*tmpSocket);
    else if (requestType == REMREQUEST) handleRemRequest(*tmpSocket);
    else if (requestType == ADDFILEREQUEST) handleAddFileRequest(*tmpSocket);
    else if (requestType == REMFILEREQUEST) handleRemFileRequest(*tmpSocket);
    else if (requestType == DOWNFILEREQUEST) boost::thread downFileThread(boost::bind(&Peer::handleDownloadFileRequest, this, tmpSocket));
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

void Peer::handleAddRequest(tcp::socket& tmpSocket) {
    std::string addressString = parseAddress(tmpSocket);    // parse address:port acceptor from socket
    addPeer(addressString);                                 // add address to list
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
        char peerCount;
        boost::asio::read(tmpSocket, boost::asio::buffer(&peerCount, 1));

        // read peerCount many addresses
        for (short i = 0; i < static_cast<short>(peerCount); ++i) {
            // read first byte for size of string
            char strSize;
            boost::asio::read(tmpSocket, boost::asio::buffer(&strSize, 1));

            boost::array<char, MAX_STRING_SIZE> buffer;
            boost::asio::read(tmpSocket, boost::asio::buffer(buffer), boost::asio::transfer_exactly(static_cast<size_t>(strSize)));
            std::string readString(buffer.begin(), buffer.begin() + static_cast<short>(strSize));

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
        char fileCount;
        boost::asio::read(tmpSocket, boost::asio::buffer(&fileCount, 1));

        // read peerCount many addresses
        for (short i = 0; i < static_cast<short>(fileCount); ++i) {
            // read first byte for size of file string
            char strSize;
            boost::asio::read(tmpSocket, boost::asio::buffer(&strSize, 1));
            // read file str
            boost::array<char, MAX_STRING_SIZE> buffer;
            boost::asio::read(tmpSocket, boost::asio::buffer(buffer), boost::asio::transfer_exactly(static_cast<size_t>(strSize)));
            std::string readString(buffer.begin(), buffer.begin() + static_cast<short>(strSize));

            // read first byte for size of addr string
            char addrSize;
            boost::asio::read(tmpSocket, boost::asio::buffer(&addrSize, 1));
            // read addr str
            boost::array<char, MAX_STRING_SIZE> addrBuffer;
            boost::asio::read(tmpSocket, boost::asio::buffer(addrBuffer), boost::asio::transfer_exactly(static_cast<size_t>(addrSize)));
            std::string readAddrString(addrBuffer.begin(), addrBuffer.begin() + static_cast<short>(addrSize));

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
        boost::asio::write(tmpSocket, boost::asio::buffer(&addressSize, 1));    // send size of port;
        boost::asio::write(tmpSocket, boost::asio::buffer(portString));         // send port
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

void Peer::addShareFile(const std::string filepath) {
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

    // send add file requests to connection manager
    try {
        tcp::socket tmpSocket(m_ioService);
        tcp::resolver::query query(m_connectionManagerAddress, m_connectionManagerPort);
        tcp::resolver::iterator endpoint_iterator = m_resolver.resolve(query);

        boost::asio::connect(tmpSocket, endpoint_iterator);
        sendAddFileRequest(tmpSocket, filename, getAcceptorPort());
    } catch (std::exception& e) {
        std::cerr << "Peer::addShareFile(): Unable to connect to connection manager. " << e.what() << std::endl;
    }
    // send add file requests to peers
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
            std::cerr << "Peer::addShareFile(): " << e.what() << std::endl;
        }
    }
    m_sharedFilesList.push_back(filepath);  // add to your list
}

void Peer::sendAddFileRequest(tcp::socket &tmpSocket, const std::string filePath, const std::string port) {
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

void Peer::remShareFile(const std::string filepath) {
    // make sure file exists
    std::string filename = boost::filesystem::path(filepath).filename().string();
    bool found = false;
    for (std::vector<std::string>::const_iterator it = m_sharedFilesList.begin(); it != m_sharedFilesList.end() && !found; ++it) {
        boost::filesystem::path curr(*it);
        std::string currentFilename = curr.filename().string();
        if (currentFilename == filename) found = true;
    }
    if (!found) throw std::invalid_argument("Peer::remShareFile(): File \"" + filename + "\" doesn't exist.");

    // send add file requests to connection manager
    try {
        tcp::socket tmpSocket(m_ioService);
        tcp::resolver::query query(m_connectionManagerAddress, m_connectionManagerPort);
        tcp::resolver::iterator endpoint_iterator = m_resolver.resolve(query);

        boost::asio::connect(tmpSocket, endpoint_iterator);
        sendRemFileRequest(tmpSocket, filename, getAcceptorPort());
    } catch (std::exception& e) {
        std::cerr << "Peer::remShareFile(): Unable to connect to connection manager. " << e.what() << std::endl;
    }
    // send rem file requests to peers
    for (std::vector<std::string>::const_iterator it = m_peersList.begin(); it != m_peersList.end(); ++it) {
        std::string currentPeerAddress = P2PNode::addressFromString(*it);
        std::string currentPeerPort = P2PNode::portFromString(*it);

        try {
            tcp::socket tmpSocket(m_ioService);
            tcp::resolver::query query(currentPeerAddress, currentPeerPort);
            tcp::resolver::iterator endpoint_iterator = m_resolver.resolve(query);

            boost::asio::connect(tmpSocket, endpoint_iterator);
            sendRemFileRequest(tmpSocket, filename, getAcceptorPort());

        } catch (std::exception& e) {
            std::cerr << "Peer::remShareFile(): " << e.what() << std::endl;
        }
    }
    m_sharedFilesList.erase(std::remove(m_sharedFilesList.begin(), m_sharedFilesList.end(), filepath), m_sharedFilesList.end());
}

void Peer::sendRemFileRequest(tcp::socket &tmpSocket, const std::string filepath, const std::string port) {
    try {
        std::string filename = boost::filesystem::path(filepath).filename().string();
        // send message type
        char messageType = static_cast<char>(Peer::REMFILEREQUEST);
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
        std::cerr << "Peer::sendRemFileRequest()" << e.what() << std::endl;
    }
}

void Peer::handleDownloadFileRequest(boost::shared_ptr<tcp::socket> tmpSocketPtr) {
    // read port
    const std::string addressString = parseAddress(*tmpSocketPtr);

    // read filename size
    char filenameStrSize;
    boost::asio::read(*tmpSocketPtr, boost::asio::buffer(&filenameStrSize, 1));
    // read filename
    boost::array<char, MAX_STRING_SIZE> filename;
    boost::asio::read(*tmpSocketPtr, boost::asio::buffer(filename), boost::asio::transfer_exactly(static_cast<size_t>(filenameStrSize)));
    std::string filenameString(filename.begin(), filename.begin() + static_cast<size_t>(filenameStrSize));

    sendFile(*tmpSocketPtr, filenameString);
}

void Peer::sendFile(tcp::socket& tmpSocket, const std::string filename) {
    std::string fullFilePath = pathFromFilename(filename);

    // open file for reading
    std::ifstream readFile;
    readFile.open(fullFilePath, std::ios::in | std::ios::binary);
    if (!readFile.is_open()) {
        std::cerr << "Unable to open file: " << fullFilePath << std::endl;
        throw;
    }

    try {
        char readBuffer[FILE_PACKET_SIZE];
        while (readFile) {
            readFile.read(readBuffer, FILE_PACKET_SIZE);                                         // read into buffer
            boost::asio::write(tmpSocket, boost::asio::buffer(readBuffer, readFile.gcount()));   // send over socket
        }
    } catch (std::exception& e) {
        std::cerr << "Peer::sendFile(): " << e.what() << std::endl;
    }
    readFile.close();
}

std::string Peer::pathFromFilename(const std::string filename) {
    std::vector<std::string>::iterator it;
    for (it = m_sharedFilesList.begin(); it != m_sharedFilesList.end(); ++it) {
        boost::filesystem::path fullpath(*it);
        if (fullpath.filename().string() == filename) return *it;
    }
    return "";
}

void Peer::downloadAvailableFile(const std::string filename) {
    const std::string hostAddr = P2PNode::addressFromString(m_availableFilesList[filename]);
    const std::string hostPort = P2PNode::portFromString(m_availableFilesList[filename]);

    try {
        tcp::socket tmpSocket(m_ioService);
        tcp::resolver::query query(hostAddr, hostPort);
        tcp::resolver::iterator endpoint_iterator = m_resolver.resolve(query);

        boost::asio::connect(tmpSocket, endpoint_iterator);
        std::cout << "Sending download request for " << filename << " to " << hostAddr << ":" << hostPort << std::endl;
        sendDownloadFileRequest(tmpSocket, filename);
    } catch (std::exception& e) {
        std::cerr << "Peer::downloadAvailableFile(): " << e.what() << std::endl;
        throw e;
    }
}

void Peer::sendDownloadFileRequest(tcp::socket &tmpSocket, const std::string filename) {
    // send request type
    const char messageType = static_cast<char>(Peer::DOWNFILEREQUEST);
    boost::asio::write(tmpSocket, boost::asio::buffer(&messageType, 1));
    std::cout << "Send request type " << static_cast<int>(messageType) << std::endl;

    // send port
    const char portStrSize = static_cast<char>(getAcceptorPort().size());
    boost::asio::write(tmpSocket, boost::asio::buffer(&portStrSize, 1));
    boost::asio::write(tmpSocket, boost::asio::buffer(getAcceptorPort()));
    std::cout << "Sent port " << getAcceptorPort() << " (size " << static_cast<size_t>(portStrSize) << ")" << std::endl;

    // send file's name
    const char filenameStrSize = static_cast<char>(filename.size());
    boost::asio::write(tmpSocket, boost::asio::buffer(&filenameStrSize, 1));
    boost::asio::write(tmpSocket, boost::asio::buffer(filename));
    std::cout << "Request for " << filename << " (size " << static_cast<size_t>(filenameStrSize) << ") has been sent." << std::endl;

    recvFile(tmpSocket, filename);
    std::cout << "Finished recieving file " << filename << std::endl;
}

void Peer::recvFile(tcp::socket &tmpSocket, const std::string filename) {
    try {
        // open file for writing
        std::ofstream outFile;
        outFile.open(filename, std::ios::out | std::ios::binary);

        char readBuffer[FILE_PACKET_SIZE];
        bool reachedEOF = false;

        while (!reachedEOF) {
            boost::system::error_code error_code;
            size_t bytesRead = tmpSocket.read_some(boost::asio::buffer(readBuffer, FILE_PACKET_SIZE), error_code);

            outFile.write(readBuffer, bytesRead);
            if (error_code == boost::asio::error::eof) reachedEOF = true;
            else if (error_code) throw boost::system::system_error(error_code);
        }

        outFile.close();
    } catch (std::exception& e) {
        std::cerr << "Peer::recvFile(): " << e.what() << std::endl;
        throw e;
    }
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

void Peer::setConnectionManagerAddress(const std::string s) {
    m_connectionManagerAddress = s;
}

void Peer::setConnectionManagerPort(const std::string s) {
    m_connectionManagerPort = s;
}
