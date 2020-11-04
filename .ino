// inclusão de bibliotecas
#include <SPI.h>
#include <SD.h> // https://www.arduino.cc/en/reference/SD
#include <Wire.h>
#include "MQ131.h" // adaptado de https://github.com/ostaquet/Arduino-MQ131-driver
#include "RTC.h" // https://github.com/adafruit/RTClib
#include "DHT.h" // https://playground.arduino.cc/Main/DHTLib/

// definição de constantes para a chave dip switch
#define CH1 7
#define CH2 6

// criando variável de tipo objeto de cada classe
MQ131Class MQ131(MQ131_RL_default);
RTC_DS3231 rtc;
dht DHT;

const int chipSelect = 4; // pino de conexão CS do RTC
const int pinoDHT11 = A2; // pino analógico utilizado pelo DHT11
int ch1, ch2; // armazena estado da chave dip switch

// leitura do arquivo
// & passa o endereço da variável para alterá-la depois
bool irparaProximoRegistro(File& Arquivo){
  if (Arquivo){
    while (Arquivo.available() && (Arquivo.read() != '\n'));
    return Arquivo.available();
  }else
    return false;
}

bool saltarColunas(File& Arquivo, int Qtd, char Delimitador = ';'){
  if (Arquivo){
    int Ctd = Qtd;
    while (Arquivo.available() && (Ctd > 0)){
      while (Arquivo.available() && (Arquivo.peek() != Delimitador) && (Arquivo.peek() != '\n')) Arquivo.read();
      if (Arquivo.peek() != '\n'){
        if (Arquivo.read() == Delimitador) Ctd--;
      }else
        Ctd = 0; // vai retornar 'true' se encontrou o fim do registro (independente do número de colunas saltadas)
    }
    return (Arquivo.available() && (Ctd == 0));
  }else
    return false;
}

// constroi um vetor de caracteres char tipo string de tamanho 20 com fim do array em \0
// com o peek procura se é fim de página \n ou caracter especial ou delimitador e então posiciona o cursor na próxima casa com o read
// se for fim de linha não deixa avançar (irparaProximoRegistro faz isso)
// alof converte a cadeia para float
float lerValor(File& Arquivo, char Delimitador = ';'){
  if (Arquivo){
    char sVal[20]; int ii = 0;
    while (Arquivo.available() && (Arquivo.peek() != Delimitador) && (Arquivo.peek() != '\n') && 
          (isDigit(Arquivo.peek()) || (Arquivo.peek() == '.')) && (ii < 19)) sVal[ii++] = Arquivo.read();
    if (ii == 0) sVal[ii++] = '0';
    sVal[ii] = '\0';
    while (Arquivo.available() && (Arquivo.peek() != Delimitador) && (Arquivo.peek() != '\n')) Arquivo.read();
    if (Arquivo.peek() != '\n') Arquivo.read(); /// posiciona no próximo campo
    return atof(sVal);
  }else
    return -1;
}

bool lerCalibragemAnterior(float& r0, long& ttr){
  File calibFile = SD.open("calib.txt", FILE_READ); // abre aquivo
  while (irparaProximoRegistro(calibFile))
    if (saltarColunas(calibFile, 4)) {
      r0 = lerValor(calibFile);
      ttr = long(lerValor(calibFile));
    }
  return ((r0 > 0) && (ttr > 0));
}

void setup(){
  pinMode(CH1,INPUT);
  pinMode(CH2,INPUT);
  Serial.begin(115200);
  // verifica o relógio
  if(! rtc.begin()){
    Serial.println("DS3231 nao encontrado");
    while(1); // loop eterno
  }
  if(rtc.lostPower()){
     Serial.println("DS3231 ficou sem bateria");
     rtc.adjust(DateTime(F(__DATE__),F(__TIME__)));
  }
  delay(100);
  Serial.println("DS3231 ok!");
  
  // verifica o cartão SD
  if(!SD.begin(chipSelect)){
    Serial.println("Cartão SD não encontrado"); // verifique as conexões
    while(1); //não faz mais nada
  }
  Serial.println("Cartão SD ok!");
  Serial.println();
    
  MQ131.begin(2,A0,1000000,6);

  ch1 = digitalRead(CH1);
  
  // se a chave 2 estiver na posição off 
  ch2 = digitalRead(CH2);
  if (ch2<0){
    // seta com valores provenientes da calibração anterior a partir da leitura do último registro no arquivo "calib.txt"
    // -1 atribuído para retornar falso senão tiver nada no arquivo ou der problema
    float MQ131R0 = -1; long MQ131TTR = -1;
    if (lerCalibragemAnterior(MQ131R0, MQ131TTR)){
      MQ131.setR0(MQ131R0);
      MQ131.setTimeToRead(MQ131TTR);
    }else{
      MQ131.setR0(815602.75);
      MQ131.setTimeToRead(58);
    }
    Serial.println(MQ131.getR0());
    Serial.println(MQ131.getTimeToRead());
  }

  // imprimir cabeçalho se não tiver nada escrito no arquivo ainda
  File dataFile = SD.open("data.txt",FILE_WRITE); // abre aquivo
  if(dataFile.size() == 0){
  dataFile.println("Data;Hora;Temperatura (ºC);Umidade (%);Rs (ohms)");
  dataFile.close(); // fecha o arquivo
  }
}

void loop(){
  if (ch2>0){
  Serial.println("Calibrando...");
    
    DateTime now = rtc.now(); // chamar função do relógio
    Serial.print(now.hour(),DEC);
    Serial.print(':');
    Serial.print(now.minute(),DEC);
    Serial.print(':');
    Serial.println(now.second(),DEC);
    
    DHT.read11(pinoDHT11); // lê os bits do sensor DHT
    Serial.print(DHT.temperature);
    Serial.println("ºC");
    Serial.print(DHT.humidity);
    Serial.println('%');
                
    MQ131.calibrate();
  }

  if (ch1>0){
    File dataFile = SD.open("data.txt",FILE_WRITE); // abre aquivo 
    // grava informações no arquivo separadas por ;
    if(dataFile){
       DateTime now = rtc.now(); // chamar função do relógio
       dataFile.print(now.day(),DEC); // imprimir no cartão SD
       dataFile.print('/');
       dataFile.print(now.month(),DEC);
       dataFile.print('/');
       dataFile.print(now.year(),DEC);
       dataFile.print(';');
       dataFile.print(now.hour(),DEC);
       dataFile.print(':');
       dataFile.print(now.minute(),DEC);
       dataFile.print(':');
       dataFile.print(now.second(),DEC);
       dataFile.print(';');
        DHT.read11(pinoDHT11); // lê os bits do sensor DHT
        dataFile.print(DHT.temperature); 
        dataFile.print(';');
        dataFile.print(DHT.humidity);
        dataFile.print(';');
          MQ131.sample(); // faz amostra
          dataFile.println(MQ131.getRs()); // imprime o valor de Rs
    
       dataFile.close(); // fecha o arquivo
    }
  }
  delay(60000); // repete a cada 1min 
}
