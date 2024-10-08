// **********************************************************************
// PUCRS/Escola Polit�cnica
// COMPUTA��O GR�FICA
//
// Programa basico para criar aplicacoes 2D em OpenGL
//
// Marcio Sarroglia Pinho
// pinho@pucrs.br
// **********************************************************************

// Para uso no Xcode:
// Abra o menu Product -> Scheme -> Edit Scheme -> Use custom working directory
// Selecione a pasta onde voce descompactou o ZIP que continha este arquivo.
//

#include <iostream>
#include <cmath>
#include <ctime>
#include <fstream>

#include <array>
#include <algorithm>

#ifdef WIN32
#include <windows.h>
#include <GL/glut.h>
#else
#include <sys/time.h>
#endif

#ifdef __APPLE__
#include <GLUT/glut.h>
#endif

#ifdef __linux__
#include <GL/glut.h>
#endif

#include "Ponto.h"
#include "Poligono.h"
#include "InstanciaBZ.h"

#include "Temporizador.h"
#include "ListaDeCoresRGB.h"

void AssociaPersonagemComCurva(int p, int c);

void EncontraProxCurva(int i, int direcao);

typedef struct Intersec
{
    Ponto ponto;
    std::vector<int> curvas;

    Intersec(Ponto _ponto, int num_curva) : ponto(_ponto)
    {
        curvas.push_back(num_curva);
    };
    
} Intersec;

Temporizador T;
double AccumDeltaT = 0;
Temporizador T2;

std::vector<InstanciaBZ> personagens;
std::vector<Bezier> curvas;
std::vector<Intersec> pontosIntersec;

// Limites logicos da Area de desenho
Ponto Min, Max;

bool desenha = false;

Poligono player, npc;

float angulo = 0.0;

double nFrames = 0;
double TempoTotal = 0;

// **********************************************************************
//
// **********************************************************************
void animate()
{
    double dt;
    dt = T.getDeltaT();
    AccumDeltaT += dt;
    TempoTotal += dt;
    nFrames++;

    if (AccumDeltaT > 1.0 / 30) // fixa a atualiza��o da tela em 30
    {
        AccumDeltaT = 0;
        angulo += 2;
        glutPostRedisplay();
    }
    if (TempoTotal > 5.0)
    {
        std::cout << "Tempo Acumulado: " << TempoTotal << " segundos. ";
        std::cout << "Nros de Frames sem desenho: " << nFrames << std::endl;
        std::cout << "FPS(sem desenho): " << nFrames / TempoTotal << std::endl;
        TempoTotal = 0;
        nFrames = 0;
    }

    // Logica Personagens
    for (int i = 0; i < personagens.size(); i++)
    {
        if (personagens[i].prox_curva == -1)
        {
            EncontraProxCurva(i, personagens[i].direcao);
        }

        if ((personagens[i].t_atual > 1 && personagens[i].direcao == 1)
            || (personagens[i].t_atual < 0 && personagens[i].direcao == -1))
        {
            personagens[i].CalculaNovaCurva(curvas[personagens[i].prox_curva]);
        }

        // if (i != 0)
        if ((i != 0) && (personagens[0].n_curva == personagens[i].n_curva))
        {
            if (calculaDistancia(personagens[0].posicao, personagens[i].posicao) < 0.5)
            {
                std::cout << "GAME OVER" << std::endl;
            }
        }
    }
}
// **********************************************************************
//  void reshape( int w, int h )
//  trata o redimensionamento da janela OpenGL
// **********************************************************************
void reshape(int w, int h)
{
    // Reset the coordinate system before modifying
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // Define a area a ser ocupada pela area OpenGL dentro da Janela
    glViewport(0, 0, w, h);
    // Define os limites logicos da area OpenGL dentro da Janela
    glOrtho(Min.x, Max.x, Min.y, Max.y, -10, +10);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}
// **********************************************************************
// **********************************************************************

void DesenhaPersonagem() //
{
    player.desenhaPoligono();
}

