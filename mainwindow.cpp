#include "mainwindow.h"
#include <QWidget>
#include <QSplitter>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QMessageBox>
#include <QFileDialog>
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
    m_peer.leaveNetwork();
}

void MainWindow::createWidgets() {
    m_peersList = new QListWidget();
    m_availableFilesList = new QListWidget();
    m_sharedFilesList = new QListWidget();
    m_addrBar = new QLineEdit();
    m_addrBar->setPlaceholderText(tr("Server Address"));
    m_portBar = new QLineEdit();
    m_portBar->setPlaceholderText(tr("Connection Manager Port"));
    m_connectButton = new QPushButton(tr("Connect"));

    m_searchBar = new QLineEdit();
    m_searchBar->setPlaceholderText(tr("Search"));
    m_searchButton = new QToolButton();
    m_searchButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogContentsView));
    m_addFileButton = new QPushButton("Add a File");

    connect(m_connectButton, SIGNAL(clicked()), this, SLOT(s_connect()));
    connect(m_addFileButton, SIGNAL(clicked()), this, SLOT(s_addShareFile()));
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
    QHBoxLayout* addFileLayout = new QHBoxLayout();
    addFileLayout->addWidget(new QLabel("Shared Files"));
    addFileLayout->addWidget(m_addFileButton);
    rBox->addLayout(addFileLayout);
    rBox->addWidget(m_sharedFilesList);
    rBox->addWidget(new QLabel("Available Files"));
    rBox->addWidget(m_availableFilesList);
    QHBoxLayout* searchLayout = new QHBoxLayout();
    searchLayout->addWidget(m_searchBar);
    searchLayout->addWidget(m_searchButton);
    rBox->addLayout(searchLayout);

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
}

void MainWindow::refreshShareList() {
    std::cout << "Refreshing list of shared files." << std::endl;
    m_sharedFilesList->clear();
    for (std::vector<std::string>::const_iterator it = m_peer.getSharedFilesList().begin(); it != m_peer.getSharedFilesList().end(); ++it) {
        m_sharedFilesList->addItem(QString::fromStdString(*it));
    }
    m_peer.printSharedFiles();
}

void MainWindow::refreshAvailableList() {
    m_availableFilesList->clear();
    std::map<std::string, std::string> availMap = m_peer.getAvailableList();
    for(auto const &it : availMap) {
        m_availableFilesList->addItem(QString("%1 (%2)").arg(QString::fromStdString(it.first), QString::fromStdString((it.second))));
    }
}

void MainWindow::s_connect() {
    try {
        m_peer.setConnectionManagerAddress(m_addrBar->text().toUtf8().constData());
        m_peer.setConnectionManagerPort(m_portBar->text().toUtf8().constData());
        m_peer.joinNetwork();

        m_connectButton->setEnabled(false);     // disable ablity to reconnect
        m_addrBar->setEnabled(false);
        m_portBar->setEnabled(false);
        refreshPeerList();
        refreshAvailableList();

        setWindowTitle(windowTitle() + QString(" [%1]").arg(QString::fromStdString(m_peer.getAcceptorPort())));
        startAcceptorThread();                  // start seperate thread for recieving connections

    } catch(std::exception& e) {
        QMessageBox::information(
            this,
            applicationName,
            QString("Unable to connect to Connection Manager.\n" \
                    "Please make sure the address and port are valid and that it is running."));

        m_peer.setConnectionManagerAddress("");
        m_peer.setConnectionManagerPort("");
        m_portBar->clear();
        m_addrBar->clear();
    }
}

void MainWindow::s_addShareFile() {
    std::string filepath = QFileDialog::getOpenFileName(this, tr("Share a File")).toUtf8().constData();
    if (filepath.empty()) return;

    try {
        m_peer.addShareFile(filepath);
    } catch (std::invalid_argument& e) {
        std::cerr << "MainWindow::s_addShareFile(): " << e.what() << std::endl;
        QMessageBox::information(
            this,
            applicationName,
            QString("Unable to add file \"%1\" because it already exists.").arg(QFileInfo(QString::fromStdString(filepath)).fileName()));
            return;
    } catch (std::exception& e) {
        std::cerr << "MainWindow::s_addShareFile(): " << e.what() << std::endl;
        QMessageBox::information(
            this,
            applicationName,
            QString("Unable to add file \"%1\". \nMake sure you have connected to the Connection Manager.").arg(QFileInfo(QString::fromStdString(filepath)).fileName()));
            return;
    }
    refreshShareList();
}

void MainWindow::startAcceptorThread() {
    m_acceptorThread = boost::thread(boost::bind(&MainWindow::waitForPeers, this));
}

void MainWindow::waitForPeers() {
    for (;;) {
        m_peer.handleConnection();
        refreshPeerList();
        refreshShareList();
        refreshAvailableList();
    }
}
