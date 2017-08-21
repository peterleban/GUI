#-------------------------------------------------
#
# Project created by QtCreator 2013-10-28T21:18:40
#
#-------------------------------------------------

QT       += core gui

TARGET = spark
TEMPLATE = app


SOURCES += main.cpp\
        spark.cpp\
        kiss_fft130/kiss_fft.c \
    about.cpp \
    console.cpp \
    reboot.cpp \
    help.cpp

HEADERS  += spark.h \
    about.h \
    console.h \
    reboot.h \
    help.h

FORMS    += spark.ui \
    about.ui \
    console.ui \
    reboot.ui \
    help.ui

INCLUDEPATH += /usr/include/qwt-qt4
LIBS += -l qwt-qt4

OTHER_FILES += \
    liberaspark.png

RESOURCES += \
    images.qrc
