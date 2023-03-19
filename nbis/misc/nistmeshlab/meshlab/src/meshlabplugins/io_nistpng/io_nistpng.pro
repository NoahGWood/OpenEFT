include (../../shared.pri)
HEADERS       += io_nistpng.h 
SOURCES       += io_nistpng.cpp 
TARGET        = io_nistpng
CONFIG       += opengl

INCLUDEPATH += ../../../../../../include
LIBS += -L../../../../../../lib -lan2k -lioutil -lutil -lfet -ljpegl
