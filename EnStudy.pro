#-------------------------------------------------
#
# Project created by QtCreator 2015-10-25T18:36:53
#
#-------------------------------------------------

QT       += sql core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = EnStudy
TEMPLATE = app

CONFIG += c++11

SOURCES += main.cpp\
        dialog.cpp \
    sqlitemanager.cpp

HEADERS  += dialog.h \
    sqlitemanager.h \
    sentence.h

FORMS    += dialog.ui

