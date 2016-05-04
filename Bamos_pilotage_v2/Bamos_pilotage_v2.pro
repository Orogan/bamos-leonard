#-------------------------------------------------
#
# Project created by QtCreator 2016-04-25T11:23:37
#
#-------------------------------------------------

QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Bamos_pilotage_v2
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    altivar.cpp

HEADERS  += mainwindow.h \
    altivar.h

FORMS    += mainwindow.ui
