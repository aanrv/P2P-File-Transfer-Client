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
    Peer(unsigned short port) : P2PNode(port), m_resolver(m_ioService) {}
    virtual ~Peer();

    void joinNetwork(std::string server, std::string service);              // retreives peers from connection manager and notifies them

    void sendAddRequest(tcp::socket &tmpSocket);                            // send a peer a request to add self to peer list
    void handleAddRequest();                                                // handle an add request sent by another peer
    void retreivePeersList(tcp::socket& tmpSocket);                         // retrieves list of peers from connection manager
    void connectToPeers();                                                  // connects to peers on peers list

private:
    tcp::resolver m_resolver;
};

#endif // PEER_H
