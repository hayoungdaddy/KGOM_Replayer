#-------------------------------------------------
#
# Project created by QtCreator 2018-11-29T01:48:48
#
#-------------------------------------------------

QT       += core gui quick sql location concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = KGOM_Replayer
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    eewinfo.cpp \
    onsiteinfo.cpp \
    pgaalertinfo.cpp \
    pgainfo.cpp

HEADERS += \
        mainwindow.h \
    common.h \
    pgaalertinfo.h \
    regends.h \
    eewinfo.h \
    onsiteinfo.h \
    pgainfo.h

FORMS += \
        mainwindow.ui \
    eewinfo.ui \
    onsiteinfo.ui \
    pgaalertinfo.ui \
    pgainfo.ui

RESOURCES += \
    KGOM_Replayer.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
