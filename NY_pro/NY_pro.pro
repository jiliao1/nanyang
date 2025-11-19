QT       += core gui network multimedia multimediawidgets websockets sql \
            3dcore 3drender 3dextras 3dinput

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ffmpegplayer.cpp \
    ipconnect.cpp \
#    logmanager.cpp \
    main.cpp \
    mapinteractiveview.cpp \
    overlaywidget.cpp \
    pointcloudrenderer.cpp \
    pointcloudviewwidget.cpp \
    customcameracontroller.cpp \
#    reporttablemodel.cpp \
    robot.cpp \
#    tcpclient.cpp \
    videomanager.cpp \
    videowallwidget.cpp\
    database.cpp \
    signup.cpp

HEADERS += \
    ffmpegplayer.h \
    ipconnect.h \
#    logmanager.h \
    mapinteractiveview.h \
    overlaywidget.h \
    pointcloudrenderer.h \
    pointcloudviewwidget.h \
    customcameracontroller.h \
#    reporttablemodel.h \
    robot.h \
#    tcpclient.h \
    videomanager.h \
    videowallwidget.h \
    videowidget.h \
    database.h \
    signup.h

FORMS += \
    ipconnect.ui \
    robot.ui \
    signup.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
include(xlsx/xlsx/qtxlsx.pri)

RESOURCES += res.qrc

# FFMpeg Integration
# 指定FFmpeg头文件所在的目录
INCLUDEPATH += $$PWD/3rdparty/ffmpeg/include

# 指定FFmpeg库文件所在的目录，并链接所有需要的库
# -L 表示库目录, -l 表示要链接的具体库 (去掉lib前缀和.lib后缀)
LIBS += -L$$PWD/3rdparty/ffmpeg/lib -lavformat -lavcodec -lavutil -lswscale

win32-msvc* {
    QMAKE_CXXFLAGS += /FS
}

# 排除 release 和 debug 目录,避免文件冲突
CONFIG(release, debug|release) {
    DESTDIR = release
}
CONFIG(debug, debug|release) {
    DESTDIR = debug
}
