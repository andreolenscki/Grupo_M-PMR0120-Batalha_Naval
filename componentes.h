#ifndef COMPONENTES_H
#define COMPONENTES_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>
#include "config.h"

class TBotao {
  private:
    int pino, eventoAssociado, dadoAssociado;
    bool estadoAnterior;
    unsigned long ultimoTempo;

  public:
    TBotao(int pin, int evento, int dado = DIR_NENHUM)
      : pino(pin), eventoAssociado(evento), dadoAssociado(dado), //Ao definir o botão, associa-se ao tipo de variavel pino e etc ao que é colocado
        estadoAnterior(HIGH), ultimoTempo(0) {} //Associa automaticamente à HIGH e tempo = 0; 

    void setup() { pinMode(pino, INPUT_PULLUP); }

    bool update() {
      bool leitura = digitalRead(pino);
      if (leitura == LOW && estadoAnterior == HIGH && (millis() - ultimoTempo > 200)) { //anti-bounce
        ultimoTempo = millis();
        estadoAnterior = leitura;
        return true;
      }
      estadoAnterior = leitura;
      return false;
    }

    int getEvento() { return eventoAssociado; }
    int getDado()   { return dadoAssociado; }
};

class TDisplay {
  private:
    Adafruit_SSD1306 oled;
  public:
    TDisplay() : oled(128, 64, &Wire, -1) {}

    void setup() {
      if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("Falha no OLED SSD1306");
        return;
      }
      oled.clearDisplay();
      oled.setTextSize(1);
      oled.setTextColor(SSD1306_WHITE);
      oled.display();
    }

    void show(String linha1, String linha2 = "") {
      oled.clearDisplay();
      oled.setCursor(0, 10);
      oled.println(linha1);
      if (linha2.length() > 0) {
        oled.setCursor(0, 32);
        oled.println(linha2);
      }
      oled.display();
      Serial.println("[DISPLAY] " + linha1 + " | " + linha2);
    }
};

class TMatrizLED {
  private:
    Adafruit_NeoPixel strip;
  public:
    TMatrizLED() : strip(TAM_GRID * TAM_GRID, PIN_LED_MTX, NEO_GRB + NEO_KHZ800) {}

    void setup() {
      strip.begin();
      strip.setBrightness(60);
      Clear();
    }

    void On(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
      if (x >= 0 && x < TAM_GRID && y >= 0 && y < TAM_GRID) {
        int idx = y * TAM_GRID + x;
        strip.setPixelColor(idx, strip.Color(r, g, b));
      }
    }

    void show() { strip.show(); }

    void Clear() {
      strip.clear();
      strip.show();
    }
};

class TNavio {
  private:
    Ponto pos;
    int tamanho;
    bool horizontal;
  public:
    TNavio() : pos({0, 0}), tamanho(3), horizontal(true) {}

    void Resetposicao(int tam = 3) {
      pos = {0, 0};
      tamanho = tam;
      horizontal = true;
    }

    void update(int dir) {
      if (dir == DIR_CIMA  && pos.y > 0)            pos.y--; //Origem é topo esquerdo, com x para direito e y p baixo
      if (dir == DIR_BAIXO && pos.y < TAM_GRID - 1) pos.y++;
      if (dir == DIR_ESQ   && pos.x > 0)            pos.x--;
      if (dir == DIR_DIR   && pos.x < TAM_GRID - 1) pos.x++;
      if (dir == DIR_GIRA)                          horizontal = !horizontal; // Troca horizontalidade
    }
//funções de retornar informações do barco:
    Ponto coordenada()  { return pos; }
    int getTamanho()    { return tamanho; }
    bool isHorizontal() { return horizontal; }
//Existem 2 verificações, essa no navio para ver se está na grid e outra no tabuleiro que checa sobreposição
    bool verifica() {
      return horizontal ? (pos.x + tamanho <= TAM_GRID) : (pos.y + tamanho <= TAM_GRID);
    }

    int sorteio() {
      randomSeed(esp_random());
      return random(1, 3); // 1 = M1 começa, 2 = M2 começa
    }
};

class TTabuleiro {
  private:
    int grid[TAM_GRID][TAM_GRID];        // Tabuleiro próprio (0=água, 1=navio, 2=atingido)
    int tirosFeitos[TAM_GRID][TAM_GRID]; // Radar de tiros em M2 (0=desc., 1=acerto, 2=água)
    int naviosPosicionados;
    int tirosAcertadosEmM2;
    int acertosDeM2EmM1;