// **********************************************************************
// void DesenhaTriangulo()
// **********************************************************************
void DesenhaTriangulo()
{
    glBegin(GL_LINE_LOOP);

    glVertex2f(-0.5,-0.5);
    glVertex2f(0, 0.5);
    glVertex2f(0.5,-0.5);
    
    glEnd();
}
// **********************************************************************
// Esta funcao deve instanciar todos os personagens do cenario
// **********************************************************************
void CriaInstancias()
{
    // int n_curva = (rand() % 4);
    // std::cout << "Player: " << n_curva << std::endl;
    // personagens.emplace_back(curvas[n_curva], n_curva, (rand() % 2)?-1:1);
    // personagens[0].modelo = DesenhaPersonagem;
    // for (size_t i = 1; i <= 10; i++)
    // {
    //     // while (n_curva == personagens[0].n_curva)
    //     {
    //         n_curva = rand() % (pontosIntersec.size()-4)+4;
    //         std::cout << "Inimigo " << i << ": " << n_curva << std::endl;
    //     }
    //     personagens.emplace_back(curvas[n_curva], n_curva, (rand() % 2)?-1:1);
    //     personagens[i].modelo = DesenhaTriangulo;

    //     // std::cout << "personagem " << i
    //     //           << " Direção: " << personagens[i].direcao
    //     //           << " - Curva: " << personagens[i].n_curva
    //     //           << " ProxCurva: " << personagens[i].prox_curva
    //     //           << " posicao: " << personagens[i].posicao.x << " " << personagens[i].posicao.y
    //     //           << " t_atual: " << personagens[i].t_atual
    //     //           << " ponto inicial: " << personagens[i].curva.pontoInicial
    //     //           << " ponto final: " << personagens[i].curva.pontoFinal << std::endl;

    // }

    int n_ponto = (rand() % 4);
    int n_curva = -1;
    int direcao = 0;
    Ponto ponto_curva {};

    std::cout << "Player: " << n_ponto << std::endl;
    ponto_curva = pontosIntersec[n_ponto].ponto;
    n_curva = pontosIntersec[n_ponto].curvas[rand() % pontosIntersec[n_ponto].curvas.size()];
    direcao = (pontosIntersec[curvas[n_curva].pontoInicial].ponto == ponto_curva)? 1 : -1;

    personagens.emplace_back(curvas[n_curva], n_curva, direcao);
    personagens[0].modelo = DesenhaPersonagem;
    for (size_t i = 1; i <= 10; i++)
    {
        // while (n_curva == personagens[0].n_curva)
        n_ponto = (rand() % (pontosIntersec.size()-4)+4);
        ponto_curva = pontosIntersec[n_ponto].ponto;
        n_curva = pontosIntersec[n_ponto].curvas[rand() % pontosIntersec[n_ponto].curvas.size()];
        direcao = (pontosIntersec[curvas[n_curva].pontoInicial].ponto == ponto_curva)? 1 : -1;
        std::cout << "Inimigo " << i << ": " << n_ponto << std::endl;
        personagens.emplace_back(curvas[n_curva], n_curva, direcao);
        personagens[i].modelo = DesenhaTriangulo;

        // std::cout << "personagem " << i
        //           << " Direção: " << personagens[i].direcao
        //           << " - Curva: " << personagens[i].n_curva
        //           << " ProxCurva: " << personagens[i].prox_curva
        //           << " posicao: " << personagens[i].posicao.x << " " << personagens[i].posicao.y
        //           << " t_atual: " << personagens[i].t_atual
        //           << " ponto inicial: " << personagens[i].curva.pontoInicial
        //           << " ponto final: " << personagens[i].curva.pontoFinal << std::endl;

    }
}
// **********************************************************************
//  Carrega os modelos que poderão representar os personagens
// **********************************************************************
void CarregaModelos()
{
    player.LePoligono("Nave.txt");
    // MeiaSeta.LePoligono("MeiaSeta.txt");
    // Mastro.LePoligono("Mastro.txt");
    
    // Ponto A, B;
    // Mapa.obtemLimites(A,B);
    // std::cout << "Limites do Mapa" << std::endl;
    // A.imprime();
    // std::cout << std::endl;
    // B.imprime();
    // std::cout << std::endl;
}
// **********************************************************************
//  Este metodo deve ser alterado para ler as curvas de um arquivo texto
// **********************************************************************
void Criacurvas()
{

    std::ifstream input;
    size_t qtdPontosControle;
    size_t qtdcurvas;
    std::vector<std::pair<double, double>> pontosDeControle;
    std::vector<std::array<double, 3>> pontosCurvas;

    input.open("PontosDeControle.txt", std::ios::in);
    if (!input)
    {
        std::cout << "Erro ao abrir PontosDeControle.txt." << std::endl;
        exit(0);
    }
    std::cout << "Lendo arquivo PontosDeControle.txt..." << std::endl;
    
    input >> qtdPontosControle;
    pontosDeControle.resize(qtdPontosControle);

    for (int i=0; i < qtdPontosControle; i++)
    {
        if(!input)
        {
            break;
        }

        input >> pontosDeControle[i].first >> pontosDeControle[i].second;
    }
    input.close();

    input.open("PontosCurvas.txt", std::ios::in);
    if (!input)
    {
        std::cout << "Erro ao abrir PontosCurvas.txt." << std::endl;
        exit(0);
    }
    std::cout << "Lendo arquivo PontosCurvas.txt..." << std::endl;
    
    input >> qtdcurvas;
    pontosCurvas.resize(qtdcurvas);

    for (int i=0; i < qtdcurvas; i++)
    {
        if(!input)
        {
            break;
        }
        
        input >> pontosCurvas[i][0] >> pontosCurvas[i][1] >> pontosCurvas[i][2];
    }
    input.close();

    // Lógica péssima ARRUMAR
    for (size_t i = 0; i < qtdcurvas; i++)
    {
        Ponto pontoInicialCurva = Ponto(pontosDeControle[pontosCurvas[i][0]].first, pontosDeControle[pontosCurvas[i][0]].second);
        Ponto pontoFinalCurva = Ponto(pontosDeControle[pontosCurvas[i][2]].first, pontosDeControle[pontosCurvas[i][2]].second);

        curvas.emplace_back(pontoInicialCurva,
                            Ponto(pontosDeControle[pontosCurvas[i][1]].first, pontosDeControle[pontosCurvas[i][1]].second),
                            pontoFinalCurva);
        

        auto it = std::find_if(pontosIntersec.begin(), pontosIntersec.end(), [&pontoInicialCurva](const Intersec& intersec) {
            return intersec.ponto == pontoInicialCurva;
        });
        if (it == pontosIntersec.end())
        {
            pontosIntersec.emplace_back(pontoInicialCurva, i);
        }
        else
        {
            size_t n_it = std::distance(pontosIntersec.begin(), it);
            if (std::find(pontosIntersec[n_it].curvas.begin(), pontosIntersec[n_it].curvas.end(), i) == pontosIntersec[n_it].curvas.end())
            {
                pontosIntersec[n_it].curvas.push_back(i);
            }
        }

        it = std::find_if(pontosIntersec.begin(), pontosIntersec.end(), [&pontoFinalCurva](const Intersec& intersec) {
            return intersec.ponto == pontoFinalCurva;
        });
        if (it == pontosIntersec.end())
        {
            pontosIntersec.emplace_back(pontoFinalCurva, i);
        }
        else
        {
            size_t n_it = std::distance(pontosIntersec.begin(), it);
            if (std::find(pontosIntersec[n_it].curvas.begin(), pontosIntersec[n_it].curvas.end(), i) == pontosIntersec[n_it].curvas.end())
            {
                pontosIntersec[n_it].curvas.push_back(i);
            }
        }
    }

    // Adiciona número dos pontos de intersecções às curvas
    for (size_t i = 0; i < pontosIntersec.size(); i++)
    {
        pontosIntersec[i].ponto.imprime();
        for (size_t j = 0; j < pontosIntersec[i].curvas.size(); j++)
        {
            int n_curva = pontosIntersec[i].curvas[j];
            if (curvas[n_curva].getPC(0) == pontosIntersec[i].ponto)
            {
                curvas[n_curva].pontoInicial = i;
            }
            else if (curvas[n_curva].getPC(2) == pontosIntersec[i].ponto)
            {
                curvas[n_curva].pontoFinal = i;
            }
        }
        std::cout << std::endl;
    }

    // Print Debug
    // for (size_t i = 0; i < curvas.size(); i++)
    // {
    //     std::cout << "Curva " << i
    //               << " - Inicio: " << curvas[i].pontoInicial
    //               << " Fim: " << curvas[i].pontoFinal << std::endl;
    // }
}
// **********************************************************************
//
// **********************************************************************
void AssociaPersonagemComCurva(int p, int c)
{
    personagens[p].curva = curvas[c];
}
// **********************************************************************
//
// **********************************************************************
void init()
{
    srand(time(NULL));
    // Define a cor do fundo da tela
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // carrega os modelos armazenados em arquivos
    CarregaModelos();
    
    // carrega as curvas que farao parte do cenario
    Criacurvas();

    // cria instancias do modelos
    CriaInstancias();

    // Debug
    // for (size_t i = 0; i < curvas.size(); i++)
    // {
    //     std::cout << "Curva " << i << " Inicio: " << curvas[i].getPC(0).x << "," << curvas[i].getPC(0).y
    //     << " | Fim: " << curvas[i].getPC(2).x << "," << curvas[i].getPC(2).y << "| ponto inicial: " << curvas[i].pontoInicial << " ponto final: " << curvas[i].pontoFinal  << std::endl;
    // }
    // for (size_t i = 0; i < pontosIntersec.size(); i++)
    // {
    //     std::cout << "Ponto Intersec " << i << ": " << pontosIntersec[i].ponto.x << "," << pontosIntersec[i].ponto.y  << " | curvas: ";
    //     for (size_t j = 0; j < pontosIntersec[i].curvas.size(); j++)
    //     {
    //         std::cout << pontosIntersec[i].curvas[j];
    //         if (j < (pontosIntersec[i].curvas.size() - 1))
    //         {
    //             std::cout << ", ";
    //         }
    //     }
    //     std::cout << std::endl;
    // }   
    

    // define os limites da área de desenho
    float d = 15;
    Min = Ponto(-d, -d);
    Max = Ponto(d, d);
}

