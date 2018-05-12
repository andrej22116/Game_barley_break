TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp


LIBS += -lGL -lGLU
LIBS += -lglut

HEADERS += stb_image.h \
    constglobalvariables.h
