TEMPLATE = app
TARGET = bsbenchmark

QT       += testlib
QT       -= gui

CONFIG   += console
CONFIG   -= app_bundle

SOURCES += bsbenchmark.cpp

DEFINES += SRCDIR=\\\"$$PWD/\\\"
