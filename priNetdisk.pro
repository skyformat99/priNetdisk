TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    prinetdisk.cpp \
    nwmanger.cpp \
    session.cpp

HEADERS += \
    prinetdisk.h \
    nwmanger.h \
    session.h

LIBS += -lboost_filesystem -lboost_system
