#include "mainwindow.h"
#include "peer.h"
#include <QWidget>
#include <QApplication>
#include <QSplitter>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QMessageBox>
#include <QSpinBox>
#include <vector>
#include <boost/asio.hpp>
#include <boost/array.hpp>

using boost::asio::ip::tcp;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    createWidgets();
    createLayouts();

    // this will be decided when user decides to connect
    m_peer = NULL;
}

MainWindow::~MainWindow() {
    delete m_peer;
}

void MainWindow::createWidgets() {
    m_peersList = new QListWidget();
    m_filesList = new QListWidget();
    m_localPortBar = new QLineEdit();
    m_localPortBar->setPlaceholderText(tr("Port number to assign client"));
    m_connectButton = new QPushButton(tr("Connect To Network"));

    m_searchBar = new QLineEdit();
    m_searchBar->setPlaceholderText(tr("Search"));
    m_searchButton = new QToolButton();
    m_searchButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogContentsView));

    connect(m_connectButton, SIGNAL(clicked()), this, SLOT(s_connect()));
    connect(this, SIGNAL(hasConnected()), this, SLOT(s_waitForPeers()));
}

void MainWindow::createLayouts() {
    QWidget* centralWidget = new QWidget();
    QVBoxLayout* centralLayout = new QVBoxLayout(centralWidget);

    QHBoxLayout* topBox = new QHBoxLayout();
    QSplitter* topSplit = new QSplitter(Qt::Orientation::Horizontal);

    QHBoxLayout* botBox = new QHBoxLayout();
    QVBoxLayout* lBox = new QVBoxLayout();
    QVBoxLayout* rBox = new QVBoxLayout();

    // Top-left, lists hosts
    lBox->addWidget(new QLabel("Peers"));
    lBox->addWidget(m_peersList);

    // Top-right, lists files available for download
    rBox->addWidget(new QLabel("Files"));
    QHBoxLayout* searchLayout = new QHBoxLayout();
    searchLayout->addWidget(m_searchBar);
    searchLayout->addWidget(m_searchButton);
    rBox->addLayout(searchLayout);
    rBox->addWidget(m_filesList);

    // Splitter for hosts and files lists
    QWidget* lWidget = new QWidget();
    QWidget* rWidget = new QWidget();
    lWidget->setLayout(lBox);
    rWidget->setLayout(rBox);
    topSplit->addWidget(lWidget);
    topSplit->addWidget(rWidget);

    topBox->addWidget(topSplit);
    botBox->addWidget(m_localPortBar);
    botBox->addWidget(m_connectButton);

    centralLayout->addLayout(topBox);
    centralLayout->addLayout(botBox);

    this->setCentralWidget(centralWidget);
    this->setWindowTitle("P2P Fileshare Interface");
}

void MainWindow::refreshPeerList() {
    std::cout << "Refreshing list of peers" << std::endl;

    if (m_peer != NULL) {
        const std::vector<std::string> peers = m_peer->getPeersList();

        m_peersList->clear();
        for (std::vector<std::string>::const_iterator it = peers.begin(); it != peers.end(); ++it) {
            m_peersList->addItem(QString::fromStdString(*it));
        }
    } else {
        std::cerr << "Cannot refresh peers list. m_peer == NULL" << std::endl;
    }
    qApp->processEvents();
}

void MainWindow::s_connect() {
    // determine user's provided port
    QByteArray tmpPortBuffer = m_localPortBar->text().toLocal8Bit();
    unsigned short port = P2PNode::strToPort(tmpPortBuffer.data());

    // check port validity
    if (port == 0) {
        QMessageBox::information(this, "P2P-Filesharing-Client",
                                 QString("Port number must be between %1 and %2.").arg(
                                     QString::number(P2PNode::PORTMIN),
                                     QString::number(P2PNode::PORTMAX)));
        m_localPortBar->clear();
        return;
    } else {
        // create peer and assign port
        try {
            m_peer = new Peer(port);
        } catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
            QMessageBox::information(this, "P2P-Filesharing-Client", QString("Unable to assign address.\nMake sure port is not already in use."));
            m_localPortBar->clear();
            // peer was unable to connect, revert to original state (deleted and pointer set NULL)
            delete m_peer;
            m_peer = NULL;
            return;
        }
    }

    try {
        // port valid, join network
        m_peer->joinNetwork("localhost", std::to_string(P2PNode::DEFPORT));

        // disable ability to reconnect
        m_connectButton->setEnabled(false);
        m_localPortBar->setEnabled(false);
        refreshPeerList();
        qApp->processEvents();

        emit hasConnected();
    } catch(std::exception& e) {
        QMessageBox::information(
            this,
            "P2P-Filesharing-Client",
            QString("Unable to connect to Connection Manager.\nPlease make sure it is running."));
        m_localPortBar->clear();
        // peer was unable to connect, revert to original state (deleted and pointer set NULL)
        delete m_peer;
        m_peer = NULL;
    }
}

void MainWindow::s_waitForPeers() {
    for (;;) {
        m_peer->handleConnection();
        refreshPeerList();
        qApp->processEvents();
    }
}
