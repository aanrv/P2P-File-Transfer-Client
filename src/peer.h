#ifndef PEER_H
#define PEER_H

#include "p2pnode.h"
#include <string>

class Peer : public P2PNode
{
public:

    static const size_t FILE_PACKET_SIZE = 1024;

    typedef enum _MessageType {
        DOWNFILEREQUEST = REMFILEREQUEST + 1                            // request to recieve a file for download
    } MessageType;

    Peer() : P2PNode(0), m_resolver(m_ioService) {}
    virtual ~Peer();

    void handleConnection();

    void joinNetwork();                                                 // retrieves peers from connection manager and notifies them
    void leaveNetwork();                                                // leaves network, connection manager told to remove from list
    void addShareFile(const std::string filepath);                      // Add a path that is available to share
    void remShareFile(const std::string filepath);                      // Remove a file from being shared
    void downloadAvailableFile(const std::string filename);             // Download a file made available by a peer

    const std::vector<std::string>& getSharedFilesList() const;
    void printSharedFiles() const;

    void setConnectionManagerAddress(const std::string s);
    void setConnectionManagerPort(const std::string s);

private:
    void retrievePeersList(tcp::socket& tmpSocket);                     // retrieves list of peers from connection manager
    void retrieveFilesList(tcp::socket& tmpSocket);                     // retrieves list of files from connection manager

    void connectToPeers();                                              // connects to peers on peers list
    void sendAddRequest(tcp::socket &tmpSocket);                        // send a peer a request to add self to peer list

    void disconnectFromPeers();                                         // leaves network and disconnects
    void sendRemRequest(tcp::socket &tmpSocket);                        // send a peer a request to remove self from peer list

    void sendAddFileRequest(tcp::socket &tmpSocket, const std::string filePath, const std::string port);    // requests peer to add file to available list
    void sendRemFileRequest(tcp::socket &tmpSocket, const std::string filepath, const std::string port);    // requests peer to remove file from their available list
    void sendDownloadFileRequest(tcp::socket &tmpSocket, const std::string filename);                       // requests peer to provide file 'filename'

    void handleAddRequest(tcp::socket &tmpSocket);                                          // handle an add request sent by another peer
    void handleDownloadFileRequest(boost::shared_ptr<tcp::socket> tmpSocketPtr);            // handle a request from peer to provide file

    void sendFile(tcp::socket &tmpSocket, const std::string filename);                      // send file filename over connected socket
    void recvFile(tcp::socket &tmpSocket, const std::string filename);                      // recieve a file over tmpSocket

    std::string pathFromFilename(const std::string filename);                               // returns the complete path from a filename in share list

    std::string m_connectionManagerAddress;
    std::string m_connectionManagerPort;

    tcp::resolver m_resolver;
    std::vector<std::string> m_sharedFilesList;                                             // list of files you are sharing
};

#endif // PEER_H
