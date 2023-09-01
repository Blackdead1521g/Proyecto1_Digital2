#include "config.h"
#include <iostream>
#include <string>
//#include <Arduino>
using namespace std;
#include <HardwareSerial.h>
#define LED 2
#define RXD2 16
#define TXD2 17

int Foto = 0;
int Infra = 0;
int Ultra = 0;
String mensaje;
String hora; 
bool inputReceivedfoto = false;
bool inputReceivedinfra = false;
bool inputReceivedultra = true;
String foto= "";
String infra = "";
String ultra = "";
String inputString = "";
int posicionf;
int posicioni;
int posicionu;


/*** Example Starts Here ****/

// this int will hold the current count for our sketch
int count = 0;
float luz = 0;
// set up the 'counter' feed
AdafruitIO_Feed *SeguridadBancariaCanal = io.feed("SeguridadBancaria");
AdafruitIO_Feed *SensorUltraCanal = io.feed("SensorUltra");
AdafruitIO_Feed *SensorInfraCanal = io.feed("SensorInfra");

HardwareSerial SerialPort(2);  //UART2

void setup() {
  Serial.begin(9600);  // UART0
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2); //UART2
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
  // start the serial connection
  //Serial.begin(115200);

  // Espera a que termine de comunicar
  while(!Serial);

  Serial.print("Connecting to Adafruit IO");

  // conexion al servidor IO
  io.connect();

  // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  //Conexión exitosa
  Serial.println();
  Serial.println(io.statusText());
}

void loop() {
  // io.run(); is required for all sketches.
  // it should always be present at the top of your loop
  // function. it keeps the client connected to
  // io.adafruit.com, and processes any incoming data.
  //io.run();
  io.run();
  /*Serial.print("sending -> ");
  Serial.println(10);
  SeguridadBancariaCanal->save(10);*/
  digitalWrite(LED, LOW);
  /*if (inputReceived) {
    if (inputString== "3") {
      Serial.println("recibido");
      delay(3000);
    } else if (inputString == "0") {
      Serial.println("infrojo = 1");
      delay(3000);
    }
      
     else {
      Serial.println("Received: " + inputString);
    }
    inputReceived = false;
    inputString = "";
  }

  Serial.println("Received: " + inputString);*/
  foto = "";
  infra = "";
  ultra = "";
  serialEvent();

  if(inputReceivedfoto == true){
    Foto = (foto.toInt());
    Serial.print("Foto -> ");
    Serial.println(Foto);
    //if(Foto > 0 && Foto < 255){
    if(Foto > 0 && Foto < 201){
      SeguridadBancariaCanal->save(Foto);
    }
  }
    //SeguridadBancariaCanal->save(Foto);
    //delay(3000);
  
  if(inputReceivedinfra == true){
    Infra = infra.toInt();
    Serial.print("Infra -> ");
    Serial.println(Infra);
    if(Infra == 0 || Infra == 255){
      SensorInfraCanal->save(Infra);
    }
    //delay(3000);
  }

  if(inputReceivedultra == true){
    Ultra = ultra.toInt();
    Serial.print("Ultra -> ");
    Serial.println(Ultra);
    if(Ultra > 0 && Ultra < 256){
      SensorUltraCanal->save(Ultra);
    }
    //delay(3000);
  }


  /*if(Foto > 0 && Foto < 255){
    SeguridadBancariaCanal->save(Foto);
  }

  if(Ultra > 0 && Ultra < 20){
    SensorUltraCanal->save(Ultra);
  }
  if(Infra == 0 || Infra == 255){
    SensorInfraCanal->save(Infra);
  }*/
  /*Serial.println(Foto);
  Serial.println(Infra);
  Serial.println(Ultra);*/
  /*SeguridadBancariaCanal->save(Foto);
  SensorInfraCanal->save(Infra);
  SensorUltraCanal->save(Ultra);
  //inputReceived = false;*/
  
  delay(1800);
}

/*void serialEvent(){
  if(Serial2.available())
  {
    inputString = Serial2.readStringUntil('\n');
    inputString.trim();
    posicionf = inputString.indexOf ("f");
    posicioni = inputString.indexOf ("i");
    posicionu = inputString.indexOf ("u");
    foto = inputString.substring (0, posicionf);
    infra = inputString.substring (posicionf, posicioni);
    ultra = inputString.substring (posicioni, inputString.length());
    //delay (300);
  }
}*/

void serialEvent(){
  while (Serial2.available())
  {
    char c = (char)Serial2.read();
    if(inputReceivedultra == true){
      if (c != 'f'){
        foto += c;
      }
      else 
      {
        foto.trim();
        inputReceivedinfra = false;
        inputReceivedfoto = true;
        inputReceivedultra = false;
        break;
      }
    }
    if(inputReceivedfoto == true){
      if (c != 'i'){
        infra += c;
      }
      else 
      {
        infra.trim();
        inputReceivedinfra = true;
        inputReceivedfoto = false;
        inputReceivedultra = false;
        break;
      }
    }
    if(inputReceivedinfra == true){
      if (c != 'u'){
        ultra += c;
      }
      else 
      {
        ultra.trim();
        inputReceivedinfra = false;
        inputReceivedfoto = false;
        inputReceivedultra = true;
        break;
      }
    }
  }
}


int stringToInt(String str) {
  int result = 0;
  int sign = 1; // Para manejar el signo del número

  // Verificar si el primer carácter es un signo
  if (str.charAt(0) == '-') {
    sign = -1;
    str = str.substring(1); // Eliminar el signo del string
  }

  // Convertir cada dígito del string a un valor entero
  for (int i = 0; i < str.length(); i++) {
    char digit = str.charAt(i);
    if (isdigit(digit)) {
      result = result * 10 + (digit - '0');
    } else {
      // Caracter inválido en el string
      return 0; // O algún otro valor que indique un error
    }
  }

  return result * sign;
}

