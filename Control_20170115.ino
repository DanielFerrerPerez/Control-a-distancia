/*
  Inicio: 2016/12/18  Daniel Ferrer
  Fin: 2017/01/15
  
  --- PROYECTO CONTROL 2 DO y 3 DI ---
  Se trata de realizar un control a distancia a través de thingspeak de 2 relés y visualizar y registrar 3 entradas digitales.
  También se permite registrar el estado de la temperatura y la humedad del sistema.
  El sistema estará conectado via WIFI a un router Livebox 2.1 Astoria que a su vez, a través de uno de sus puertos USB se ha
  colocado un adaptador USB-3G (pincho) que permite la conexión a internet mediante una tarjeta prepago de Simyo con una tarifa de datos
  de 100MB al mes, con un consumo económico de 1 euro mensual.
  Este sistema está ajustado para que el consumo de datos de bajada sea pequeño (consumo real en torno a 1,5MB por día).
  Esto provoca que las ordenes que damos a los actuadores (relés) en el peor de los casos, tengan un retardo de 5 minutos.
  
  Componentes:
  - 1 WEMOS D1 mini https://www.wemos.cc/product/d1-mini-pro.html
  - 2 Relés https://www.wemos.cc/product/relay-shield.html
  - 1 Sensor de temperatura y humedad DHT22 http://www.ebay.es/itm/DHT22-Digital-Temperature-Humidity-Sensor-Module-for-Arduino-DHT-22-raspberry-pi-/322222315164?hash=item4b05f2569c&_uhb=1
  - 1 Resistencia de 10kOhm (en caso de tener el DHT22 sin la placa shield de wemos)
  - 1 Condensador de 10nF   (en caso de tener el DHT22 sin la placa shield de wemos)
  - 1 Led verde de 5 mm de diámetro.
  - 1 Resistencia de 390 Ohm. (para el led).
  - 1 mt de cable rigido de un hilo.
  - 17 fichas de empalme de 2,5mm.
  
  Circuito:
    D0 - Libre.
    D1 - Salida Relé C1
    D2 - Salida Relé C2
    D3 - Entrada I1
    D4 - Led placa WEMOS 
    D5 - Entrada I2
    D6 - Entrada I3
    D7 - Entrada sensor de temperatura DHT22
    D8 - Salida piloto verde

  Funciones:
   - Mandar a la nube -> Thingspeak, los datos de temperatura, humedad, estado de relés y entradas digitales cada 5 minutos.                         OK.
   - Enviar a través de Thingspeak -> Twitter inmediatamente un mensaje con la información del estado de las entradas y salidas en caso de cambio    OK.

  Programas Android recomendados:
  https://play.google.com/store/apps/details?id=com.cinetica_tech.thingview&hl=es
  https://play.google.com/store/apps/details?id=ua.livi.thingspeakmonitor&hl=es

  Gestión de activación y desactivación de los relés:
  http://api.thingspeak.com/update?key=XXXXXXXXXXXXX&fieldY=1 Activa relé1
  http://api.thingspeak.com/update?key=XXXXXXXXXXXXX&fieldY=0 Desactiva relé1
  http://api.thingspeak.com/update?key=XXXXXXXXXXXXX&fieldZ=1 Activa relé2
  http://api.thingspeak.com/update?key=XXXXXXXXXXXXX&fieldZ=0 Desactiva relé2

  Siendo XXXXXXXXXXXX la Key de escritura.
  Siendo Y el campo del relé1. (en este caso es 6).
  Siendo Z el campo del relé2. (en este caso es 7).
  
 */

/*----------------- LIBRERIAS ----------------*/
#include <dht.h>                  /*    DHT22 temperatura y humedad       */
#include <ESP8266WiFi.h>          /*    Internet                          */
#include <WiFiClient.h>           /*    Modo cliente                      */
#include <ThingSpeak.h>           /*    Datos en la nube                  */
#include <EEPROM.h>               /*    Guardar y recuperar datos         */
#include "variables.h"            /*    variables                         */
#include "funciones.h"            /*    funciones                         */

