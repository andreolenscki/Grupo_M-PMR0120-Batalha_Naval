#ifndef FSM_H
#define FSM_H
#include "config.h"
#include "componentes.h"

class TMaquinaEstados {
  private:
    int estadoAtual;
    int matrizProximo[NUM_ESTADOS][NUM_EVENTOS];
    int matrizAcao[NUM_ESTADOS][NUM_EVENTOS];

    void executarAcao(int acao, int dado) {
      switch (acao) {
        case A00: // E0_IDLE -> E1_PAREAMENTO_BT
          display.show("Pareamento BT...", "Buscando M2...");
          simBuscarM2 = true;
          break;

        case A01: // E1_PAREAMENTO_BT -> E2_NAVEGACAO
          display.show("M2 Conectada!", "Posicione navio 1/6");
          tabuleiro.start();
          simIniciarM2();
          navio.Resetposicao(TAMANHOS_NAVIOS[0]);
          tabuleiro.desenharPosicionamento(matrizLED, navio);
          break;

        case A02: // E2_NAVEGACAO -> E2_NAVEGACAO
          navio.update(dado);
          tabuleiro.desenharPosicionamento(matrizLED, navio);
          break;

        case A07: // E2_NAVEGACAO -> E3_VERIFICACAO
          if (tabuleiro.verificaSobreposicao(navio)) publicarEvento(EV_POSICAO_LEGAL);
          else                                       publicarEvento(EV_POSICAO_ILEGAL);
          break;

        case A08: // E3_VERIFICACAO -> E4_CONFIRMA_POSICIONAMENTO
          if (tabuleiro.salvaPosicao(navio)) {
            int qtd = tabuleiro.getNaviosPosicionados();
            display.show("Navio " + String(qtd) + "/6 salvo!");
            if (qtd < 6) {
              publicarEvento(EV_MENOS_6_NAVIOS);
            } else {
              display.show("6 Navios prontos!", "Aguardando M2...");
              simEsperandoM2 = true;
              publicarEvento(EV_SEIS_NAVIOS);
            }
          } else {
            publicarEvento(EV_POSICAO_ILEGAL);
          }
          break;

        case A09: { // E3_VERIFICACAO -> E2_NAVEGACAO ou E4_CONFIRMA_POSICIONAMENTO -> E2_NAVEGACAO
          int idx = tabuleiro.getNaviosPosicionados();
          navio.Resetposicao(TAMANHOS_NAVIOS[idx]);
          tabuleiro.desenharPosicionamento(matrizLED, navio);
          display.show("Posicione navio:", String(idx + 1) + "/6 (Tam: " + String(TAMANHOS_NAVIOS[idx]) + ")");
          break;
        }

        case A10: { // E5_ESPERA_POSICIONAMENTO_M2 -> E6_SORTEIO (Sorteio + Esconder navios)
          int quemComeca = navio.sorteio();
          tabuleiro.esconderNavios(matrizLED);
          navio.Resetposicao(1); // Mira de 1 casa
          if (quemComeca == 1) {
            display.show("Sorteio: M1!", "Sua vez de atirar");
            tabuleiro.desenharBatalha(matrizLED, navio);
            publicarEvento(EV_SORTEIO_M1);
          } else {
            display.show("Sorteio: M2!", "Aguardando tiro M2");
            simTurnoM2 = true;
            publicarEvento(EV_SORTEIO_M2);
          }
          break;
        }

        case A12: // E7_TURNO_M1 -> E7_TURNO_M1 (Move mira)
          navio.update(dado);
          tabuleiro.desenharBatalha(matrizLED, navio);
          break;

        case A16: { // E7_TURNO_M1 -> E9_CONFIRMA_FIM_JOGO (Atira em M2)
          Ponto alvo = navio.coordenada();
          if (tabuleiro.jaAtirou(alvo)) {
            display.show("Ja atirou aqui!", "Escolha outra casa");
            vTaskDelay(pdMS_TO_TICKS(700));
            publicarEvento(EV_JOGO_CONTINUA_M1);
            break;
          }

          bool acertou = tabuleiro.registrarTiroM1(alvo);
          tabuleiro.desenharBatalha(matrizLED, navio);
          display.show(acertou ? "FOGO! Acertou M2!" : "AGUA! Errou tiro.");
          vTaskDelay(pdMS_TO_TICKS(1000));

          if (tabuleiro.fimDeJogo()) publicarEvento(EV_FIM_DE_JOGO);
          else                       publicarEvento(EV_JOGO_CONTINUA_M2);
          break;
        }

        case A17: // E9_CONFIRMA_FIM_JOGO -> E0_IDLE
          simBuscarM2    = false;
          simEsperandoM2 = false;
          simTurnoM2     = false;
          display.show(tabuleiro.venceu() ? "VITORIA DE M1!" : "M2 VENCEU!", "Aperte OK p/ jogar");
          matrizLED.Clear();
          break;

        case A18: // E9_CONFIRMA_FIM_JOGO -> E8_TURNO_M2
          display.show("Turno M2", "M2 escolhendo tiro");
          simTurnoM2 = true;
          break;

        case A_TIMEOUT_BT: // E1_PAREAMENTO_BT/E5_ESPERA_POSICIONAMENTO_M2/E7_TURNO_M1/E8_TURNO_M2 -> E0_IDLE (Falha ou timeout no BT)
          simBuscarM2    = false;
          simEsperandoM2 = false;
          simTurnoM2     = false;
          matrizLED.Clear();
          display.show("Falha/Timeout BT!", "Aperte OK p/ tentar");
          break;

        case A20: { // E8_TURNO_M2 -> E9_CONFIRMA_FIM_JOGO (Recebe tiro de M2)
          bool m2Acertou = tabuleiro.receberTiroM2(simTiroM2X, simTiroM2Y);
          display.show(m2Acertou ? "M2 ACERTOU!" : "M2 deu AGUA!",
                       String("Tiro em (") + simTiroM2X + "," + simTiroM2Y + ")");
          vTaskDelay(pdMS_TO_TICKS(900));

          if (tabuleiro.fimDeJogo()) publicarEvento(EV_FIM_DE_JOGO);
          else                       publicarEvento(EV_JOGO_CONTINUA_M1);
          break;
        }

        case A21: // E9_CONFIRMA_FIM_JOGO -> E7_TURNO_M1
          display.show("Sua vez (M1)!", "Mova mira e OK");
          tabuleiro.desenharBatalha(matrizLED, navio);
          break;
      }
    }

