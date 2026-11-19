#ifndef TASKS_H
#define TASKS_H

#include "config.h"
#include "componentes.h"
#include "fsm.h"


void vTaskInputHandler(void *pvParameters) {
  BCima.setup(); BDir.setup(); BBaixo.setup();
  BEsq.setup();  BGira.setup(); BOK.setup(); // Define os botões de movimentação, giro e confirmação e deixa a checkagem de sua atualização em loop

  for (;;) {
    if (BCima.update())  publicarEvento(BCima.getEvento(),  BCima.getDado());
    if (BDir.update())   publicarEvento(BDir.getEvento(),   BDir.getDado());
    if (BBaixo.update()) publicarEvento(BBaixo.getEvento(), BBaixo.getDado());
    if (BEsq.update())   publicarEvento(BEsq.getEvento(),   BEsq.getDado());
    if (BGira.update())  publicarEvento(BGira.getEvento(),  BGira.getDado());
    if (BOK.update())    publicarEvento(BOK.getEvento(),    BOK.getDado());

    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

void vTaskBluetoothSim(void *pvParameters) {
  for (;;) {
    if (simBuscarM2) { //Achou M2
      vTaskDelay(pdMS_TO_TICKS(1500)); 
      simBuscarM2 = false; 

      // Aleatoriza se a conexão Bluetooth conseguiu parear ou falhou (timeout)
      int sorte = esp_random() % 100;
      if (sorte < CHANCE_CONEXAO_BT) {
        Serial.printf("[BT SIM] Sucesso (%d < %d)! Maquina 2 encontrada e pareada!\n",
                      sorte, CHANCE_CONEXAO_BT);
        publicarEvento(EV_ENCONTRA_DISPOSITIVO);
      } else {
        Serial.printf("[BT SIM] Falha (%d >= %d)! M2 nao respondeu (Timeout).\n",
                      sorte, CHANCE_CONEXAO_BT);
        publicarEvento(EV_TIMEOUT_BT_PAREAMENTO);
      }
    }

    if (simEsperandoM2) {
      vTaskDelay(pdMS_TO_TICKS(1200));
      simEsperandoM2 = false;
      Serial.println("[BT SIM] Recebido: M2 PRONTO");
      publicarEvento(EV_M2_PRONTO);
    }

    if (simTurnoM2) {
      vTaskDelay(pdMS_TO_TICKS(1200));
      simTurnoM2 = false;

      int rx, ry;
      do { // Aleatoriza o tiro do M2 ao M1, verificando que já não atirou antes (se atirou antes = 0)
        rx = random(0, TAM_GRID);
        ry = random(0, TAM_GRID);
      } while (simM2TirosFeitos[rx][ry]);
      simM2TirosFeitos[rx][ry] = true; 

      simTiroM2X = rx;
      simTiroM2Y = ry;
      Serial.printf("[BT SIM] M2 atirou em (%d,%d)\n", rx, ry);
      publicarEvento(EV_OK_BT);
    }

    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void vTaskFSM(void *pvParameters) {
  EventoMsg msg;
  for (;;) {
    if (xQueueReceive(xQueueEventos, &msg, portMAX_DELAY) == pdTRUE) {
      fsm.update(msg); //Loop de atualizar os estados da máquina e efetuar as ações
    }
  }
}

#endif