/*----------------- INSTANCIAS ----------------*/
WiFiClient  client;                   /* Wifi cliente                 */
dht DHT;                              /* Sensor temperatura y humedad */

/*----------------- ARRANQUE ------------------*/
void setup() {
  inestable = HIGH;         /* Cuando arranca el Wemos, activa esta marca para evitar trabajar con valores no estables. Pasado 120 segundos de arrancar, pasa a LOW */
  arranque = HIGH;          /* Esta marca se pone a 0 pasados 5,5 minutos después de tener tensión. Esto permite que el router con el pincho 3G se estabilicen si se va la tensión */
  Serial.begin(9600);       /* Configuramos el puerto serie */
  WiFi.mode(WIFI_STA);      /* Establecemos el ESP8266 como estación para acceder a un AP */
  WiFi.begin(ssid, password);  /* Nombre del router y password ver pestaña variables */
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi conectado OK");
  Serial.println ( "" );
  Serial.print ( "Conectado a: " );
  Serial.println (WiFi.SSID());
  Serial.println(WiFi.localIP());
  
  ThingSpeak.begin(client); /* Arrancamos el cliente para Thinkspeak */

// SALIDAS DIGITALES
  pinMode(RELE_1_PIN, OUTPUT);
  pinMode(RELE_2_PIN, OUTPUT);
  pinMode(LED_VERDE,  OUTPUT);
  
// ENTRADAS DIGITALES
  pinMode(IN_1_PIN , INPUT_PULLUP);
  pinMode(IN_2_PIN , INPUT_PULLUP);
  pinMode(IN_3_PIN , INPUT_PULLUP);

}

