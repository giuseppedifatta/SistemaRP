#-------------------------------------------------
#
# Project created by QtCreator 2017-09-11T15:11:47
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SistemaRP
TEMPLATE = app


SOURCES += main.cpp\
        mainwindowrp.cpp \
    useragentrp.cpp \
    sslclient.cpp

HEADERS  += mainwindowrp.h \
    useragentrp.h \
    sslclient.h

FORMS    += mainwindowrp.ui

LIBS        += -L/usr/local/lib -L/usr/lib \
            -lcryptopp \
    -lssl \
    -lcrypto \


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../usr/lib/x86_64-linux-gnu/release/ -lssl
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../usr/lib/x86_64-linux-gnu/debug/ -lssl
else:unix: LIBS += -L$$PWD/../../../../usr/lib/x86_64-linux-gnu/ -lssl


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../usr/lib/x86_64-linux-gnu/release/ -lcryptopp
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../usr/lib/x86_64-linux-gnu/debug/ -lcryptopp
else:unix: LIBS += -L$$PWD/../../../../usr/lib/x86_64-linux-gnu/ -lcryptopp

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../usr/lib/i386-linux-gnu/release/ -ltinyxml2
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../usr/lib/i386-linux-gnu/debug/ -ltinyxml2
else:unix: LIBS += -L$$PWD/../../../../usr/lib/i386-linux-gnu/ -ltinyxml2

INCLUDEPATH += $$PWD/../../../../usr/lib/i386-linux-gnu
DEPENDPATH += $$PWD/../../../../usr/lib/i386-linux-gnu

INCLUDEPATH += $$PWD/../../../../usr/include/cryptopp
DEPENDPATH += $$PWD/../../../../usr/include/cryptopp
