#include "mainwindow.h"
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

const QString applicationName("P2P Filesharing Client");

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    createWidgets();
    createLayouts();
}

MainWindow::~MainWindow() {
    m_peer.leaveNetwork(m_addrBar->text().toUtf8().constData(), m_portBar->text().toUtf8().constData());
}

void MainWindow::createWidgets() {
    m_peersList = new QListWidget();
    m_filesList = new QListWidget();
    m_addrBar = new QLineEdit();
    m_addrBar->setPlaceholderText(tr("Server Address"));
    m_portBar = new QLineEdit();
    m_portBar->setPlaceholderText(tr("Connection Manager Port"));
    m_connectButton = new QPushButton(tr("Connect"));

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
    botBox->addWidget(m_addrBar);
    botBox->addWidget(m_portBar);
    botBox->addWidget(m_connectButton);

    centralLayout->addLayout(topBox);
    centralLayout->addLayout(botBox);

    this->setCentralWidget(centralWidget);
    this->setWindowTitle(applicationName);
}

void MainWindow::refreshPeerList() {
    boost::mutex::scoped_lock lock(refreshMutex);
    std::cout << "Refreshing list of peers" << std::endl;

    const std::vector<std::string> peers = m_peer.getPeersList();

    m_peersList->clear();
    for (std::vector<std::string>::const_iterator it = peers.begin(); it != peers.end(); ++it) {
        m_peersList->addItem(QString::fromStdString(*it));
    }

    qApp->processEvents();
}

void MainWindow::s_connect() {
    try {
        m_peer.joinNetwork(m_addrBar->text().toUtf8().constData(), m_portBar->text().toUtf8().constData());

        m_connectButton->setEnabled(false);     // disable ablity to reconnect
        m_addrBar->setEnabled(false);
        m_portBar->setEnabled(false);
        refreshPeerList();
        setWindowTitle(windowTitle() + QString(" [Connected]"));
        qApp->processEvents();

        startAcceptorThread();                  // start seperate thread for recieving connections

    } catch(std::exception& e) {
        QMessageBox::information(
            this,
            applicationName,
            QString("Unable to connect to Connection Manager.\n" \
                    "Please make sure it is running."));
        m_portBar->clear();
        m_addrBar->clear();
    }
}

void MainWindow::startAcceptorThread() {
    m_acceptorThread = boost::thread(boost::bind(&MainWindow::waitForPeers, this));
}

void MainWindow::waitForPeers() {
    for (;;) {
        m_peer.handleConnection();
        refreshPeerList();
        qApp->processEvents();
    }
}
