// 1. + - buttons for sharing files.
// 2. Change single size buffers to chars
// 3. Use more mutex.
// 4. handleConnection should return connection type to avoid having to refresh everything.
// 5. Send add/rem req functions can be shortened.

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
class QListWidgetItem;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();   

private:
    Peer m_peer;

    QListWidget*    m_peersList;            // Displays list of peers on network
    QListWidget*    m_sharedFilesList;      // Displays list of files being shared by user
    QListWidget*    m_availableFilesList;   // Displays list of files available for download from other peers
    
    QPushButton*    m_addFileButton;        // s_addShareFile()
    QPushButton*    m_remFileButton;        // s_remShareFile()
    QLineEdit*      m_searchBar;            // Used to search for filenames in peers list
    QToolButton*    m_searchButton;

    QLineEdit*      m_addrBar;              // Address of connection manager host
    QLineEdit*      m_portBar;              // Connection manager port number

    QPushButton*    m_connectButton;        // s_connect()

    boost::thread   m_acceptorThread;

    void createWidgets();
    void createLayouts();
    void startAcceptorThread();
    void waitForPeers();

    void refreshPeerList();
    void refreshShareList();
    void refreshAvailableList();

private slots:
    void s_connect();                       // Connects client to network and displays peers and files available for download
    void s_addShareFile();                  // Browse for a file to allow sharing it
    void s_remShareFile();                  // Remove file that is currently being shared
};

#endif // MAINWINDOW_H
