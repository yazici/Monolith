# ----------------------------------------------------
# This file is generated by the Qt Visual Studio Add-in.
# ------------------------------------------------------

TEMPLATE = app
TARGET = ComponentEditor
DESTDIR = ../Win32/Debug
QT += core opengl widgets gui quick
CONFIG += debug
DEFINES += WIN64 QT_DLL QT_OPENGL_LIB QT_WIDGETS_LIB QT_QUICK_LIB
INCLUDEPATH += ./GeneratedFiles \
    . \
    ./GeneratedFiles/Debug
LIBS += -lopengl32 \
    -lglu32
DEPENDPATH += .
MOC_DIR += ./GeneratedFiles/debug
OBJECTS_DIR += debug
UI_DIR += ./GeneratedFiles
RCC_DIR += ./GeneratedFiles