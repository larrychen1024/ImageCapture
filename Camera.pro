#-------------------------------------------------
#
# Project created by QtCreator 2016-04-11T21:57:25
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = camera
TEMPLATE = app


SOURCES += main.cpp\
        camera.cpp \
    imageconvert.cpp \
    color.c

HEADERS  += camera.h \
    common.h \
    imageconvert.h \
    color.h

FORMS    += camera.ui


INCLUDEPATH += ./include

#                -I/home/larrychen/qtworkspace/qtmycamera/Camera/include
#------------------For ARM--------------------------------
#LIBS    += -L/home/larrychen/qtworkspace/qtmycamera/Camera/lib_ARM -lcolor \


#------------------For X86--------------------------------
#LIBS += -L/home/larrychen/qtworkspace/qtmycamera/Camera/lib_X86 -lcolor
