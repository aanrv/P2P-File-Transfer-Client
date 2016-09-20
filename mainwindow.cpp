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
#include <vector>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

using boost::asio::ip::tcp;
boost::mutex refreshMutex;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    createWidgets();
    createLayouts();
}

MainWindow::~MainWindow() {
    m_peer->leaveNetwork("localhost", std::to_string(P2PNode::DEFPORT));
}

void MainWindow::createWidgets() {
    m_peersList = new QListWidget();
    m_filesList = new QListWidget();
    m_localPortBar = new QLineEdit();
    m_localPortBar->setPlaceholderText(tr("Enter the port number you wish to run on"));
    m_connectButton = new QPushButton(tr("Connect To Network"));

    m_searchBar = new QLineEdit();
    m_searchBar->setPlaceholderText(tr("Search"));
    m_searchButton = new QToolButton();
    m_searchButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogContentsView));

    connect(m_connectButton, SIGNAL(clicked()), this, SLOT(s_connect()));
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
    boost::mutex::scoped_lock lock(refreshMutex);
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
        QMessageBox::information(
                    this,
                    "P2P-Filesharing-Client",
                    QString("Port number must be between %1 and %2.").arg(
                        QString::number(P2PNode::PORTMIN),
                        QString::number(P2PNode::PORTMAX)));
        m_localPortBar->clear();
        return;
    } else {
        // valid port, create peer
        try {
            m_peer.reset(new Peer(port));
        } catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
            QMessageBox::information(
                        this,
                        "P2P-Filesharing-Client",
                        QString("Unable to assign address.\n" \
                                "Please make sure port is not already in use."));
            m_localPortBar->clear();
            m_peer.reset();             // peer was unable to have port assigned, delete
            return;
        }
    }

    try {
        m_peer->joinNetwork("localhost", std::to_string(P2PNode::DEFPORT));

        m_connectButton->setEnabled(false);     // disable ablity to reconnect
        m_localPortBar->setEnabled(false);
        refreshPeerList();
        qApp->processEvents();

        startAcceptorThread();                  // start seperate thread for recieving connections

    } catch(std::exception& e) {
        QMessageBox::information(
            this,
            "P2P-Filesharing-Client",
            QString("Unable to connect to Connection Manager.\n" \
                    "Please make sure it is running."));
        m_localPortBar->clear();
        m_peer.reset();
    }
}

void MainWindow::startAcceptorThread() {
    m_acceptorThread = boost::thread(boost::bind(&MainWindow::waitForPeers, this));
}

void MainWindow::waitForPeers() {
    for (;;) {
        m_peer->handleConnection();
        refreshPeerList();
        qApp->processEvents();
    }
}
