QT += gui widgets opengl

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
