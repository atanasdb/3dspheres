#-------------------------------------------------
#
# Project created by QtCreator 2015-07-31T16:31:30
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SphereFlake
TEMPLATE = app

INCLUDEPATH += $$PWD/glm

SOURCES += main.cpp\
    mainwindow.cpp \
    glwidget.cpp \
    camera.cpp \
    raytraycer.cpp \
    scenedata.cpp \

HEADERS  += mainwindow.h \
    glwidget.h \
    asyncrunner.h \
    camera.h \
    raytracer.h \
    scenedata.h \
    settings.h
