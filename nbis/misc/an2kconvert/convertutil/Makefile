APP_NAME = an2kconvert

LIB_XERCES = xerces-c
LIB_BOOST_REGEX = boost_regex

SRC = src
INCLUDE = include
OBJ = obj

HEADER_FILES = $(INCLUDE)/*.hxx $(INCLUDE)/part1/*.hxx $(INCLUDE)/part2/*.hxx $(INCLUDE)/convert/*.hxx $(INCLUDE)/convert/format/*.hxx $(INCLUDE)/validate/*.hxx $(INCLUDE)/validate/text/*.hxx
SRC_FILES = $(SRC)/*.cxx $(SRC)/part1/*.cxx $(SRC)/part2/*.cxx $(SRC)/convert/*.cxx $(SRC)/convert/format/*.cxx $(SRC)/validate/*.cxx $(SRC)/validate/text/*.cxx
OBJ_FILES = $(OBJ)/*.o

.PHONY: all
all: $(APP_NAME)

$(APP_NAME): $(HEADER_FILES) $(SRC_FILES) -l$(LIB_XERCES) -l$(LIB_BOOST_REGEX)
	g++ -g -o $(APP_NAME) $(SRC_FILES) -I$(INCLUDE) -l$(LIB_XERCES) -l$(LIB_BOOST_REGEX)

clean:
	mkdir -p $(OBJ)
	rm -f $(OBJ_FILES)
	rm -f $(APP_NAME)

