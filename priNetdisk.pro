TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    prinetdisk.cpp

HEADERS += \
    prinetdisk.h

LIBS += -lboost_filesystem -lboost_system -lpthread

unix:!macx: LIBS += -L$$PWD/../build-NWmanager-Desktop_Qt_5_6_1_GCC_64bit-Debug/ -lNWmanager

INCLUDEPATH += $$PWD/../build-NWmanager-Desktop_Qt_5_6_1_GCC_64bit-Debug
DEPENDPATH += $$PWD/../build-NWmanager-Desktop_Qt_5_6_1_GCC_64bit-Debug