// **********************************************************************
void DesenhaPersonagens(float tempoDecorrido)
{
    for (int i = 0; i < personagens.size(); i++)
    {
        defineCor(personagens[i].cor);
        personagens[i].AtualizaPosicao(tempoDecorrido);
        personagens[i].desenha();
    }
}

// void DesenhaPoligonoDeControle(int curva)
// {
//     Ponto P;
//     glBegin(GL_LINE_STRIP);
//     for (int v=0;v<3;v++)
//     {
//         P = curvas[curva].getPC(v);
//         glVertex2d(P.x, P.y);
//     }
//     glEnd();
// }
// **********************************************************************
//
// **********************************************************************
void DesenhaCurvas()
{
    defineCor(CadetBlue);
    glLineWidth(3);
    for (int i = 0; i < curvas.size(); i++)
    {
        // defineCor(OrangeRed);
        // glLineWidth(3);
        curvas[i].Traca();
        // defineCor(VioletRed);
        // glLineWidth(2);
        // DesenhaPoligonoDeControle(i);
    }
    defineCor(OrangeRed);
    glLineWidth(4);
    curvas[personagens[0].prox_curva].Traca();
}

void DesenhaCruzamentos()
{
    glEnable (GL_POINT_SMOOTH);
    glPointSize(5.0);

    glBegin (GL_POINTS);
    defineCor(YellowGreen);
    for (int i = 0; i < pontosIntersec.size(); i++)
    {
        glVertex2f(pontosIntersec[i].ponto.x, pontosIntersec[i].ponto.y);
        // curvas[i].Traca();
        // defineCor(VioletRed);
        // glLineWidth(2);
        // DesenhaPoligonoDeControle(i);
    }
    glEnd();
}

