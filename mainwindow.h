#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "peer.h"

class QListWidget;
class QLineEdit;
class QPushButton;
class QToolButton;
class QListWidgetItem;
class QMessageBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();   

private:
    enum DownloadStatus {
        DOWNLOAD_START,
        DOWNLOAD_SUCCESS,
        DOWNLOAD_FAILURE
    };

    Peer m_peer;

    QListWidget*    m_peersList;                // Displays list of peers on network
    QListWidget*    m_sharedFilesList;          // Displays list of files being shared by user
    QListWidget*    m_availableFilesList;       // Displays list of files available for download from other peers
    
    QPushButton*    m_addFileButton;            // s_addShareFile()
    QPushButton*    m_remFileButton;            // s_remShareFile()
    QPushButton*    m_downloadFileButton;       // s_downloadAvailableFile();
    QLineEdit*      m_searchBar;                // Used to search for filenames in peers list
    QToolButton*    m_searchButton;

    QLineEdit*      m_addrBar;                  // Address of connection manager host
    QLineEdit*      m_portBar;                  // Connection manager port number

    QPushButton*    m_connectButton;            // s_connect()

    QMessageBox*    m_downloadResultMessage;    // message box indicating whether download succeeded or failed

    void createWidgets();
    void createLayouts();
    void startAcceptorThread();
    void waitForPeers();
    void downloadFile();

    void refreshPeerList();
    void refreshShareList();
    void refreshAvailableList();

private slots:
    void s_connect();                                                           // Connects client to network and displays peers and files available for download
    void s_addShareFile();                                                      // Browse for a file to allow sharing it
    void s_remShareFile();                                                      // Remove file that is currently being shared
    void s_downloadAvailableFile();                                             // Download selected file
    void s_downloadStatus(const QString filename, const int downloadStatus);    // Displays message box informing download success/failure
    void s_updateLists();

signals:
    void downloadStatus(const QString filename, const int downloadStatus);      // Indicates which message box to be displayed during file download process
    void updateLists();                                                         // Emitted after handling connection, notifying main thread to update GUI
};

#endif // MAINWINDOW_H
