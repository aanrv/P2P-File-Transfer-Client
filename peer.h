#ifndef PEER_H
#define PEER_H

#include "p2pnode.h"
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/tuple/tuple.hpp>
#include <string>
#include <map>

class Peer : public P2PNode
{
public:
    Peer() : P2PNode(0), m_resolver(m_ioService) {}
    virtual ~Peer();

    void joinNetwork();                                                     // retrieves peers from connection manager and notifies them
    void leaveNetwork();                                                    // leaves network, connection manager told to remove from list
    void addShareFile(std::string filepath);                                // Add a path that is available to share

    void printSharedFiles() const;
    const std::vector<std::string>& getSharedFilesList() const;

    void setConnectionManagerAddress(std::string s);
    void setConnectionManagerPort(std::string s);

private:
    void retrievePeersList(tcp::socket& tmpSocket);                         // retrieves list of peers from connection manager
    void retrieveFilesList(tcp::socket& tmpSocket);                         // retrieves list of files from connection manager

    void connectToPeers();                                                  // connects to peers on peers list
    void sendAddRequest(tcp::socket &tmpSocket);                            // send a peer a request to add self to peer list

    void disconnectFromPeers();                                             // leaves network and disconnects
    void sendRemRequest(tcp::socket &tmpSocket);                            // send a peer a request to remove self from peer list

    void sendAddFileRequest(tcp::socket &tmpSocket, std::string filePath, std::string port);    // requests peer to add file to available list

    void handleAddRequest();                                                // handle an add request sent by another peer

    std::string m_connectionManagerAddress;
    std::string m_connectionManagerPort;

    tcp::resolver m_resolver;
    std::vector<std::string> m_sharedFilesList;                             // list of files you are sharing
};

#endif // PEER_H
