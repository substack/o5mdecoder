all: osmxml geom
osmxml: osmxml.c ../o5mdecoder.h
	g++ -I.. osmxml.c -o osmxml --debug
geom: geom.c ../o5mdecoder.h
	g++ -I.. geom.c -o geom --debug
