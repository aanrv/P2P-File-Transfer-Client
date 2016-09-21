#ifndef PEER_H
#define PEER_H

#include "p2pnode.h"
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/tuple/tuple.hpp>
#include <string>

class Peer : public P2PNode
{
public:
    Peer() : P2PNode(0), m_resolver(m_ioService) {}
    virtual ~Peer();

    void joinNetwork(std::string server, std::string service);              // retreives peers from connection manager and notifies them
    void leaveNetwork(std::string server, std::string service);             // leaves network, connection manager told to remove from list

    void connectToPeers();                                                  // connects to peers on peers list
    void disconnectFromPeers();                                             // leaves network and disconnects

    void sendAddRequest(tcp::socket &tmpSocket);                            // send a peer a request to add self to peer list
    void sendRemRequest(tcp::socket &tmpSocket);                            // send a peer a request to remove self from peer list

    void retreivePeersList(tcp::socket& tmpSocket);                         // retrieves list of peers from connection manager
    void handleAddRequest();                                                // handle an add request sent by another peer

private:
    tcp::resolver m_resolver;
};

#endif // PEER_H
