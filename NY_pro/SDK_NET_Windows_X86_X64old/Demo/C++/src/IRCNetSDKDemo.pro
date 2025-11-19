QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = IRCNetSDKDemo
TEMPLATE = app

DEFINES += IRCNETSDKDEMO_LIBRARY
DEFINES += SYS_LINUX
DESTDIR = $$PWD/../bin/linux/
CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    MainWindow.cpp \
    ConfigPage.cpp \
    AlarmPage.cpp \
    PreviewPage.cpp \
    TempAlarmPage.cpp \
    VideoLabel.cpp \
    DownloadPage.cpp \
    PTZPage.cpp \
    ControlPage.cpp \
    MsgBox.cpp \
    PeripheralPage.cpp \




HEADERS += \
    MainWindow.h \
    AlarmPage.h \
    ConfigPage.h \
    PreviewPage.h \
    TempAlarmPage.h \
    VideoLabel.h \
    DownloadPage.h \
    PTZPage.h \
    ControlPage.h \
    MsgBox.h \
    PeripheralPage.h \


FORMS += \
    MainWindow.ui \
    AlarmPage.ui \
    ConfigPage.ui \
    PreviewPage.ui \
    TempAlarmPage.ui \
    DownloadPage.ui \
    PTZPage.ui \
    ControlPage.ui \
    PeripheralPage.ui \

RESOURCES += \
    demo.qrc


INCLUDEPATH += \
    $$PWD/../../../../../include

win32 {
    LIBS += -L$$PWD/../../../../../bin/win/x64 -lIRCNetSDK
}
unix {
    target.path = /usr/lib
    INSTALLS += target
    LIBS += -L$$PWD/../../../../../lib/linux/x86_64 -lIRCNetSDK \
            -lStdPlaySDK -lRTSPClient -lavcodec -lavformat -lswscale -lswresample -lavutil -lSDL2 \
            -lcurl -llz4 -lcrypto -lssl -lPocoCrypto -lPocoFoundation -lPocoJSON -lIvsPlaySDK \
}
#QMAKE_POST_LINK += $$quote(cp $$PWD/*.csv $$DESTDIR)

