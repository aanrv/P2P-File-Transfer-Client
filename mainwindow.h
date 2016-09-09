#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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

private:
    QListWidget*    m_peersList;        /* Displays list of peers on network. */
    QListWidget*    m_filesList;        /* Displays list of files available for download from other peers. */
    
    QLineEdit*      m_searchBar;        /* Used to search for filenames in peers list. */
    QToolButton*    m_searchButton;

    QPushButton*    m_connectButton;    /* s_connect() */

    void createWidgets();
    void createLayouts();

private slots:
    void s_connect();                   /* Connects client to network and displays peers and files available for download. */
};

#endif // MAINWINDOW_H