void updateTwitterStatus(String tsData) {
  if (client.connect(thingSpeakAddress, 80))
  { 
    // Create HTTP POST Data
    tsData = "api_key="+thingtweetAPIKey+"&status="+tsData;
    client.print("POST /apps/thingtweet/1/statuses/update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(tsData.length());
    client.print("\n\n");
    client.print(tsData);
    lastConnectionTime = millis();
    if (client.connected()) {
      Serial.println("Connecting to ThingSpeak...");
      Serial.println();
      failedCounter = 0;
    } else {
      failedCounter++;
      Serial.println("Connection to ThingSpeak failed ("+String(failedCounter, DEC)+")");   
      Serial.println();
    }
  } else {
    failedCounter++;
    Serial.println("Connection to ThingSpeak Failed ("+String(failedCounter, DEC)+")");   
    Serial.println();
    lastConnectionTime = millis(); 
  }
}


void OB35() { /*100ms*/

}

void OB36() { /*200ms*/
 
}

void OB37() { /*500ms*/

}

void OB38() { /*1s*/
  String ciclo;             /* Texto que identifica el ciclo de arranque */
  String mensaje;           /* Mensaje a enviar a través de Twitter */
  byte arranqueEEPROM;      /* Arranque número de ciclo (0-255) */
  if (millis() >= 330000 && arranque == HIGH) {
    EEPROM.begin(512);
    arranqueEEPROM = EEPROM.read(0);    /* Arranque número de ciclo (0-255) */
    ciclo=itoa(arranqueEEPROM,buf,10);  /* En texto ponemos el nº de arranque que llevamos */
    mensaje = "Arranque nº" + ciclo;
    EEPROM.write(0,byte(arranqueEEPROM + 1));
    EEPROM.commit();
    updateTwitterStatus(mensaje);       /* Informamos que se ha arrancado el termostato */
    arranque = LOW;
  }
}

void OB39() { /*2s*/

}

void OB40() { /*5s*/
 
}

void OB41() { /*10s*/
  int chk = DHT.read22(DHT22_PIN);  
  filtro(DHT.temperature,DHT.humidity); 
  Serial.print("T=");
  Serial.println(temperatura_filtrada);
  Serial.print("H=");
  Serial.println(humedad_filtrada);
}

void OB42() { /*30s*/

}

void OB43() { /*1m*/

}

void OB44() { /*5m*/
  float estado_RELE1 = ThingSpeak.readFloatField(myChannelNumber, 6 ,myReadAPIKey);  /* Rele 1 */
  float estado_RELE2 = ThingSpeak.readFloatField(myChannelNumber, 7 ,myReadAPIKey);  /* Rele 2 */
  if (estado_RELE1 > 0.5) {digitalWrite(RELE_1_PIN,HIGH); RELE1 = HIGH; }
  if (estado_RELE1 < 0.5) {digitalWrite(RELE_1_PIN,LOW); RELE1 = LOW; }  
  if (estado_RELE2 > 0.5) {digitalWrite(RELE_2_PIN,HIGH); RELE2 = HIGH; }
  if (estado_RELE2 < 0.5) {digitalWrite(RELE_2_PIN,LOW); RELE2 = LOW; }
  delay(1000);
  if(digitalRead(IN_1_PIN) == LOW)  { IN1 = HIGH; }
  if(digitalRead(IN_1_PIN) == HIGH) { IN1 = LOW; }
  if(digitalRead(IN_2_PIN) == LOW)  { IN2 = HIGH; }
  if(digitalRead(IN_2_PIN) == HIGH) { IN2 = LOW; }
  if(digitalRead(IN_3_PIN) == LOW)  { IN3 = HIGH; }
  if(digitalRead(IN_3_PIN) == HIGH) { IN3 = LOW; }
  if(inestable == LOW) {  /* Cada 5 minutos, enviamos a thingspeak los valores filtrados */
    Serial.println("SUBIENDO DATOS A THINGSPEAK");
    ThingSpeak.setField(1,temperatura_filtrada);            /* Dato de la temperatura actual a enviar a thingspeak */
    ThingSpeak.setField(2,humedad_filtrada);                /* Dato de la humedad actual a enviar a thingspeak */
    ThingSpeak.setField(3,IN1);                             /* Estado entrada IN1 */
    ThingSpeak.setField(4,IN2);                             /* Estado entrada IN2 */
    ThingSpeak.setField(5,IN3);                             /* Estado entrada IN3 */
    ThingSpeak.setField(6,RELE1);                           /* Estado del relé 1 */
    ThingSpeak.setField(7,RELE2);                           /* Estado del relé 2 */
    ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey); /* Enviamos los datos */
    Serial.println("DATOS A THINGSPEAK SUBIDOS cada 5 min");
  }
}

void periodicas() {
  unsigned long T = millis();
  if (LT[0]+MS[0] <= T)   {if (LT[0] < MAXUL - MS[0])   {LT[0] = T;} else {LT[0] = MAXUL - T;} OB35();} /*100ms*/
  if (LT[1]+MS[1] <= T)   {if (LT[1] < MAXUL - MS[1])   {LT[1] = T;} else {LT[1] = MAXUL - T;} OB36();} /*200ms*/
  if (LT[2]+MS[2] <= T)   {if (LT[2] < MAXUL - MS[2])   {LT[2] = T;} else {LT[2] = MAXUL - T;} OB37();} /*500ms*/
  if (LT[3]+MS[3] <= T)   {if (LT[3] < MAXUL - MS[3])   {LT[3] = T;} else {LT[3] = MAXUL - T;} OB38();} /*1s   */
  if (LT[4]+MS[4] <= T)   {if (LT[4] < MAXUL - MS[4])   {LT[4] = T;} else {LT[4] = MAXUL - T;} OB39();} /*2s   */
  if (LT[5]+MS[5] <= T)   {if (LT[5] < MAXUL - MS[5])   {LT[5] = T;} else {LT[5] = MAXUL - T;} OB40();} /*5s   */
  if (LT[6]+MS[6] <= T)   {if (LT[6] < MAXUL - MS[6])   {LT[6] = T;} else {LT[6] = MAXUL - T;} OB41();} /*10s  */
  if (LT[7]+MS[7] <= T)   {if (LT[7] < MAXUL - MS[7])   {LT[7] = T;} else {LT[7] = MAXUL - T;} OB42();} /*30s  */
  if (LT[8]+MS[8] <= T)   {if (LT[8] < MAXUL - MS[8])   {LT[8] = T;} else {LT[8] = MAXUL - T;} OB43();} /*1m   */
  if (LT[9]+MS[9] <= T)   {if (LT[9] < MAXUL - MS[9])   {LT[9] = T;} else {LT[9] = MAXUL - T;} OB44();} /*5m   */
}

