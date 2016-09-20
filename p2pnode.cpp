#include "p2pnode.h"
#include <iostream>
#include <algorithm>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <string>
#include <vector>
#include <cassert>

void P2PNode::handleConnection() {
    // wait for a peer to connect
    m_acceptor.accept(m_socket);
    std::cout << "Received connection" << std::endl;

    // determine request type
    boost::array<char, 1> requestType;
    boost::asio::read(m_socket, boost::asio::buffer(requestType), boost::asio::transfer_exactly(1));

    // handle request
    if (requestType[0] == ADDREQUEST) handleAddRequest();
    else if (requestType[0] == REMREQUEST) handleRemRequest();

    // close connection after handling request
    m_socket.close();
}

P2PNode::~P2PNode() {
    closeConnection();
}

void P2PNode::handleAddRequest() {
    std::string addressString = parseAddress();	// parse address from socket
    sendPeersList();                            // send list of peers
    addPeer(addressString);                     // add connecting address to list
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

void P2PNode::handleRemRequest() {
    std::string addressString = parseAddress();
    remPeer(addressString);
}

void P2PNode::sendPeersList() {
    // send num peers
    char numPeers = static_cast<char>(m_peersList.size());
    try {
        boost::asio::write(m_socket, boost::asio::buffer(&numPeers, 1));

        // send peers
        for (std::vector<std::string>::iterator it = m_peersList.begin(); it != m_peersList.end(); ++it) {
            // send size of string
            assert((*it).size() < BYTE_SIZE);
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

void P2PNode::addPeer(std::string peer) {
    if (std::find(m_peersList.begin(), m_peersList.end(), peer) == m_peersList.end()) m_peersList.push_back(peer);
    else std::cerr << peer << " alread exists in list of peers" << std::endl;
}

void P2PNode::remPeer(std::string peer) {
    m_peersList.erase(std::remove(m_peersList.begin(), m_peersList.end(), peer), m_peersList.end());
}

void P2PNode::closeConnection() {
    m_socket.close();
}

const std::vector<std::string>& P2PNode::getPeersList() const {
    return m_peersList;
}

void P2PNode::printPeers() {
    std::cout << "peers>\n";
    for (std::vector<std::string>::iterator it = m_peersList.begin(); it != m_peersList.end(); ++it) std::cout << *it << std::endl;
    std::cout << "<peers" << std::endl;
}