// **********************************************************************
//  void display( void )
// **********************************************************************
void display(void)
{
    // Limpa a tela coma cor de fundo
    glClear(GL_COLOR_BUFFER_BIT);

    // Define os limites l�gicos da �rea OpenGL dentro da Janela
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    // Coloque aqui as chamadas das rotinas que desenham os objetos
    // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

    glLineWidth(1);
    glColor3f(1, 1, 1); // R, G, B  [0..1]

    // DesenhaEixos();

    DesenhaCurvas();

    DesenhaCruzamentos();
    
    // Desenha os personagens no tempo T2.getDeltaT()
    DesenhaPersonagens(T2.getDeltaT());

    glutSwapBuffers();
}

// **********************************************************************
// ContaTempo(double tempo)
//      conta um certo n�mero de segundos e informa quanto frames
// se passaram neste per�odo.
// **********************************************************************
void ContaTempo(double tempo)
{
    Temporizador T;

    unsigned long cont = 0;
    std::cout << "Inicio contagem de " << tempo << "segundos ..." << std::flush;
    while (true)
    {
        tempo -= T.getDeltaT();
        cont++;
        if (tempo <= 0.0)
        {
            std::cout << "fim! - Passaram-se " << cont << " frames." << std::endl;
            break;
        }
    }
}

