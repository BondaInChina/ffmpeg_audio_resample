TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    util.cpp

LIBS += -L/usr/local/ffmpeg/lib \
        -lavcodec \
        -lavdevice \
        -lavfilter \
        -lavformat \
        -lavutil \
        -lswresample \
        -lswscale \

INCLUDEPATH += /usr/local/ffmpeg/include

HEADERS += \
    util.h
