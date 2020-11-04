// by Olivier Staquet https://github.com/ostaquet/Arduino-MQ131-driver
// adaptado por Marina Gouvêa

#include "MQ131.h"

// construtor
MQ131Class::MQ131Class(long _RL) {
  valueRL = _RL;
}

// destuidor
MQ131Class::~MQ131Class() {
}

// variáveis de inicialização
void MQ131Class::begin(int _pinPower, int _pinSensor, long _RL, int _pinKey) {
  
    // armazenar informações do circuito
    pinPower = _pinPower;
    pinSensor = _pinSensor;
    valueRL = _RL;
    pinKey = _pinKey;

    // setup padrão
    setR0(MQ131_R0_default);
    setTimeToRead(MQ131_time2read_default);
      
    // setup: pinMode
    pinMode(pinPower, OUTPUT);
    pinMode(pinSensor, INPUT);
    pinMode(pinKey,INPUT);

    // desligar o heater como estado padrão
    digitalWrite(pinPower, LOW);
 }

 // ciclo completo (heater, leitura, stop heater)
 void MQ131Class::sample() {
      int count = 0;
    startHeater();
  // checa se o heater foi inicializado
  if(secLastStart >= 0) {
      while(!isTimeToRead()) {
      delay(1000);
    }
    while(count <= win) {
    float value = readRs();
    Serial.println(value); // conferir se media ok
    sum += value;
    delay(1000);
    count++;
    }
  stopHeater();
  medRs = sum/win;
  }
 }
 
  // inicia o heater
  void MQ131Class::startHeater() {
   digitalWrite(pinPower, HIGH);
   secLastStart = millis()/1000;
  }

  // checa se já está na hora de ler Rs (baseado na calibração)
  bool MQ131Class::isTimeToRead() {
    if(millis()/1000 >= secLastStart + getTimeToRead()) {
    return true;
   }
   return false;
  } 

  // stop heater
  void MQ131Class::stopHeater() {
   digitalWrite(pinPower, LOW);
   secLastStart = -1;
  }

  // pega e seta o parâmetro time2read
  long MQ131Class::getTimeToRead() {
   return secToRead;
  }
  void MQ131Class::setTimeToRead(long sec) {
   secToRead = sec;
  }

  // lê o valor de Rs
  float MQ131Class::readRs() {
   int valueSensor = analogRead(pinSensor);
   // o valor anaógico é de 0 a 1023 e corresponde a 0 a 5V
   float vRL = ((float)valueSensor) / 1024 * 5;
   // calculo final para circulos com voltagem dividida
   float rS = (5.0 / vRL - 1.0) * valueRL;
   return rS;
  }

  // imprime Rs
  float MQ131Class::getRs() {
   if(medRs < 0) {
    return 0.0;
  }
  return medRs;
 }

  // função para obter parâmetros iniciais (R0 e time2read)
  void MQ131Class::calibrate() {
  startHeater();
  File dataFile = SD.open("test.txt",FILE_WRITE);
  int key = digitalRead(pinKey);
  while(key>0) {
    float value = readRs();

    Serial.println(value);
    dataFile.println(value);

      delay(250);
  key = digitalRead(pinKey); // se desligar a chave vai interromper
  }
 stopHeater();
 dataFile.close();
 while(1); // travar aqui
}

  // armazena o valor de R0 (vindo da calibração ou setado)
  void MQ131Class::setR0(float _valueR0) {
    valueR0 = _valueR0;
  }

  // pega o valor de R0 (anote!)
  float MQ131Class::getR0() {
  return valueR0;
  }

// MQ131Class MQ131(MQ131_RL_default);
