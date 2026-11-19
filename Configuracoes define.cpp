#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#define NUM_ESTADOS 10
#define NUM_EVENTOS 17
#define TAM_GRID    8   // Matriz 8x8 no Wokwi

//Chance (%) de o pareamento Bluetooth simulado ter sucesso (0 a 100), para simular uma eventual falha
#define CHANCE_CONEXAO_BT 50

#define PIN_B_CIMA   13
#define PIN_B_DIR    12
#define PIN_B_BAIXO  14
#define PIN_B_ESQ    27
#define PIN_B_GIRA   26
#define PIN_B_OK     25
#define PIN_LED_MTX   4

//Estados 
#define E0_IDLE                      0
#define E1_PAREAMENTO_BT             1
#define E2_NAVEGACAO                 2
#define E3_VERIFICACAO               3
#define E4_CONFIRMA_POSICIONAMENTO   4
#define E5_ESPERA_POSICIONAMENTO_M2  5
#define E6_SORTEIO                   6
#define E7_TURNO_M1                  7
#define E8_TURNO_M2                  8
#define E9_CONFIRMA_FIM_JOGO         9

//Eventos
#define NENHUM_EVENTO               -1
#define EV_OK                        0
#define EV_TIMEOUT_BT_PAREAMENTO     1
#define EV_ENCONTRA_DISPOSITIVO      2
#define EV_BOTOES_MOVIMENTO          3
#define EV_POSICAO_ILEGAL            4
#define EV_POSICAO_LEGAL             5
#define EV_MENOS_6_NAVIOS            6
#define EV_SEIS_NAVIOS               7
#define EV_TIMEOUT_BT_POSICIONAMENTO 8
#define EV_M2_PRONTO                 9
#define EV_SORTEIO_M1               10
#define EV_SORTEIO_M2               11
#define EV_TIMEOUT_BT_TURNO         12
#define EV_OK_BT                    13
#define EV_FIM_DE_JOGO              14
#define EV_JOGO_CONTINUA_M2         15
#define EV_JOGO_CONTINUA_M1         16

//Ações
#define NENHUMA_ACAO                -1
#define A00           0
#define A01           1
#define A02           2
#define A07           3
#define A08           4
#define A09           5
#define A10           6
#define A12           7
#define A16           8
#define A17           9
#define A18          10
#define A_TIMEOUT_BT 11  // Feedback visual ao voltar para E0 por falha/timeout BT
#define A20          12
#define A21          13

//Direções dos botões (lido como dado nas funções)
#define DIR_NENHUM 0
#define DIR_CIMA   1
#define DIR_BAIXO  2
#define DIR_ESQ    3
#define DIR_DIR    4
#define DIR_GIRA   5

//Tamanhos dos 6 navios a serem posicionados, definimos antes como 3, 3, 2, 2, 1 e 1
extern const int TAMANHOS_NAVIOS[6];

struct Ponto {
  int x;
  int y;
};

struct EventoMsg {
  int tipo;
  int dado;
};

extern QueueHandle_t xQueueEventos;
extern TaskHandle_t  xHandleBluetooth;

//Simulação da Máquina 2 no Wokwi, por ausência de bluetooth
extern volatile bool simBuscarM2;
extern volatile bool simEsperandoM2;
extern volatile bool simTurnoM2;
extern volatile int  simTiroM2X;
extern volatile int  simTiroM2Y;

//Tabuleiro e tiros simulados do M2
extern int  simGridM2[TAM_GRID][TAM_GRID];
extern bool simM2TirosFeitos[TAM_GRID][TAM_GRID];

void publicarEvento(int tipo, int dado = DIR_NENHUM);
void simIniciarM2();

#endif
