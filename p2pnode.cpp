#include "p2pnode.h"
#include <iostream>
#include <algorithm>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <string>
#include <vector>
#include <cassert>

void P2PNode::handleConnection() {
    std::cout << "---------->" << std::endl;
    // wait for a peer to connect
    m_acceptor.accept(m_socket);

    // determine request type
    boost::array<char, 1> requestType;
    boost::asio::read(m_socket, boost::asio::buffer(requestType), boost::asio::transfer_exactly(1));

    // handle request
    if (requestType[0] == ADDREQUEST) handleAddRequest();
    else if (requestType[0] == REMREQUEST) handleRemRequest();
    else if (requestType[0] == ADDFILEREQUEST) handleAddFileRequest();
    // else if (requestType[0] == REMFILEREQUEST) handleRemFileRequest();

    // close connection after handling request
    m_socket.close();

    printPeers();
    printAvailableFiles();
    std::cout << "<----------" << std::endl;
}

std::string P2PNode::parseAddress() {
    // read sender's port string size
    boost::array<char, 1> addressSize;
    boost::asio::read(m_socket, boost::asio::buffer(addressSize), boost::asio::transfer_exactly(1));

    // read port
    boost::array<char, MAX_STRING_SIZE> portBuffer;
    boost::asio::read(m_socket, boost::asio::buffer(portBuffer), boost::asio::transfer_exactly(static_cast<short>(addressSize[0])));
    std::string portString(portBuffer.begin(), portBuffer.begin() + static_cast<size_t>(addressSize[0]));

    // get sender's address
    std::string addressString(m_socket.remote_endpoint().address().to_string());

    return  addressString + ":" + portString;
}

void P2PNode::handleAddRequest() {
    std::string addressString = parseAddress();	// parse address from socket
    sendPeersList();                            // send list of peers
    sendFilesList();                            // send list of files
    addPeer(addressString);                     // add connecting address to list
}

void P2PNode::addPeer(std::string peer) {
    if (std::find(m_peersList.begin(), m_peersList.end(), peer) == m_peersList.end()) m_peersList.push_back(peer);
    else std::cerr << peer << " alread exists in list of peers" << std::endl;
}

void P2PNode::sendPeersList() {
    // send num peers
    char numPeers = static_cast<char>(m_peersList.size());
    try {
        boost::asio::write(m_socket, boost::asio::buffer(&numPeers, 1));

        // send peers
        for (std::vector<std::string>::iterator it = m_peersList.begin(); it != m_peersList.end(); ++it) {
            // send size of string
            assert(it->size() < BYTE_SIZE);
            char stringSize = static_cast<char>((*it).size());
            boost::asio::write(m_socket, boost::asio::buffer(&stringSize, 1));

            // send string
            boost::asio::write(m_socket, boost::asio::buffer(*it));
        }
    } catch (std::exception& e) {
        std::cerr << "P2PNode::sendPeersList(): " << e.what() << std::endl;
        throw e;
    }
}

void P2PNode::sendFilesList() {
    // send num files
    char numFiles = static_cast<char>(m_availableFilesList.size());
    try {
        boost::asio::write(m_socket, boost::asio::buffer(&numFiles, 1));

        // send files and address
        for (auto it = m_availableFilesList.begin(); it != m_availableFilesList.end(); ++it) {
            // send size of filename string
            assert(it->first.size() < BYTE_SIZE);
            char stringSize = static_cast<char>(it->first.size());
            boost::asio::write(m_socket, boost::asio::buffer(&stringSize, 1));
            // send filename string
            boost::asio::write(m_socket, boost::asio::buffer(it->first));

            // send size of address string
            assert(it->second.size() < BYTE_SIZE);
            char addrStringSize = static_cast<char>(it->second.size());
            boost::asio::write(m_socket, boost::asio::buffer(&addrStringSize, 1));
            // send address string
            boost::asio::write(m_socket, boost::asio::buffer(it->second));
        }
    } catch (std::exception& e) {
        std::cerr << "P2PNode::sendFilesList(): " << e.what() << std::endl;
    }
}

void P2PNode::handleRemRequest() {
    std::string addressString = parseAddress();
    remPeerFiles(addressString);
    remPeer(addressString);
}

void P2PNode::remPeer(std::string peer) {
    m_peersList.erase(std::remove(m_peersList.begin(), m_peersList.end(), peer), m_peersList.end());
}

const std::vector<std::string>& P2PNode::getPeersList() const {
    return m_peersList;
}

std::string P2PNode::getAcceptorPort() const {
    return std::to_string(m_acceptor.local_endpoint().port());
}

void P2PNode::printPeers() const {
    std::cout << "Peers List:\n";
    for (std::vector<std::string>::const_iterator it = m_peersList.begin(); it != m_peersList.end(); ++it) std::cout << *it << std::endl;
    std::cout << std::endl;
}

void P2PNode::printAvailableFiles() const {
    std::cout << "Available Files:\n";
    for (auto const &it : m_availableFilesList) {
        std::cout << it.first << " (" << it.second << ")" << std::endl;
    }
    std::cout << std::endl;
}

const std::map<std::string, std::string>& P2PNode::getAvailableList() const {
    return m_availableFilesList;
}

void P2PNode::handleAddFileRequest() {
    // parse address:port from acceptor socket
    std::string addressString = parseAddress();

    // read filename size
    boost::array<char, 1> fnSize;
    boost::asio::read(m_socket, boost::asio::buffer(fnSize), boost::asio::transfer_exactly(1));

    // read filename
    boost::array<char, MAX_STRING_SIZE> filenameBuffer;
    boost::asio::read(m_socket, boost::asio::buffer(filenameBuffer), boost::asio::transfer_exactly(static_cast<size_t>(fnSize[0])));

    std::string filenameString(filenameBuffer.begin(), filenameBuffer.begin() + static_cast<size_t>(fnSize[0]));

    addAvailableFile(filenameString, addressString);
}

void P2PNode::addAvailableFile(std::string filename, std::string address) {
    m_availableFilesList[filename] = address;
    printAvailableFiles();
}

void P2PNode::remPeerFiles(std::string peer) {
    for (auto it = m_availableFilesList.begin(); it != m_availableFilesList.end(); ) {
        if (it->second == peer) m_availableFilesList.erase(it);
        else ++it;
    }
}
