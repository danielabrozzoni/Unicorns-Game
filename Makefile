CXX ?= g++

all:
	$(CXX) -o UnicornsGame -lSDL2 -lSDL2_net -lSDL2_image -lSDL2_ttf -lSDL2_mixer main.cpp vsgl2.cpp
