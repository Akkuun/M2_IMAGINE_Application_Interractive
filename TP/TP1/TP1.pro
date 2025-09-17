MOC_DIR = ./moc
OBJECTS_DIR = ./obj

INCLUDEPATH += $$PWD

HEADERS       = glwidget.h \
                windows/window.h \
                windows/mainwindow.h \
                mesh/mesh.h
SOURCES       = glWidget/glwidget.cpp \
                main.cpp \
                windows/window.cpp \
                windows/mainwindow.cpp \
                mesh/mesh.cpp

RESOURCES += \
    shaders/shaders.qrc

QT           += widgets opengl openglwidgets

# Configuration Debug
CONFIG += debug
QMAKE_CXXFLAGS_DEBUG += -g -O0
QMAKE_LFLAGS_DEBUG += -g


