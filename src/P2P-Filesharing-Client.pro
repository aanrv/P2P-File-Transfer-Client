#-------------------------------------------------
#
# Project created by QtCreator 2016-09-09T12:56:03
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = P2P-Filesharing-Client
TEMPLATE = app

QMAKE_CXXFLAGS += -Wall \
    -Wextra \
    -Werror \
    -pedantic \
    -std=c++11

LIBS += -lboost_system \
    -lboost_thread \
    -lboost_filesystem

SOURCES += main.cpp \
    mainwindow.cpp \
    peer.cpp \
    p2pnode.cpp

HEADERS  += mainwindow.h \
    peer.h \
    p2pnode.h