void estabilidad() {
  if (millis() >= 120000 && inestable == HIGH) { /* Esperamos 2 min después del arranque a que se cargue el buffer del filtro y tengamos buenos valores medios */
    inestable = LOW;
  }
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {digitalWrite(LED_VERDE,HIGH);} else {digitalWrite(LED_VERDE,LOW);}  /* Piloto verde si hay conexión wifi */
  if(digitalRead(IN_1_PIN) == LOW)  { IN1 = HIGH; }  /* Entradas conectadas a masa -> circuito cerrado -> nos ahorramos resistencia Pullup */
  if(digitalRead(IN_1_PIN) == HIGH) { IN1 = LOW; }   /* Entradas conectadas a masa -> circuito cerrado -> nos ahorramos resistencia Pullup */
  if(digitalRead(IN_2_PIN) == LOW)  { IN2 = HIGH; }  /* Entradas conectadas a masa -> circuito cerrado -> nos ahorramos resistencia Pullup */
  if(digitalRead(IN_2_PIN) == HIGH) { IN2 = LOW; }   /* Entradas conectadas a masa -> circuito cerrado -> nos ahorramos resistencia Pullup */
  if(digitalRead(IN_3_PIN) == LOW)  { IN3 = HIGH; }  /* Entradas conectadas a masa -> circuito cerrado -> nos ahorramos resistencia Pullup */
  if(digitalRead(IN_3_PIN) == HIGH) { IN3 = LOW; }   /* Entradas conectadas a masa -> circuito cerrado -> nos ahorramos resistencia Pullup */

  if(IN1_last!=IN1 || IN2_last!=IN2 || IN3_last!=IN3 || RELE1_last!=RELE1 || RELE2_last!=RELE2 ){  /*SI hay cambio envia Twitter */
    byte cambio;                /* Arranque número de ciclo (0-255) */
    String ciclo;               /* Texto que identifica el ciclo de arranque */
    String mensaje;             /* Mensaje a enviar a través de Twitter */
    cambio = EEPROM.read(1);    /* Arranque número de ciclo (0-255) */
    ciclo=itoa(cambio,buf,10);  /* En texto ponemos el nº de arranque que llevamos */
    EEPROM.write(1,byte(cambio + 1));
    EEPROM.commit();

    mensaje = "Entradas: IN1=";
    if (IN1 == HIGH) {
      mensaje += " 1"; 
    }
    if (IN1 == LOW) {
      mensaje += " 0"; 
    }
    mensaje += " IN2=";
    if (IN2 == HIGH) {
      mensaje += " 1"; 
    }
    if (IN2 == LOW) {
      mensaje += " 0"; 
    }
    mensaje += " IN3=";
    if (IN3 == HIGH) {
      mensaje += " 1"; 
    }
    if (IN3 == LOW) {
      mensaje += " 0"; 
    }
   
    
    mensaje += " Salidas: RELE1=";
    if (RELE1 == HIGH) {
      mensaje += " 1"; 
    }
    if (RELE1 == LOW) {
      mensaje += " 0"; 
    }
    mensaje += " RELE2=";
    if (RELE2 == HIGH) {
      mensaje += " 1"; 
    }
    if (RELE2 == LOW) {
      mensaje += " 0"; 
    }
    
    mensaje += " Cambio nº: " + ciclo;
    
    updateTwitterStatus(mensaje);                                     
    IN1_last = IN1;
    IN2_last = IN2;
    IN3_last = IN3;
    RELE1_last = RELE1;
    RELE2_last = RELE2;
  }
 
  periodicas();
  estabilidad();
}

