QT += gui widgets opengl

CONFIG(debug, debug|release){
    DESTDIR = $$OUT_PWD/../bin/debug
}else{
    DESTDIR = $$OUT_PWD/../bin/release
}

CONFIG += c++11 console
CONFIG -= app_bundle

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        globj.cpp \
        main.cpp \
        mainwindow.cpp \
        mat.cpp \
        outputimage.cpp \
        parserpmf.cpp

QMAKE_CXXFLAGS += /openmp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    globj.h \
    mainwindow.h \
    mat.h \
    outputimage.h \
    parserpmf.h

FORMS += \
    mainwindow.ui

RESOURCES += \
    res.qrc

greaterThan(QT_MAJOR_VERSION, 4) {
    TARGET_ARCH=$${QT_ARCH}
} else {
    TARGET_ARCH=$${QMAKE_HOST.arch}
}

contains(TARGET_ARCH, x86_64) {
    BITS = 64
    win32: PLATFORM = x64
} else {
    BITS = 32
    win32: PLATFORM = win32
}

win32{
    defineTest(deployApp){
        win32: EXT = .exe
        DESTFILE = $$DESTDIR/$$TARGET$$EXT
        DESTFILE = \"$$quote($$shell_path($$DESTFILE))\"
        QMAKE_POST_LINK += $$[QT_INSTALL_BINS]/windeployqt --qmldir $$PWD/qml $$DESTFILE $$escape_expand(\\n\\t)
        export(QMAKE_POST_LINK)
    }

    deployApp()
}
