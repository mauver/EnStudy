#-------------------------------------------------
#
# Project created by QtCreator 2015-10-25T18:36:53
#
#-------------------------------------------------

QT       += sql core gui texttospeech network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = EnStudy
TEMPLATE = app

CONFIG += c++11

SOURCES += main.cpp\
        dialog.cpp \
    sqlitemanager.cpp \
    filemanager.cpp \
    webmanager.cpp \
    progressdialog.cpp

HEADERS  += dialog.h \
    sqlitemanager.h \
    sentence.h \
    filemanager.h \
    webmanager.h \
    userdefs.h \
    progressdialog.h

FORMS    += dialog.ui \
    progressdialog.ui

