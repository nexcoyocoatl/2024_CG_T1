//
//  InstanciaBZ.cpp
//  Created by Márcio Sarroglia Pinho on 22/09/20.
//
#include "InstanciaBZ.h"

// ***********************************************************
//  void InstanciaPonto(Ponto3D *p, Ponto3D *out)
//  Esta funcao calcula as coordenadas de um ponto no
//  sistema de referencia do universo (SRU), ou seja,
//  aplica as rotacoes, escalas e translacoes a um
//  ponto no sistema de referencia do objeto (SRO).
// ***********************************************************
void InstanciaPonto(Ponto &p, Ponto &out)
{
    GLfloat ponto_novo[4];
    GLfloat matriz_gl[4][4];
    int  i;
    
    glGetFloatv(GL_MODELVIEW_MATRIX,&matriz_gl[0][0]);
    
    for(i=0;i<4;i++)
    {
        ponto_novo[i]= matriz_gl[0][i] * p.x+
        matriz_gl[1][i] * p.y+
        matriz_gl[2][i] * p.z+
        matriz_gl[3][i];
    }
    out.x=ponto_novo[0];
    out.y=ponto_novo[1];
    out.z=ponto_novo[2];
}

Ponto InstanciaPonto(Ponto P)
{
    Ponto temp;
    InstanciaPonto(P, temp);
    return temp;
}

InstanciaBZ::InstanciaBZ()
{
    
    rotacao = 0;
    posicao = Ponto(0,0,0);
    escala = Ponto(1,1,1);
    
    n_curva = 0;
    prox_curva = -1;
    t_atual = 0.0;
    direcao = 1;

    velocidade = 1;
}
InstanciaBZ::InstanciaBZ(Bezier _curva)
{
    rotacao = 0;
    posicao = _curva.getPC(0);
    escala = Ponto(1,1,1);

    n_curva = 0;
    prox_curva = -1;    
    curva = _curva;
    t_atual = 0;
    direcao = 1;

    velocidade = 1;
}

InstanciaBZ::InstanciaBZ(Bezier _curva, int _n_curva, int _direcao)
{
    rotacao = 0;
    
    escala = Ponto(1,1,1);
    
    n_curva = _n_curva;
    prox_curva = -1;
    curva = _curva;

    posicao = _curva.getPC(((_direcao == 1)?0:2)); // Pega ponto inicial ou final da curva dependendo da direção

    direcao = _direcao;
    t_atual = ((direcao == 1)?0:1); // t_atual depende de onde está na curva

    velocidade = 1;
    cor = rand() % 94;
}

void InstanciaBZ::CalculaNovaCurva(Bezier nova_curva)
{
    Ponto p_final_curva = curva.getPC(((direcao == 1)? 2 : 0));
    Ponto p_curva_nova_1 = nova_curva.getPC(0);

    direcao = (p_final_curva == p_curva_nova_1)? 1 : -1;
    t_atual = (direcao == 1)? 0.0 : 1.0;
    posicao = (direcao == 1)? p_curva_nova_1 : nova_curva.getPC(2);

    curva = nova_curva;
    n_curva = prox_curva;
    prox_curva = -1;
}

void InstanciaBZ::MudaDirecao()
{
    direcao *= -1;
    prox_curva = -1;
}

void InstanciaBZ::desenha()
{
    // Escala.imprime("Escala: ");
    // std::cout << std::endl;
    // Aplica as transformacoes geometricas no modelo
    glPushMatrix();
        glTranslatef(posicao.x, posicao.y, 0);
        glRotatef(rotacao, 0, 0, 1);
        glScalef(escala.x, escala.y, escala.z);
        
        (*modelo)(); // desenha a instancia
        
    glPopMatrix();
}
Ponto InstanciaBZ::ObtemPosicao()
{
    // aplica as transformacoes geometricas no modelo
    // desenha a geometria do objeto
    
    glPushMatrix();
        glTranslatef(posicao.x, posicao.y, 0);
        glRotatef(rotacao, 0, 0, 1);
        Ponto PosicaoDoPersonagem;
        Ponto Origem (0,0,0);
        InstanciaPonto(Origem, PosicaoDoPersonagem);
        //PosicaoDoPersonagem.imprime(); std::cout << std::endl;
    glPopMatrix();
    return PosicaoDoPersonagem;
}
void InstanciaBZ::AtualizaPosicao(float tempoDecorrido)
{
    t_atual += curva.CalculaT(velocidade*tempoDecorrido) * ((direcao ==1)?1:-1);
    posicao = curva.Calcula(t_atual);

    // Logica de Rotacao
    
}