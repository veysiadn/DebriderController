#-------------------------------------------------
#
# Project created by QtCreator 2020-04-10T13:19:08
#
#-------------------------------------------------

QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DebriderControllerTest
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
        emergency_window.cpp \
        epos4_can.cpp \
        main.cpp \
        mainwindow.cpp \
        motorthread.cpp \
        serial_com.cpp

HEADERS += \
        emergency_window.h \
        epos4_can.h \
        m_defines.h \
        mainwindow.h \
        motorthread.h \
        serial_com.h

FORMS += \
        emergency_window.ui \
        mainwindow.ui
INCLUDEPATH += $$[QT_SYSROOT]/opt
INCLUDEPATH += $$[QT_SYSROOT]/opt/vc/lib

INCLUDEPATH += $$[QT_SYSROOT]/usr/include/c++/8/iostream
INCLUDEPATH += $$[QT_SYSROOT]/usr/include
INCLUDEPATH += $$[QT_SYSROOT]/opt/EposCmdLib_6.5.1.0/lib/v7/
LIBS += -lEposCmd -lftd2xx -lwiringPi

target.path = /home/pi
INSTALLS += target

unix:!macx: LIBS += -L$$PWD/../raspi/sysroot/opt/EposCmdLib_6.5.1.0/lib/v7/ -lEposCmd

INCLUDEPATH += $$PWD/../raspi/sysroot/opt/EposCmdLib_6.5.1.0/lib/v7
DEPENDPATH += $$PWD/../raspi/sysroot/opt/EposCmdLib_6.5.1.0/lib/v7

RESOURCES += \
    resources.qrc
