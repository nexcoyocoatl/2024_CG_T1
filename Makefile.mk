# Makefile para Windows

PROG = bezier.exe
FONTES = AnimacaoComBezier.cpp Bezier.cpp InstanciaBZ.cpp ListaDeCoresRGB.cpp Poligono.cpp Ponto.cpp Temporizador.cpp
OBJETOS = $(FONTES:.cpp=.o)
CPPFLAGS = -O3 -g -Iinclude # -Wall -g  # Todas as warnings, infos de debug
# Se estiver usando mingw64, troque lib por lib\x64 abaixo
LDFLAGS = -Llib -lfreeglut -lopengl32 -lglu32 -lm
CC = g++

$(PROG): $(OBJETOS)
	$(CC) $(CPPFLAGS) $(OBJETOS) -o $@ $(LDFLAGS)

clean:
	-@ del $(OBJETOS) $(PROG)
