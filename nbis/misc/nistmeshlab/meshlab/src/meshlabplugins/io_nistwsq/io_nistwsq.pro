include (../../shared.pri)
HEADERS       += io_nistwsq.h 
SOURCES       += io_nistwsq.cpp 
TARGET        = io_nistwsq
CONFIG       += opengl

INCLUDEPATH += ../../../../../../include
LIBS += -L../../../../../../lib -lwsq -lioutil -lutil -lfet -ljpegl
#LIBS += -L../../../../../../lib -Wl,--start-group -lwsq -lioutil -lutil -lfet -Wl,--end-group
#LIBS += -L../../../../../../lib -lwsq -lan2k -lbozorth3 -lcblas -lclapck -lf2c -lfet -lfft -lihead -limage -ljpegb -ljpegl -lmindtct -lmlp -lnfiq -lnfseg -lopenjpeg -lpca -lpcautil -lpcax -lpng -lutil -lz