  public:
    void setup() {
      estadoAtual = E0_IDLE;
      for (int i = 0; i < NUM_ESTADOS; i++) {
        for (int j = 0; j < NUM_EVENTOS; j++) {
          matrizAcao[i][j] = NENHUMA_ACAO;
          matrizProximo[i][j] = i;
        }
      }

      matrizProximo[E0_IDLE][EV_OK] = E1_PAREAMENTO_BT;
      matrizAcao[E0_IDLE][EV_OK]    = A00;

      matrizProximo[E1_PAREAMENTO_BT][EV_TIMEOUT_BT_PAREAMENTO] = E0_IDLE;
      matrizAcao[E1_PAREAMENTO_BT][EV_TIMEOUT_BT_PAREAMENTO]    = A_TIMEOUT_BT;
      matrizProximo[E1_PAREAMENTO_BT][EV_ENCONTRA_DISPOSITIVO]  = E2_NAVEGACAO;
      matrizAcao[E1_PAREAMENTO_BT][EV_ENCONTRA_DISPOSITIVO]     = A01;

      matrizProximo[E2_NAVEGACAO][EV_BOTOES_MOVIMENTO] = E2_NAVEGACAO;
      matrizAcao[E2_NAVEGACAO][EV_BOTOES_MOVIMENTO]    = A02;
      matrizProximo[E2_NAVEGACAO][EV_OK]               = E3_VERIFICACAO;
      matrizAcao[E2_NAVEGACAO][EV_OK]                  = A07;

      matrizProximo[E3_VERIFICACAO][EV_POSICAO_ILEGAL] = E2_NAVEGACAO;
      matrizAcao[E3_VERIFICACAO][EV_POSICAO_ILEGAL]    = A09;
      matrizProximo[E3_VERIFICACAO][EV_POSICAO_LEGAL]  = E4_CONFIRMA_POSICIONAMENTO;
      matrizAcao[E3_VERIFICACAO][EV_POSICAO_LEGAL]     = A08;

      matrizProximo[E4_CONFIRMA_POSICIONAMENTO][EV_MENOS_6_NAVIOS] = E2_NAVEGACAO;
      matrizAcao[E4_CONFIRMA_POSICIONAMENTO][EV_MENOS_6_NAVIOS]    = A09;
      matrizProximo[E4_CONFIRMA_POSICIONAMENTO][EV_POSICAO_ILEGAL] = E2_NAVEGACAO;
      matrizAcao[E4_CONFIRMA_POSICIONAMENTO][EV_POSICAO_ILEGAL]    = A09;
      matrizProximo[E4_CONFIRMA_POSICIONAMENTO][EV_SEIS_NAVIOS]    = E5_ESPERA_POSICIONAMENTO_M2;

      matrizProximo[E5_ESPERA_POSICIONAMENTO_M2][EV_TIMEOUT_BT_POSICIONAMENTO] = E0_IDLE;
      matrizAcao[E5_ESPERA_POSICIONAMENTO_M2][EV_TIMEOUT_BT_POSICIONAMENTO]    = A_TIMEOUT_BT;
      matrizProximo[E5_ESPERA_POSICIONAMENTO_M2][EV_M2_PRONTO]                 = E6_SORTEIO;
      matrizAcao[E5_ESPERA_POSICIONAMENTO_M2][EV_M2_PRONTO]                    = A10;

      matrizProximo[E6_SORTEIO][EV_SORTEIO_M1] = E7_TURNO_M1;
      matrizProximo[E6_SORTEIO][EV_SORTEIO_M2] = E8_TURNO_M2;

      matrizProximo[E7_TURNO_M1][EV_BOTOES_MOVIMENTO] = E7_TURNO_M1;
      matrizAcao[E7_TURNO_M1][EV_BOTOES_MOVIMENTO]    = A12;
      matrizProximo[E7_TURNO_M1][EV_OK]               = E9_CONFIRMA_FIM_JOGO;
      matrizAcao[E7_TURNO_M1][EV_OK]                  = A16;
      matrizProximo[E7_TURNO_M1][EV_TIMEOUT_BT_TURNO] = E0_IDLE;
      matrizAcao[E7_TURNO_M1][EV_TIMEOUT_BT_TURNO]    = A_TIMEOUT_BT;

      matrizProximo[E8_TURNO_M2][EV_OK_BT]            = E9_CONFIRMA_FIM_JOGO;
      matrizAcao[E8_TURNO_M2][EV_OK_BT]               = A20;
      matrizProximo[E8_TURNO_M2][EV_TIMEOUT_BT_TURNO] = E0_IDLE;
      matrizAcao[E8_TURNO_M2][EV_TIMEOUT_BT_TURNO]    = A_TIMEOUT_BT;

      matrizProximo[E9_CONFIRMA_FIM_JOGO][EV_FIM_DE_JOGO]      = E0_IDLE;
      matrizAcao[E9_CONFIRMA_FIM_JOGO][EV_FIM_DE_JOGO]         = A17;
      matrizProximo[E9_CONFIRMA_FIM_JOGO][EV_JOGO_CONTINUA_M2] = E8_TURNO_M2;
      matrizAcao[E9_CONFIRMA_FIM_JOGO][EV_JOGO_CONTINUA_M2]    = A18;
      matrizProximo[E9_CONFIRMA_FIM_JOGO][EV_JOGO_CONTINUA_M1] = E7_TURNO_M1;
      matrizAcao[E9_CONFIRMA_FIM_JOGO][EV_JOGO_CONTINUA_M1]    = A21;
    }

    void update(EventoMsg msg) {
      if (msg.tipo < 0 || msg.tipo >= NUM_EVENTOS) return;

      int acao = matrizAcao[estadoAtual][msg.tipo];
      int prox = matrizProximo[estadoAtual][msg.tipo];

      Serial.printf("[FSM] E%d --(EV %d)--> E%d | Acao: %d\n", estadoAtual, msg.tipo, prox, acao);
      estadoAtual = prox;

      executarAcao(acao, msg.dado);
    }
};

TMaquinaEstados fsm;
#endif
