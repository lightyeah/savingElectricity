#-------------------------------------------------
#
# Project created by QtCreator 2016-07-13T23:18:18
#
#-------------------------------------------------

QT       += core gui
QT += serialport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = readAndWrite
TEMPLATE = app


SOURCES += main.cpp\
        dialog.cpp \
    qcustomplot.cpp

HEADERS  += dialog.h \
    qcustomplot.h

FORMS    += dialog.ui
