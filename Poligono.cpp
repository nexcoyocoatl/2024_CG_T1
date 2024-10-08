//
//  Poligono.cpp
//  OpenGLTest
//
//  Created by Márcio Sarroglia Pinho on 18/08/20.
//  Copyright © 2020 Márcio Sarroglia Pinho. All rights reserved.
//
#include <iostream>
#include <fstream>
// using namespace std;

#include "Poligono.h"

Poligono::Poligono()
{
    
}

void Poligono::insereVertice(Ponto p)
{
    Vertices.push_back(p);
}

void Poligono::insereVertice(Ponto p, int pos)
{
    if ((pos < 0) || (pos>Vertices.size()))
    {
        std::cout << "Metodo " << __FUNCTION__ << ". Posicao Invalida. Vertice nao inserido." << std::endl;
        return;
    }
    Vertices.insert(Vertices.begin()+pos, p);
}

Ponto Poligono::getVertice(int i)
{
    return Vertices[i];
}

void Poligono::pintaPoligono()
{
    glBegin(GL_POLYGON);
    for (int i=0; i<Vertices.size(); i++)
        glVertex3f(Vertices[i].x,Vertices[i].y,Vertices[i].z);
    glEnd();
}

void Poligono::desenhaPoligono()
{
    glBegin(GL_TRIANGLE_FAN);
    for (int i=0; i<Vertices.size(); i++)
        glVertex3f(Vertices[i].x,Vertices[i].y,Vertices[i].z);
    glEnd();
}
void Poligono::desenhaPoligonoWire()
{
    glBegin(GL_LINE_LOOP);
    for (int i=0; i<Vertices.size(); i++)
        glVertex3f(Vertices[i].x,Vertices[i].y,Vertices[i].z);
    glEnd();
}
void Poligono::desenhaVertices()
{
    glBegin(GL_POINTS);
    for (int i=0; i<Vertices.size(); i++)
        glVertex3f(Vertices[i].x,Vertices[i].y,Vertices[i].z);
    glEnd();
}
void Poligono::imprime()
{
    for (int i=0; i<Vertices.size(); i++)
        Vertices[i].imprime();
}
unsigned long Poligono::getNVertices()
{
    return Vertices.size();
}

void Poligono::obtemLimites(Ponto &Min, Ponto &Max)
{
    Max = Min = Vertices[0];
    
    for (int i=0; i<Vertices.size(); i++)
    {
        Min = ObtemMinimo (Vertices[i], Min);
        Max = ObtemMaximo (Vertices[i], Max);
    }
}

// **********************************************************************
//
// **********************************************************************
void Poligono::LePoligono(const char *nome)
{
    std::ifstream input;            // ofstream arq;
    input.open(nome, std::ios::in); //arq.open(nome, ios::out);
    if (!input)
    {
        std::cout << "Erro ao abrir " << nome << ". " << std::endl;
        exit(0);
    }
    std::cout << "Lendo arquivo " << nome << "...";
    std::string S;
    //int nLinha = 0;
    unsigned int qtdVertices;
    
    input >> qtdVertices;  // arq << qtdVertices

    for (int i=0; i< qtdVertices; i++)
    {
        double x,y;
        // Le cada elemento da linha
        input >> x >> y; // arq << x  << " " << y << std::endl
        if(!input)
            break;
        //nLinha++;
        insereVertice(Ponto(x,y));
    }
    std::cout << "Poligono lido com sucesso!" << std::endl;

}

void Poligono::getAresta(int n, Ponto &P1, Ponto &P2)
{
    P1 = Vertices[n];
    int n1 = (n+1) % Vertices.size();
    P2 = Vertices[n1];
}

void Poligono::desenhaAresta(int n)
{
    glBegin(GL_LINES);
        glVertex3f(Vertices[n].x,Vertices[n].y,Vertices[n].z);
        int n1 = (n+1) % Vertices.size();
        glVertex3f(Vertices[n1].x,Vertices[n1].y,Vertices[n1].z);
    glEnd();
}

void Poligono::alteraVertice(int i, Ponto P)
{
    Vertices[i] = P;
}
