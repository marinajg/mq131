// by Olivier Staquet https://github.com/ostaquet/Arduino-MQ131-driver
// adaptado por Marina Gouvêa

#ifndef _MQ131_H_
#define _MQ131_H_

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

// valores padrão
#define MQ131_RL_default                            1000000           // load resistance 1MOhms padrão
#define MQ131_R0_default                            110470.60         // R0 padrão
#define MQ131_time2read_default                     72                // tempo para amostragem
#define MQ131_window_default                        10                // tempo para cálculo da média

class MQ131Class {
  public:
    // construtor
    MQ131Class(long _RL);
    virtual ~MQ131Class();
  
    // inicializa
    void begin(int _pinPower, int _pinSensor, long _RL, int _pinKey);

    // ciclo com delay sem voltar ao loop principal
    void sample();                

    // lê Rs e a concentração do gás
    float getRs();
        
    // time2read após iniciado o heater
    void setTimeToRead(long sec);
    long getTimeToRead();

    // set define o R0 e o get pergunta
    void setR0(float _valueR0);
    float getR0();

    // iniciar calibração; idealmente em 20°C e 65% umid
    void calibrate();

  private:
    // funções internas para gerenciar o heater
    void startHeater();
    bool isTimeToRead();
    void stopHeater();

    // função para ler Rs
    float readRs();
    float win = MQ131_window_default;

    // detalhes do circuito (pins e RL)
    int pinPower = -1;
    int pinSensor = -1;
    int pinKey = 0;
    long valueRL = -1;
    float valueR0 = -1;

    // timer para acompanhar o pre-heating
    long secLastStart = -1;
    long secToRead = -1;

    // variáveis internas;
    float sum = 0;
    float medRs = 0;

};

// extern MQ131Class MQ131;

#endif // _MQ131_H_