// **********************************************************************
//  void keyboard ( unsigned char key, int x, int y )
// **********************************************************************
void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 27:     // Termina o programa qdo
        exit(0); // a tecla ESC for pressionada
        break;
    case 't':
        ContaTempo(3);
        break;
    case ' ':
        desenha = !desenha;
        break;
    default:
        break;
    }
}

// **********************************************************************
//  void arrow_keys ( int a_keys, int x, int y )
// **********************************************************************
void arrow_keys(int a_keys, int x, int y)
{
    switch (a_keys)
    {
    case GLUT_KEY_LEFT:
        personagens[0].posicao.x -= 0.5;
        break;
    case GLUT_KEY_RIGHT:
        personagens[0].rotacao++;
        break;
    case GLUT_KEY_UP:
        if (personagens[0].velocidade > 0)
        {
            personagens[0].velocidade = 0;
        }
        else
        {
            personagens[0].velocidade = 1;
        }
        // glutFullScreen(); // Vai para Full Screen
        break;
    case GLUT_KEY_DOWN:
        personagens[0].MudaDirecao();
        // glutPositionWindow(50, 50);
        // glutReshapeWindow(700, 500);
        break;
    case 'r':
    {
        std::cout << "Dsadadsadsadsa";
        init();
        break;
    }
    default:
        break;
    }
}

void EncontraProxCurva(int i, int direcao)
{
    int temp {};
    int i_ponto = (personagens[i].direcao == 1)? personagens[i].curva.pontoFinal : personagens[i].curva.pontoInicial;
    std::cout << i_ponto << std::endl;
    int i_curva = rand() % (pontosIntersec[i_ponto].curvas.size());
    while((temp = pontosIntersec[i_ponto].curvas[i_curva]) == personagens[i].n_curva)
    {
        i_curva = rand() % (pontosIntersec[i_ponto].curvas.size());
    };
    // std::cout << "TEMP: " << temp << " Curva " << pontosIntersec[i_ponto].curvas[i_curva] << " = " << personagens[i].n_curva << "?"<< std::endl;

    personagens[i].prox_curva = temp;
    // std::cout << "Nova Curva - Ponto: " << i_ponto << " " << "Curva: " << pontosIntersec[i_ponto].curvas[i_curva] << " " << curvas[i_curva].getPC(0).x << " " << curvas[i_curva].getPC(0).y << std::endl;
}

// **********************************************************************
//  void main ( int argc, char** argv )
//
// **********************************************************************
int main(int argc, char **argv)
{
    std::cout << "Programa OpenGL" << std::endl;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGB);
    glutInitWindowPosition(0, 0);

    // Define o tamanho inicial da janela grafica do programa
    glutInitWindowSize(700, 500);

    // Cria a janela na tela, definindo o nome da
    // que aparecera na barra de t�tulo da janela.
    glutCreateWindow("Animacao com Bezier");

    // executa algumas inicializa��es
    init();

    // Define que o tratador de evento para
    // o redesenho da tela. A funcao "display"
    // ser� chamada automaticamente quando
    // for necess�rio redesenhar a janela
    glutDisplayFunc(display);

    // Define que o tratador de evento para
    // o invalida��o da tela. A funcao "display"
    // ser� chamada automaticamente sempre que a
    // m�quina estiver ociosa (idle)
    glutIdleFunc(animate);

    // Define que o tratador de evento para
    // o redimensionamento da janela. A funcao "reshape"
    // ser� chamada automaticamente quando
    // o usu�rio alterar o tamanho da janela
    glutReshapeFunc(reshape);

    // Define que o tratador de evento para
    // as teclas. A funcao "keyboard"
    // ser� chamada automaticamente sempre
    // o usu�rio pressionar uma tecla comum
    glutKeyboardFunc(keyboard);

    // Define que o tratador de evento para
    // as teclas especiais(F1, F2,... ALT-A,
    // ALT-B, Teclas de Seta, ...).
    // A funcao "arrow_keys" ser� chamada
    // automaticamente sempre o usu�rio
    // pressionar uma tecla especial
    glutSpecialFunc(arrow_keys);

    // inicia o tratamento dos eventos
    glutMainLoop();

    return 0;
}
