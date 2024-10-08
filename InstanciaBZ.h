//
//  Instancia.h
//  OpenGLTest
//
//  Created by Márcio Sarroglia Pinho on 22/09/20.
//  Copyright © 2020 Márcio Sarroglia Pinho. All rights reserved.
//


#ifndef Instancia_hpp
#define Instancia_hpp
#include <iostream>

#include "Bezier.h"
#include "ListaDeCoresRGB.h"

typedef void TipoFuncao();

class InstanciaBZ
{
public:
    InstanciaBZ();
    InstanciaBZ(Bezier _curva);
    InstanciaBZ(Bezier _curva, int _n_curva, int _direcao);
    
    TipoFuncao *modelo; // Modelo a ser desenhado

    Bezier curva; // referencia para a curva onde esta' a instancia
    Ponto posicao, escala;
    float rotacao;

    int n_curva;
    int prox_curva;
    int cor;
    float velocidade;
    float t_atual;
    int direcao; // andando do fim para o inicio, ou ao contrario

    void CalculaNovaCurva(Bezier nova_curva);
    void MudaDirecao();

    void desenha();
    void AtualizaPosicao(float tempoDecorrido);
    Ponto ObtemPosicao();
};

#endif /* Instancia_hpp */
