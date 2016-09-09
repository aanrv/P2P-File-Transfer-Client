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

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    createWidgets();
    createLayouts();
}

MainWindow::~MainWindow() {

}

void MainWindow::createWidgets() {
    m_peersList = new QListWidget();
    m_filesList = new QListWidget();
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
    botBox->addWidget(m_connectButton);

    centralLayout->addLayout(topBox);
    centralLayout->addLayout(botBox);

    this->setCentralWidget(centralWidget);
    this->setWindowTitle("P2P Fileshare Interface");
}

void MainWindow::s_connect() {
    m_peersList->clear();
    m_filesList->clear();

    m_peersList->insertItem(0, "Peer1");
    m_peersList->insertItem(0, "Peer2");
    m_peersList->insertItem(0, "Peer3");

    m_filesList->insertItem(0, "File1");
    m_filesList->insertItem(0, "File2");
    m_filesList->insertItem(0, "File3");
    m_filesList->insertItem(0, "File4");
    m_filesList->insertItem(0, "File5");
    m_filesList->insertItem(0, "File6");
    m_filesList->insertItem(0, "File7");
    m_filesList->insertItem(0, "File8");
    m_filesList->insertItem(0, "File9");
    m_filesList->insertItem(0, "File10");
    m_filesList->insertItem(0, "File11");
    m_filesList->insertItem(0, "File12");
    m_filesList->insertItem(0, "File13");
    m_filesList->insertItem(0, "File14");
    m_filesList->insertItem(0, "File15");
    m_filesList->insertItem(0, "File16");
    m_filesList->insertItem(0, "File17");
    m_filesList->insertItem(0, "File18");
    m_filesList->insertItem(0, "File19");
    m_filesList->insertItem(0, "File20");

    m_connectButton->setText(tr("Refresh"));
}
