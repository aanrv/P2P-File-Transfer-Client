// TO DO
// 1. Make port spinbox
// 2. Add connection manager port
// 4. Other general error handling
// 6. Understand why child destructor virtual

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <boost/asio.hpp>

class QListWidget;
class QLineEdit;
class QPushButton;
class QToolButton;
class Peer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Peer* m_peer;

    QListWidget*    m_peersList;        /* Displays list of peers on network. */
    QListWidget*    m_filesList;        /* Displays list of files available for download from other peers. */
    
    QLineEdit*      m_searchBar;        /* Used to search for filenames in peers list. */
    QToolButton*    m_searchButton;

    QLineEdit*      m_localPortBar;     /* Port number to assign client. */
    QPushButton*    m_connectButton;    /* s_connect() */

    void createWidgets();
    void createLayouts();
    void refreshPeerList();

private slots:
    void s_connect();                   /* Connects client to network and displays peers and files available for download. */
    void s_waitForPeers();

signals:
    void hasConnected();
};

#endif // MAINWINDOW_H
