#-------------------------------------------------
#
# Project created by QtCreator 2016-09-09T12:56:03
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = P2P-Filesharing-Client
TEMPLATE = app

LIBS += -lboost_system

SOURCES += main.cpp\
        mainwindow.cpp \
    peer.cpp \
    p2pnode.cpp

HEADERS  += mainwindow.h \
    peer.h \
    p2pnode.h
