#-------------------------------------------------
# This is a pro file, includes files to built,
# directories and libraries to be included,
# executable name, and build arguments.
# Last update VeysiAdn 03/11/2022 16:58
#-------------------------------------------------
QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DebriderControllerADC
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

CONFIG += c++14

SOURCES += \
        emergency_window.cpp \
        epos4_can.cpp \
        eposthread.cpp \
        footpedal.cpp \
        main.cpp \
        mainwindow.cpp \
        motorthread.cpp \
        initialization_window.cpp

HEADERS += \
        emergency_window.h \
        epos4_can.h \
        eposthread.h \
        footpedal.h \
        m_defines.h \
        mainwindow.h \
        motorthread.h \
        initialization_window.h


FORMS += \
        emergency_window.ui \
        mainwindow.ui \
        initialization_window.ui

RESOURCES += \
    resources.qrc

LIBS += -lEposCmd -lftd2xx -lwiringPi

INCLUDEPATH += /opt
INCLUDEPATH += /opt/vc/lib


INCLUDEPATH += /opt/EposCmdLib_6.5.1.0/lib/v7/



unix:!macx: LIBS += -L/opt/EposCmdLib_6.5.1.0/lib/v7/ -lEposCmd

INCLUDEPATH += /opt/EposCmdLib_6.5.1.0/lib/v7
DEPENDPATH += /opt/EposCmdLib_6.5.1.0/lib/v7


target.path = /home/pi
INSTALLS += target

