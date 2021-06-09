/*
  Protótipo Pomodoro

  Arquivo: pomodorov01.ino
  Versão: 0.1
  Autor:
    Georg Augusto Schlegel <georgaugusto@gmail.com>
*/

// -------- Bibliotecas --------
#include <Arduino.h>
#include <Wire.h>
#include <TimerOne.h>

// -------- Configurações do projeto --------
#define pinBotaoStart 4  // Porta do Arduino em que o botao Start esta ligado (o segundo terminal do botao deve ser ligado em GND)

#define tempoReset 2000     // Define o tempo em que o botao Start deve ficar apertado para efetuar o reset (em milisegundos)
#define tempoContador 1500  // Define o tempo do contador (em segundos)
#define tempoDescanso 300  // Define o tempo do contador (em segundos)
#define tempoDescansoMaior 1200  // Define o tempo do contador (em segundos)

// -------- Configurações do LCD --------
#include <LiquidCrystal_I2C.h>  // Carrega a biblioteca LiquidCrystal

LiquidCrystal_I2C lcd(0x27, 20, 4);  // Define o endereço do LCD para 0x27 e display para 20 colunas e 4 linhas

// -------- Funções --------
byte pinBotaoStartApertado();
void showContador();
void contaTempo();

// -------- Variaveis publicas --------
const int amarelo = 8;
const int verde = 9;
const int vermelho = 10;

int tempo = tempoContador;
int contador = 0;
byte contadorStatus = 0;  // 0=Pause, 1=Start

int cicloContador = 0;
int cicloTela = 0;

void setup() {
  Serial.begin(9600);
   pinMode(amarelo, OUTPUT);
   pinMode(verde, OUTPUT);
   pinMode(vermelho, OUTPUT);
  pinMode(pinBotaoStart, INPUT_PULLUP);

  lcd.begin();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.begin();
  // Inicio do timer
  Timer1.initialize(1000000);  // O timer ira executar a cada 1 segundo (parâmetro em microssegundos)
  Timer1.attachInterrupt(contaTempo);
}

void loop() {
  byte estadoBotaoStart = pinBotaoStartApertado();
  if (estadoBotaoStart == 1) {
    if (tempo > 0) {
      if (contadorStatus == 0) {
        if (contador == 0) {
          contador = tempo;
        }
        // Inicia o contador novamente
        digitalWrite(verde, LOW);
        digitalWrite(amarelo, LOW);
        digitalWrite(vermelho, HIGH);
        Timer1.start();
        Timer1.attachInterrupt(contaTempo);
        contadorStatus = 1;
      } else {
        // Para o contador
        digitalWrite(verde, LOW);
        digitalWrite(amarelo, HIGH);
        digitalWrite(vermelho, LOW);
        Timer1.stop();
        contadorStatus = 0;
      }
    }
  }
  if ( estadoBotaoStart == 2 ) {
    digitalWrite(verde, LOW);
    digitalWrite(amarelo, HIGH);
    digitalWrite(vermelho, LOW);
    if (tempo > 0) {
      Timer1.stop();
      contadorStatus = 0;  
      contador = 0;
      cicloTela = 0;
      cicloContador = 0;
    }
  }
  showContador();
}

byte pinBotaoStartApertado() {
#define tempoDebounce 50  // (Tempo para eliminar o efeito Bounce EM MILISEGUNDOS)

  static bool estadoBotaoAnt;
  static unsigned long delayBotao = 0;
  static unsigned long botaoApertado;
  static byte fase = 0;

  bool estadoBotao;
  byte estadoRet;

  estadoRet = 0;
  if ((millis() - delayBotao) > tempoDebounce) {
    estadoBotao = digitalRead(pinBotaoStart);
    if (!estadoBotao && (estadoBotao != estadoBotaoAnt)) {
      delayBotao = millis();
      botaoApertado = millis();
      fase = 1;
    }
    if ((fase == 1) && ((millis() - botaoApertado) > tempoReset)) {
      fase = 0;
      estadoRet = 2;
    }
    if (estadoBotao && (estadoBotao != estadoBotaoAnt)) {
      delayBotao = millis();
      if (fase == 1) {
        estadoRet = 1;
      }
      fase = 0;
    }
    estadoBotaoAnt = estadoBotao;
  }
  return estadoRet;
}

void showContador() {
  static int contadorAnt = contador + 1;
  static byte digitosAnt[4] = { 99, 99, 99, 99 };

  int segundo;
  int minuto;
  byte digitos[4];

  if (contador != contadorAnt) {
    segundo = contador % 60;
    minuto = contador / 60;

    digitos[0] = minuto / 10;
    digitos[1] = minuto % 10;
    digitos[2] = segundo / 10;
    digitos[3] = segundo % 10;

    for (int nL = 0; nL < 4; nL++) {
      if (digitosAnt[nL] != digitos[nL]) {
        lcd.setCursor(6, 0); // Mostra od dados no LCD
        lcd.print("Pomodoro");
        lcd.setCursor(2, 2);
        lcd.print(digitos[0]);
        lcd.setCursor(3, 2);
        lcd.print(digitos[1]);
        lcd.setCursor(5, 2);
        lcd.print("mins");
        lcd.setCursor(10, 2);
        lcd.print(digitos[2]);
        lcd.setCursor(11, 2);
        lcd.print(digitos[3]);
        lcd.setCursor(13, 2);
        lcd.print("secs");
        lcd.setCursor(19, 3);
        lcd.print(cicloTela);
      }
      digitosAnt[nL] = digitos[nL];
    }
  }
  contadorAnt = contador;
}

void contaTempo() {
  if (contadorStatus == 1) {
    contador--;
    if (contador < 0) {
      digitalWrite(verde, LOW);
      digitalWrite(amarelo, LOW);
      digitalWrite(vermelho, HIGH);
      if (cicloContador % 2 == 0) {
        digitalWrite(verde, HIGH);
        digitalWrite(amarelo, LOW);
        digitalWrite(vermelho, LOW);
        contador = tempoDescanso;
        if (cicloContador % 8 == 0 && cicloContador != 0) {
          contador = tempoDescansoMaior;
        }
      } else {
        contador = tempoContador;
        cicloTela++;
      }
      cicloContador++;
    }
  }
}
