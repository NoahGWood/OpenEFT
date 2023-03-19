#!/bin/sh
APP_NAME = an2kconvert.exe
#ARCH=-m64
ARCH=-m32

LIB_XERCES = xerces-c
LIB_BOOST_REGEX = boost_regex

LOCAL_LIB = -L/usr/local/lib
LOCAL_INC = -I/<BOOST_INSTALL_ROOT_DRIVE>/boost_1_40_0 -I/usr/local/include

SRC = src
INCLUDE = include
OBJ = obj

HEADER_FILES = $(INCLUDE)/*.hxx $(INCLUDE)/part1/*.hxx $(INCLUDE)/part2/*.hxx $(INCLUDE)/convert/*.hxx $(INCLUDE)/convert/format/*.hxx $(INCLUDE)/validate/*.hxx $(INCLUDE)/validate/text/*.hxx
SRC_FILES = $(SRC)/*.cxx $(SRC)/part1/*.cxx $(SRC)/part2/*.cxx $(SRC)/convert/*.cxx $(SRC)/convert/format/*.cxx $(SRC)/validate/*.cxx $(SRC)/validate/text/*.cxx
OBJ_FILES = $(OBJ)/*.o

.PHONY: all
all: $(APP_NAME)

$(APP_NAME): $(HEADER_FILES) $(SRC_FILES)
	g++ $(ARCH) -O2 -s -g -o $(APP_NAME) $(LOCAL_LIB) $(SRC_FILES) -I$(INCLUDE) $(LOCAL_INC) -l$(LIB_XERCES) -l$(LIB_BOOST_REGEX)

.PHONY : clean
clean:
	mkdir -p $(OBJ)
	rm -f $(OBJ_FILES)
	rm -f $(APP_NAME)

