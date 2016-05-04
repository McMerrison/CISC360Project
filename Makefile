ifdef GPU
CXX := pgc++
CXXFLAGS := -acc -ta=nvidia,7.5,cc50 -O3 -Minfo -std=c++11
SRC := PixelPaintersACC.cpp
else
CXX := g++
CXXFLAGS := -O3 -march=native
SRC := PixelPaintersSerial.cpp
endif

TARGET := $(SRC:.cpp=)

default : $(TARGET)

all :
	make -C .
	make -C . GPU=1

clean :
	-rm -rf PixelPainters PixelPaintersACC