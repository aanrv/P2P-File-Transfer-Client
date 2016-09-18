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

    void joinNetwork(std::string server, std::string service);
    void sendAddRequest(tcp::socket &tmpSocket);
    void handleAddRequest();
    void retreivePeersList(tcp::socket& tmpSocket);
    void connectToPeers();

private:
    tcp::resolver m_resolver;
};

#endif // PEER_H
