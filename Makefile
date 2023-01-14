# Simple makefile for utils

CC=g++
WCC=i686-w64-mingw32-g++
SRC=src
WBIN=win32
BIN=bin
INSTALL_DIR=~/.local/bin

all: c1541

c1541: $(SRC)/c1541.cpp
	$(CC) -o $(BIN)/c1541 $(SRC)/c1541.cpp $(SRC)/comm/Buffer.cpp $(SRC)/comm/ByteArray.cpp $(SRC)/comm/CBM.cpp $(SRC)/comm/Interface.cpp $(SRC)/comm/Config.cpp $(SRC)/comm/Serial.cpp $(SRC)/disks/Disk.cpp $(SRC)/disks/NativeFs.cpp $(SRC)/disks/PrgFile.cpp
#	$(WCC) -o $(WBIN)/c1541 $(SRC)/c1541.cpp $(SRC)/Buffer.cpp $(SRC)/mainw.cpp

clean:
	rm -f $(WBIN)/* $(BIN)/* *~ $(SRC)/*~ 

install:
	cp $(BIN)/* $(INSTALL_DIR)/
