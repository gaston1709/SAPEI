QT += core gui widgets serialport

CONFIG += c++17

TARGET = qtSAPEI
TEMPLATE = app

SOURCES += main.cpp \
           addcarddialog.cpp \
           mainwindow.cpp

HEADERS += mainwindow.h \
    addcarddialog.h

FORMS += mainwindow.ui \
    addcarddialog.ui

RESOURCES +=
