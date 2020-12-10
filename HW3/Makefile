SRIPATH ?= /root/srilm-1.5.10
MACHINE_TYPE ?= i686-m64
INC_PATH ?= inc
SRC_PATH ?= src
vpath lib%.a $(SRIPATH)/lib/$(MACHINE_TYPE)
vpath %.c $(SRC_PATH)
vpath %.cpp $(SRC_PATH)

CXX = g++
CXXFLAGS = -O2 -I$(SRIPATH)/include -I$(INC_PATH)

TARGET = mydisambig
SRC = mydisambig.cpp
OBJ = $(SRC:.cpp=.o)
FROM ?= Big5-ZhuYin.map
TO ?= ZhuYin-Big5.map
#DATA ?= corpus.txt
#DATA_SEP ?= corpus_sep.txt
#DATA_COUNT ?= corpus_count.txt
.PHONY: all clean map

$(TARGET): $(OBJ) -loolm -ldstruct -lmisc
	$(CXX) $(LDFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

all: $(TARGET)

map:
	@python3 src/mapping.py $(FROM) $(TO)

#preprocess:
#	@source setup.sh
#	@perl separator_big5.pl $(DATA) > $(DATA_SEP)
#	@ngram-count ‒text $(DATA_SEP) ‒write $(DATA_COUNT) ‒order $3

clean:
	$(RM) $(OBJ) $(TARGET)
