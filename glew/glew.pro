TEMPLATE = lib 
TARGET = glews
QT =
CONFIG = staticlib 
HEADERS = include/gl/glew.h \
	include/gl/wglew.h 

SOURCES += src/glew.c 

RESOURCES += 
INCLUDEPATH += include

win32: DEFINES += _PLATFORM_AGNOSTIC_ _CRT_SECURE_NO_WARNINGS WIN32 NDEBUG _LIB WIN32_LEAN_AND_MEAN VC_EXTRALEAN GLEW_STATIC

mac: QMAKE_CXXFLAGS += -fasm-blocks

