//--------------DEFINE----------------------------------------
#define DHT22_PIN         D7
#define RELE_1_PIN        D1
#define RELE_2_PIN        D2
#define IN_1_PIN          D3
#define IN_2_PIN          D5
#define IN_3_PIN          D6
#define LED_VERDE         D8

//--------------VARIABLES-------------------------------------
/* Periodicas */
const unsigned long MAXUL = 4294967295UL; 
unsigned long LT[10] = {0,0,0,0,0,0,0,0,0,0};
unsigned long MS[10] = {100,200,500,1000,2000,5000,10000,30000,60000,300000};

/* DHT22 */
bool  inestable;
bool  arranque;
float humedad;
float temperatura;
float humedad_filtrada;
float temperatura_filtrada;
int   humedad_parte_entera;
int   temperatura_parte_entera;
int   temperatura_parte_decimal;

/* ThingSpeak */
unsigned long myChannelNumber = XXXXXX;
const char * myWriteAPIKey = "XXXXXXXXXXXXXXXX";
const char * myReadAPIKey = "XXXXXXXXXXXXXXXX";
String    thingtweetAPIKey = "XXXXXXXXXXXXXXXX";
char   thingSpeakAddress[] = "api.thingspeak.com";

long    lastConnectionTime = 0; 
boolean      lastConnected = false;
int          failedCounter = 0;

/* Señales de entrada y de salida */
bool RELE1;
bool RELE2;
bool IN1;
bool IN2;
bool IN3;
bool IN1_last;
bool IN2_last;
bool IN3_last;
bool RELE1_last;
bool RELE2_last;

char buf[12];           /* Auxiliar */

/* Acceso al router */
const char* ssid = "XXXXXXXXXXXXX";
const char* password = "XXXXXXXX";



