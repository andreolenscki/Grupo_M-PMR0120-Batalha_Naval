#include "config.h"
#include "componentes.h"
#include "fsm.h"
#include "tasks.h"

void setup() {
  //Inicializa o display, tabuleiro, a máquina de estados e a matriz.
  Serial.begin(115200);
  display.setup();
  matrizLED.setup();
  tabuleiro.setup();
  fsm.setup();

  display.show("Batalha Naval M1", "Aperte OK (Verde)");

  xQueueEventos = xQueueCreate(20, sizeof(EventoMsg)); //Cria fila 

  if (xQueueEventos != NULL) {
    xTaskCreatePinnedToCore(vTaskBluetoothSim, "BTSimTask", 2048, NULL, 1, &xHandleBluetooth, 0);
    xTaskCreatePinnedToCore(vTaskInputHandler, "InputTask", 2048, NULL, 1, NULL,              1);
    xTaskCreatePinnedToCore(vTaskFSM,          "FSMTask",   4096, NULL, 2, NULL,              1);
  }
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}
