#include "componentes.h"
//Define botões e matrizes;
TBotao BCima  (PIN_B_CIMA,  EV_BOTOES_MOVIMENTO, DIR_CIMA);
TBotao BDir   (PIN_B_DIR,   EV_BOTOES_MOVIMENTO, DIR_DIR);
TBotao BBaixo (PIN_B_BAIXO, EV_BOTOES_MOVIMENTO, DIR_BAIXO);
TBotao BEsq   (PIN_B_ESQ,   EV_BOTOES_MOVIMENTO, DIR_ESQ);
TBotao BGira  (PIN_B_GIRA,  EV_BOTOES_MOVIMENTO, DIR_GIRA);
TBotao BOK    (PIN_B_OK,    EV_OK,               DIR_NENHUM);

TDisplay   display;
TMatrizLED matrizLED;
TNavio     navio;
TTabuleiro tabuleiro;