  public:
    void setup() { start(); }

    void start() {
      naviosPosicionados = 0;
      tirosAcertadosEmM2 = 0;
      acertosDeM2EmM1    = 0;
      memset(grid, 0, sizeof(grid));
      memset(tirosFeitos, 0, sizeof(tirosFeitos));
    }
//Não pudemos unir as verificações pois uma precisava da outra.
    bool verificaSobreposicao(TNavio &n) {
      if (!n.verifica()) return false;
      Ponto p = n.coordenada();

      for (int i = 0; i < n.getTamanho(); i++) {
        int nx = n.isHorizontal() ? p.x + i : p.x;
        int ny = n.isHorizontal() ? p.y : p.y + i;
        if (nx < 0 || nx >= TAM_GRID || ny < 0 || ny >= TAM_GRID) return false;
        if (grid[nx][ny] != 0) return false;
      }
      return true;
    }

    bool salvaPosicao(TNavio &n) {
      if (!verificaSobreposicao(n)) return false;

      Ponto p = n.coordenada();
      for (int i = 0; i < n.getTamanho(); i++) {
        int nx = n.isHorizontal() ? p.x + i : p.x;
        int ny = n.isHorizontal() ? p.y : p.y + i;
        grid[nx][ny] = 1;
      }

      naviosPosicionados++;
      return true;
    }

    int getNaviosPosicionados() { return naviosPosicionados; }

    void esconderNavios(TMatrizLED &led) { led.Clear(); }

    // Verde = salvos | Amarelo = cursor válido | Vermelho = borda ou sobreposição
    void desenharPosicionamento(TMatrizLED &led, TNavio &n) {
      led.Clear();
      for (int x = 0; x < TAM_GRID; x++) {
        for (int y = 0; y < TAM_GRID; y++) {
          if (grid[x][y] == 1) led.On(x, y, 0, 200, 0);
        }
      }

      Ponto p = n.coordenada();
      bool valido = verificaSobreposicao(n);
      for (int i = 0; i < n.getTamanho(); i++) {
        int nx = n.isHorizontal() ? p.x + i : p.x;
        int ny = n.isHorizontal() ? p.y : p.y + i;
        if (valido) led.On(nx, ny, 255, 200, 0);
        else        led.On(nx, ny, 255, 0, 0);
      }
      led.show();
    }

    // Vermelho = acerto | Azul = água | Branco = mira
    void desenharBatalha(TMatrizLED &led, TNavio &mira) {
      led.Clear();
      for (int x = 0; x < TAM_GRID; x++) {
        for (int y = 0; y < TAM_GRID; y++) {
          if (tirosFeitos[x][y] == 1)      led.On(x, y, 255, 0, 0);
          else if (tirosFeitos[x][y] == 2) led.On(x, y, 0, 0, 255);
        }
      }
      Ponto p = mira.coordenada();
      led.On(p.x, p.y, 255, 255, 255);
      led.show();
    }
//Verifica se o local de tiro é válido
    bool jaAtirou(Ponto p) {
      return (tirosFeitos[p.x][p.y] != 0);
    }

    bool registrarTiroM1(Ponto p) {
      bool acertou = (simGridM2[p.x][p.y] == 1);
      if (acertou) {
        simGridM2[p.x][p.y] = 2;
        tirosFeitos[p.x][p.y] = 1;
        tirosAcertadosEmM2++;
      } else {
        tirosFeitos[p.x][p.y] = 2;
      }
      return acertou;
    }

    bool receberTiroM2(int x, int y) {
      if (x < 0 || x >= TAM_GRID || y < 0 || y >= TAM_GRID) return false;
      if (grid[x][y] == 1) {
        grid[x][y] = 2;
        acertosDeM2EmM1++;
        return true;
      }
      return false;
    }

    bool fimDeJogo() {
      return (tirosAcertadosEmM2 >= 12 || acertosDeM2EmM1 >= 12);
    }

    bool venceu() {
      return (tirosAcertadosEmM2 >= 12);
    }
};

// Instâncias globais (definidas em componentes.cpp)
extern TBotao BCima;
extern TBotao BDir;
extern TBotao BBaixo;
extern TBotao BEsq;
extern TBotao BGira;
extern TBotao BOK;

extern TDisplay   display;
extern TMatrizLED matrizLED;
extern TNavio     navio;
extern TTabuleiro tabuleiro;

#endif
