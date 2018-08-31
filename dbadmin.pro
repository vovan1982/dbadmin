#-------------------------------------------------
#
# Project created by QtCreator 2018-08-15T15:45:29
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = dbadmin
TEMPLATE = app
RC_FILE = dbadmin.rc

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        dbadmin.cpp \
    connectdb.cpp \
    addnewuser.cpp \
    createdb.cpp \
    optimizedb.cpp \
    optimizedbworker.cpp \
    importexportdb.cpp \
    importexportdbworker.cpp \
    createdbworker.cpp \
    about.cpp

HEADERS += \
        dbadmin.h \
    connectdb.h \
    addnewuser.h \
    createdb.h \
    optimizedb.h \
    optimizedbworker.h \
    importexportdb.h \
    enums.h \
    importexportdbworker.h \
    createdbworker.h \
    about.h

FORMS += \
        dbadmin.ui \
    connectdb.ui \
    addnewuser.ui \
    createdb.ui \
    optimizedb.ui \
    importexportdb.ui \
    about.ui

RESOURCES += \
    icons.qrc \
    sqlscripts.qrc
