#include "config.h"

const int TAMANHOS_NAVIOS[6] = {3, 3, 2, 2, 1, 1};

QueueHandle_t xQueueEventos    = NULL;
TaskHandle_t  xHandleBluetooth = NULL;

volatile bool simBuscarM2    = false;
volatile bool simEsperandoM2 = false;
volatile bool simTurnoM2     = false;
volatile int  simTiroM2X     = 0;
volatile int  simTiroM2Y     = 0;

int  simGridM2[TAM_GRID][TAM_GRID];
bool simM2TirosFeitos[TAM_GRID][TAM_GRID];

void publicarEvento(int tipo, int dado) {
  if (xQueueEventos != NULL) {
    EventoMsg msg = { tipo, dado };
    xQueueSend(xQueueEventos, &msg, portMAX_DELAY);
  }
}

void simIniciarM2() {
  memset(simGridM2, 0, sizeof(simGridM2));
  memset(simM2TirosFeitos, 0, sizeof(simM2TirosFeitos));
  randomSeed(esp_random());

  // Posiciona os 6 navios da M2 aleatoriamente sem sobreposição
  for (int idx = 0; idx < 6; idx++) {
    int tam = TAMANHOS_NAVIOS[idx];
    bool salvo = false;
    for (int tentativa = 0; tentativa < 200 && !salvo; tentativa++) {
      bool horiz = (random(0, 2) == 0);
      int px = random(0, horiz ? (TAM_GRID - tam + 1) : TAM_GRID);
      int py = random(0, horiz ? TAM_GRID : (TAM_GRID - tam + 1));

      bool livre = true;
      for (int i = 0; i < tam; i++) {
        int nx = horiz ? px + i : px;
        int ny = horiz ? py : py + i;
        if (simGridM2[nx][ny] != 0) {
          livre = false;
          break;
        }
      }

      if (livre) {
        for (int i = 0; i < tam; i++) {
          int nx = horiz ? px + i : px;
          int ny = horiz ? py : py + i;
          simGridM2[nx][ny] = 1;
        }
        salvo = true;
      }
    }
  }

  // Imprime localização dos barcos de M2 no Serial Monitor para facilitar testes no Wowki
  Serial.println("[M2 SIM] Mapa secreto de M2 (1 = navio):");
  for (int y = 0; y < TAM_GRID; y++) {
    String linha = "  ";
    for (int x = 0; x < TAM_GRID; x++) {
      linha += String(simGridM2[x][y]) + " ";
    }
    Serial.println(linha);
  }
}
