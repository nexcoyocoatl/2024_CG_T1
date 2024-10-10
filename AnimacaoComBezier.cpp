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

#define N_INIMIGOS 10

// Protótipos de novas funções
void EncontraProxCurva(int i, int direcao);
bool operator<(Bezier a, Bezier b);
int IndexCurvaDoPonto();
void MudaCurvaDireita();
void MudaCurvaEsquerda();

// struct de pontos de intersecção
typedef struct Intersec
{
    Ponto ponto;
    std::vector<int> curvas;

    Intersec(Ponto _ponto, int num_curva) : ponto(_ponto)
    {
        curvas.push_back(num_curva);
    };
    
} Intersec;

// Variáveis adicionadas
bool personagem_parado = false;
bool game_over = false;
float dash_countdown_max = 100;
float dash_countdown = dash_countdown_max;
float tempo_antigo {};
float tempo {};
float diferenca_tempo {};

Temporizador T;
double AccumDeltaT = 0;
Temporizador T2;

// Arrays agora são arraylists
std::vector<InstanciaBZ*> personagens;
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

    if (AccumDeltaT > 1.0 / 30) // fixa a atualiza��o da tela em 30
    {
        AccumDeltaT = 0;
        angulo += 2;
        glutPostRedisplay();
    }
    
    // Nova lógica de delta_tempo (dt não parece funcionar no Linux)
    tempo = (float)glutGet(GLUT_ELAPSED_TIME);
    diferenca_tempo = (tempo - tempo_antigo);
    tempo_antigo = tempo;

    // Lógica Dash
    if (personagens[0]->dash)
    {
        if (dash_countdown <= 0)
        {
            personagens[0]->dash = false;
            personagens[0]->velocidade = 1;
        }
        else
        {
            dash_countdown -= 0.1*diferenca_tempo;
            personagens[0]->velocidade = 40;
        }        
    }
    else
    {
        if (dash_countdown >= dash_countdown_max)
        {
            dash_countdown = dash_countdown_max;
            if (personagens[0]->velocidade < 3 && !personagem_parado)
            {
                personagens[0]->velocidade = 3;
            }
        }
        else
        {
            if (personagem_parado)
            {
                dash_countdown += 0.05*diferenca_tempo;
            }
            else
            {
                dash_countdown += 0.01*diferenca_tempo;
            }
        }
    }

    // Logica Personagens
    for (int i = 0; i < personagens.size(); i++)
    {
        if (!personagens[i]->vivo)
        {
            continue;
        }
        // Encontra nova curva no próximo ponto
        if (personagens[i]->prox_curva == -1)
        {
            EncontraProxCurva(i, personagens[i]->direcao);
        }

        // Calcula troca para nova curva após sair de uma
        if ((personagens[i]->t_atual > 1 && personagens[i]->direcao == 1)
            || (personagens[i]->t_atual < 0 && personagens[i]->direcao == -1))
        {
            personagens[i]->CalculaNovaCurva(curvas[personagens[i]->prox_curva]);
        }

        // Logica de colisão (mesma curva)
        if ((i != 0) && (personagens[0]->n_curva == personagens[i]->n_curva))
        {
            if (calculaDistancia(personagens[0]->posicao, personagens[i]->posicao) < 1)
            {
                if (personagens[0]->dash)
                {
                    personagens[i]->vivo = false;
                }
                else
                {
                    personagens[0]->vivo = false;
                    game_over = true;
                }                
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

// Desenha player
void DesenhaPersonagem() //
{
    player.desenhaPoligono();
}

// **********************************************************************
// void DesenhaTriangulo()
// **********************************************************************
// Desenha inimigos
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
    int n_ponto = (rand() % 4);
    int n_curva = -1;
    int direcao {};
    int cor {};
    Ponto ponto_curva {};

    // Instância Player
    ponto_curva = pontosIntersec[n_ponto].ponto;
    n_curva = pontosIntersec[n_ponto].curvas[rand() % pontosIntersec[n_ponto].curvas.size()];
    direcao = (pontosIntersec[curvas[n_curva].pontoInicial].ponto == ponto_curva)? 1 : -1;
    cor = BrightGold;

    personagens.emplace_back(new InstanciaBZ(curvas[n_curva], n_curva, direcao, cor));
    personagens[0]->modelo = DesenhaPersonagem;
    personagens[0]->escala = Ponto(1.3, 1.3, 1.3);

    // Instâncias de Inimigos
    for (size_t i = 1; i <= N_INIMIGOS; i++)
    {
        n_ponto = (rand() % (pontosIntersec.size()-4)+4);
        ponto_curva = pontosIntersec[n_ponto].ponto;
        n_curva = pontosIntersec[n_ponto].curvas[rand() % pontosIntersec[n_ponto].curvas.size()];
        direcao = (pontosIntersec[curvas[n_curva].pontoInicial].ponto == ponto_curva)? 1 : -1;
        
        // Encontra cores que não são do cenário ou player
        do
        {
            cor = rand() % 94;
        }
        while (cor == SteelBlue || cor == SkyBlue || cor == YellowGreen || cor == BrightGold || cor == Brass || cor == White);

        personagens.emplace_back(new InstanciaBZ(curvas[n_curva], n_curva, direcao, cor));
        personagens[i]->modelo = DesenhaTriangulo;
    }
}
// **********************************************************************
//  Carrega os modelos que poderão representar os personagens
// **********************************************************************
void CarregaModelos()
{
    player.LePoligono("Nave.txt");
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

    // Lê arquivo .txt de vértices
    input.open("PontosDeControle.txt", std::ios::in);
    if (!input)
    {
        std::cout << "Erro ao abrir PontosDeControle.txt" << std::endl;
        exit(0);
    }
    
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

    // Lê arquivo .txt de curvas
    input.open("PontosCurvas.txt", std::ios::in);
    if (!input)
    {
        std::cout << "Erro ao abrir PontosCurvas.txt" << std::endl;
        exit(0);
    }
    
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

    // Insere curvas em um arraylist
    for (size_t i = 0; i < qtdcurvas; i++)
    {
        Ponto pontoInicialCurva = Ponto(pontosDeControle[pontosCurvas[i][0]].first, pontosDeControle[pontosCurvas[i][0]].second);
        Ponto pontoFinalCurva = Ponto(pontosDeControle[pontosCurvas[i][2]].first, pontosDeControle[pontosCurvas[i][2]].second);

        curvas.emplace_back(pontoInicialCurva,
                            Ponto(pontosDeControle[pontosCurvas[i][1]].first, pontosDeControle[pontosCurvas[i][1]].second),
                            pontoFinalCurva);
        
        // Insere pontos iniciais das curvas em um arraylist de intersecções
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

        // Insere pontos finais das curvas em um arraylist de intersecções
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

    // Ordena as curvas pelo ângulo do seu vetor inicial
    for (size_t i = 0; i < pontosIntersec.size(); i++)
    {
        sort(pontosIntersec[i].curvas.begin(), pontosIntersec[i].curvas.end());
    }

    // Adiciona número dos pontos de intersecções às curvas
    for (size_t i = 0; i < pontosIntersec.size(); i++)
    {
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
    }
}

// **********************************************************************
//
// **********************************************************************
void init()
{
    // Define a cor do fundo da tela
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // carrega os modelos armazenados em arquivos
    CarregaModelos();
    
    // carrega as curvas que farao parte do cenario
    Criacurvas();

    // cria instancias do modelos
    CriaInstancias();    

    // define os limites da área de desenho
    float d = 15;
    Min = Ponto(-d, -d);
    Max = Ponto(d, d);
}

// **********************************************************************
void DesenhaPersonagens(float tempoDecorrido)
{
    // Desenha personagens que estiverem "vivos"
    for (int i = 0; i < personagens.size(); i++)
    {
        if (!personagens[i]->vivo)
        {
            continue;
        }
        if (personagens[i]->dash)
        {
            defineCor(White);
            personagens[i]->escala = Ponto(1.3,1.3,1.3);
        }
        else
        {
            defineCor(personagens[i]->cor);
            personagens[i]->escala = Ponto(1,1,1);
        }

        personagens[i]->AtualizaPosicao(tempoDecorrido);
        personagens[i]->desenha();
    }
}


// **********************************************************************
//
// **********************************************************************
void DesenhaCurvas()
{
    defineCor(SteelBlue);
    glLineWidth(3);
    for (int i = 0; i < curvas.size(); i++)
    {
        curvas[i].Traca();
    }
    // Desenha curvas ligadas ao player
    glLineWidth(4);
    defineCor(SkyBlue);
    curvas[personagens[0]->n_curva].Traca();
    defineCor(Brass);
    curvas[personagens[0]->prox_curva].Traca();
}

// Desenha intersecções das curvas
void DesenhaCruzamentos()
{
    glEnable (GL_POINT_SMOOTH);
    glPointSize(5.0);

    glBegin (GL_POINTS);
    defineCor(YellowGreen);
    for (int i = 0; i < pontosIntersec.size(); i++)
    {
        glVertex2f(pontosIntersec[i].ponto.x, pontosIntersec[i].ponto.y);
    }
    glEnd();
}

// Desenha textos na tela
void DesenhaTextos()
{
    if (game_over)
    {
        std::string game_over_text = "GAME OVER";
        defineCor(White);
        glRasterPos2f(-3, 0);
        for(char& c : game_over_text)
        {
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
        }

        glRasterPos2f(-4, -1.2);        
        game_over_text = "Pressione R para reiniciar";
        for(char& c : game_over_text)
        {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
        }
    }

    std::string dash_text = "DASH: " + std::to_string((int)dash_countdown) + "%";
    defineCor(White);
    glRasterPos2f(10, -14);
    for(char& c : dash_text)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
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

    DesenhaTextos();

    glutSwapBuffers();
}

// **********************************************************************
// ContaTempo(double tempo)
//      conta um certo n�mero de segundos e informa quanto frames
// se passaram neste per�odo.
// **********************************************************************

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

    // Espaço, para o personagem
    case 32:
        if (personagens[0]->velocidade > 0)
        {
            personagem_parado = true;
            personagens[0]->velocidade = 0;
        }
        else
        {
            personagem_parado = false;
            if (dash_countdown < dash_countdown_max)
            {
                personagens[0]->velocidade = 1;
            }
            else
            {
                personagens[0]->velocidade = 3;
            }            
        }
        break;

    // R, Reinicia o jogo
    case 'r':
        game_over = false;
        dash_countdown = dash_countdown_max;
        personagem_parado = false;
        for (size_t i = 0; i < personagens.size(); i++)
        {
            free(personagens[i]);
        }        
        personagens.clear();
        curvas.clear();
        pontosIntersec.clear();
        init();
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
        MudaCurvaEsquerda();
        break;
    case GLUT_KEY_RIGHT:
        MudaCurvaDireita();
        break;
    case GLUT_KEY_UP:
        if (dash_countdown >= dash_countdown_max)
        {
            personagem_parado = false;
            personagens[0]->dash = true;
        }
        break;
    case GLUT_KEY_DOWN:
        personagens[0]->MudaDirecao();
        break;
    default:
        break;
    }
}

// Lógica para encontrar próxima curva
void EncontraProxCurva(int i, int direcao)
{
    // Encontra as próximas curvas pelo ponto de intersecção ao final
    // (ou início, dependendo da direção) da curva em que o personagem está
    int temp {};
    int i_ponto = (personagens[i]->direcao == 1)? personagens[i]->curva.pontoFinal : personagens[i]->curva.pontoInicial;
    int i_curva = rand() % (pontosIntersec[i_ponto].curvas.size());

    // Sorteia curva com rand(), desde que não seja a mesma que o player já está
    while((temp = pontosIntersec[i_ponto].curvas[i_curva]) == personagens[i]->n_curva)
    {
        i_curva = rand() % (pontosIntersec[i_ponto].curvas.size());
    };

    personagens[i]->prox_curva = temp;
    personagens[i]->prox_ponto = i_ponto;
}

// Encontra index da curva pelo próximo ponto de intersecção
int IndexCurvaDoPonto()
{
    int prox_curva = personagens[0]->prox_curva;
    int i_ponto = personagens[0]->prox_ponto;
    return std::find(pontosIntersec[i_ponto].curvas.begin(), pontosIntersec[i_ponto].curvas.end(), prox_curva) - pontosIntersec[i_ponto].curvas.begin();
}

void MudaCurvaDireita()
{
    int i_curva = IndexCurvaDoPonto();
    
    // Lógica para andar pela array de curvas, sem que caia fora da array ou seja do player
    while (true)
    {
        i_curva++;
        if (i_curva > pontosIntersec[personagens[0]->prox_ponto].curvas.size()-1)
        {
            i_curva = 0;
        }
        if (pontosIntersec[personagens[0]->prox_ponto].curvas[i_curva] == personagens[0]->n_curva)
        {
            continue;
        }
        personagens[0]->prox_curva = pontosIntersec[personagens[0]->prox_ponto].curvas[i_curva];
        break;
    }
    
}
void MudaCurvaEsquerda()
{
    int i_curva = IndexCurvaDoPonto();

    while (true)
    {
        i_curva--;
        if (i_curva < 0)
        {
            i_curva = pontosIntersec[personagens[0]->prox_ponto].curvas.size()-1;
        }
        if (pontosIntersec[personagens[0]->prox_ponto].curvas[i_curva] == personagens[0]->n_curva)
        {
            continue;
        }
        personagens[0]->prox_curva = pontosIntersec[personagens[0]->prox_ponto].curvas[i_curva];
        break;
    }
}

// Lógica de comparação de curvas por direção do vetor para ordenação no arraylist
bool operator<(Bezier a, Bezier b)
{
    Ponto p_a;
    Ponto p_b;
    float t_a {};
    float t_b {};
    Ponto pos_a;
    Ponto pos_b;
    bool result;

    if (a.getPC(0) == b.getPC(0) || a.getPC(0) == b.getPC(2))
    {
        p_a = a.getPC(0);
        t_a = 1;
    }
    else
    {
        p_a = b.getPC(2);
        t_a = 0;
    }

    if (p_a == b.getPC(0))
    {
        p_b = b.getPC(0);
        t_b = 1;
    }
    else
    {
        p_b = b.getPC(2);
        t_b = 0;
    }

    pos_a = a.Calcula(t_a +(0.01 * ((t_a == 0)?1:-1)));
    pos_b = b.Calcula(t_b +(0.01 * ((t_b == 0)?1:-1)));
    result = (atan2(pos_a.y, pos_a.x)*180/M_PI) < (atan2(pos_b.y, pos_b.x)*180/M_PI);

    return result;
}

// **********************************************************************
//  void main ( int argc, char** argv )
//
// **********************************************************************
int main(int argc, char **argv)
{
    // std::cout << "Programa OpenGL" << std::endl;
    srand(time(NULL));

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
