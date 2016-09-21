#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/scoped_ptr.hpp>
#include "peer.h"

class QListWidget;
class QLineEdit;
class QPushButton;
class QToolButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void refreshPeerList();

private:
    Peer m_peer;

    QListWidget*    m_peersList;        // Displays list of peers on network
    QListWidget*    m_filesList;        // Displays list of files available for download from other peers
    
    QLineEdit*      m_searchBar;        // Used to search for filenames in peers list
    QToolButton*    m_searchButton;

    QLineEdit*      m_addrBar;          // Address of connection manager host
    QLineEdit*      m_portBar;          // Connection manager port number
    QPushButton*    m_connectButton;    // s_connect()

    boost::thread   m_acceptorThread;

    void createWidgets();
    void createLayouts();
    void startAcceptorThread();
    void waitForPeers();

private slots:
    void s_connect();                   // Connects client to network and displays peers and files available for download
};

#endif // MAINWINDOW_H
