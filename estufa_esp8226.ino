//------------------------------------------------------
// Xavi v4 - Estufa Ravelli
//------------------------------------------------------
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

//------------------------------------------------------
// Variables de connexio
//------------------------------------------------------
const char *ssid[] = {"wifi1","wifi2","ERROR"};
const char *password[] = {"passwifi1","passwifi2", "ERROR"};
const int num_ssid = 2;
int ssid_count = 0;
int ssid_conn = 0;
//------------------------------------------------------

//------------------------------------------------------
// Variables de connexio MQTT, el servidor va associat al SSID
//------------------------------------------------------
const char* mqtt_server[] = { "mqtt1","mqtt2","ERROR"};
const char* CanalPub = "casa_estufa";
const char* CanalSub = "casa_estufa_status";
const char* nom_client = "estufa_7ks";
const char* msg_hello = "Estufa 7k v4 Connectada" ;
const char* mqtt_user = "user";
const char* mqtt_pass = "pass";
boolean peticio = false ; // Variable per portar un segon check per arrancar l'estufa
//------------------------------------------------------

//------------------------------------------------------
// Enviament de missatges
//------------------------------------------------------
#define MAX_ARRAY 50
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[MAX_ARRAY];
int value = 0;
int count = 0;
byte incomingData[MAX_ARRAY];

//------------------------------------------------------
// Missatges d'estat
//------------------------------------------------------
const char* miss_estat[] = { "00-Apagada","01-Encesa","02-Espera flama", "03-Flama present","04-Treball","05-??","06-Neteja","07-Espera encesa","08-??","09-??","10-??","11-??" } ;

//------------------------------------------------------
// Configuració Wifi ampliada amb varis SSIDs
//------------------------------------------------------
void setup_wifi() {
  // Posem en mode només STATION
  WiFi.mode(WIFI_STA);
  delay(10);
  // We start by connecting to a WiFi network

  WiFi.begin(ssid[ssid_count], password[ssid_count]);

  // Bucle per connectar-nos a una de les ssid admesos
  while (WiFi.status() != WL_CONNECTED) {
    for ( ssid_count = 0; ssid_count < num_ssid && WiFi.status() != WL_CONNECTED ; ssid_count ++ ) {
      WiFi.begin(ssid[ssid_count], password[ssid_count]);
      ssid_conn = ssid_count;
      for ( count = 0; count < 30 && WiFi.status() != WL_CONNECTED ; count++) {
          delay(500);
      }
      Serial.println ( "provant .." );
      Serial.println ( ssid[ssid_count] );
    }
  }
};

//------------------------------------------------------
// funcio setup
//------------------------------------------------------
void setup() {
  // Configuracio del port serie
  Serial.begin ( 1200, SERIAL_8N2);
  // Configurem Wifi
  setup_wifi();
  // Connectem al MQTT
  Serial.println ( "MQTT" );
  Serial.println ( mqtt_server[ssid_conn]);
  client.setServer(mqtt_server[ssid_conn], 1883);
  while (!client.connected()) {
    // Attempt to connect
    if (client.connect(nom_client,mqtt_user,mqtt_pass)) {
      // Once connected, publish an announcement...
      client.publish(CanalPub,msg_hello);
      // ... and resubscribe
      client.subscribe(CanalSub);
    } else {
      // Failed 
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  client.setCallback(callback);
  
}

//------------------------------------------------------
// Enviament comanda ON
//------------------------------------------------------
void set_on() {
  //byte message[] = {0x80, 0x21, 0x01, 0xA2};
  byte message[] = {0x80, 0x21, 0x07, 0xA8, 0x80, 0x2F, 0x09, 0xB8 };
  Serial.write(message, sizeof(message));
  delay(80);
}

//------------------------------------------------------
// Enviament comanda OFF
//------------------------------------------------------
void set_off() {
  byte message[] = {0x80, 0x21, 0x06, 0xA7};
  Serial.write(message, sizeof(message));
}

//------------------------------------------------------
// Enviament comanda TEMP
//------------------------------------------------------
void set_temp( int val_temp) {
#define MAX_TEMP 21
#define MIN_TEMP 15
  byte message[][5] = { { 0xA0, 0x7D, 0x0F, 0x2C },{0xA0, 0x7D, 0x10, 0x2D },{0xA0, 0x7D, 0x11, 0x2E },{0xA0, 0x7D, 0x12, 0x2F },{0xA0, 0x7D, 0x13, 0x30 },{0xA0, 0x7D, 0x14, 0x31 },{0xA0, 0x7D, 0x15, 0x32 } };
  if ( val_temp >= MIN_TEMP and val_temp <= MAX_TEMP )
  {
    Serial.write(message[val_temp - MIN_TEMP], sizeof(message[MAX_TEMP]));
  }
}

//------------------------------------------------------
// Enviament comanda Potencia
//------------------------------------------------------
void set_potencia ( int val_pot) {
#define MAX_POT 5
#define MIN_POT 1
byte message[][5] = { { 0x00 }, {0xA0, 0x7F, 0x01, 0x20},{0xA0, 0x7F, 0x02, 0x21},{0xA0, 0x7F, 0x03, 0x22},{0xA0, 0x7F, 0x04, 0x23},{0xA0, 0x7F, 0x05, 0x24}};
  if ( val_pot >= MIN_POT and val_pot <= MAX_POT )
  {
    Serial.write(message[val_pot], sizeof(message[MAX_POT]));
  }
}



//------------------------------------------------------
// Consulta estat estufa
//------------------------------------------------------
int cons_status() {
  byte message[] = {0x00, 0x21};
  Serial.write(message, sizeof(message));
    delay(500);
  count=0;
  while ( Serial.available() > 0 && count < MAX_ARRAY ) {
    incomingData[count] = Serial.read();
    count++;
  }
  if ( count > 1 ) return (int)incomingData[1] ;
  return 0;
}

//------------------------------------------------------
// Consulta temperatura
//------------------------------------------------------
int cons_temp() {
  byte message[] = {0x00, 0x01 };
  Serial.write(message, sizeof(message));
  delay(500);
  count=0;
  while ( Serial.available() > 0 && count < MAX_ARRAY ) {
    incomingData[count] = Serial.read();
    count++;
  }
  if ( count > 1 ) return (int)incomingData[1]/2 ;
  return 0;
}

//------------------------------------------------------
// Consulta temperatura Ambient
//------------------------------------------------------
int cons_tempamb() {
  byte message[] = {0x20,0x7D};
  Serial.write(message, sizeof(message));
  delay(500);
  count=0;
  while ( Serial.available() > 0 && count < MAX_ARRAY ) {
    incomingData[count] = Serial.read();
    count++;
  }
  if ( count > 1 ) return (int)incomingData[1];
  return 0;
}

//------------------------------------------------------
// Consulta Potencia
//------------------------------------------------------
int cons_pot() {
  byte message[] = { 0x20,0x7F};
  Serial.write(message, sizeof(message));
  delay(500);
  count=0;
  while ( Serial.available() > 0 && count < MAX_ARRAY ) {
    incomingData[count] = Serial.read();
    count++;
  }
  if ( count > 1 ) return (int)incomingData[1];
  return 0;
}



//------------------------------------------------------
// Funcio ens arriba una peticio, resum de les peticions:
// ON -> Petició d'arrencada
// AC -> confirmació d'arrencada
// OF -> Petició d'apagar
// XT -> modificació de temperatura ambient (XT15 ... XT21)
// XP -> modificació de potencia (XP1, XP2, XP3, XP4 o XP5
// T -> consulta temperatura
// S -> consulta estat
// ? -> consultat estat complet
//------------------------------------------------------
void callback(char* topic, byte* payload, unsigned int length) {
  if ((char)payload[0] == 'O' && (char)payload[1] == 'N'  ) {
    client.publish(CanalPub,"Envia ACK");
    peticio = true ;
  } else {
    if ( (char)payload[0] == 'A' && (char)payload[1] == 'C' ) {
      if ( peticio ) {
        client.publish(CanalPub,"ACK OK - En marxa!");
        set_on();
        peticio = false;
      } else {
        client.publish(CanalPub,"ACK NOK - Primer start"); 
      }
    } else {
    if ( (char)payload[0] == 'O' && (char)payload[1] == 'F' ) {
      client.publish(CanalPub,"OFF Requested");
      peticio = false;
      set_off();
    } else {
    if ( (char)payload[0] == 'X' && (char)payload[1] == 'T' ) {
      int val_temp=0;
      String strTemp;
      strTemp += (char)payload[2];
      strTemp += (char)payload[3];
      val_temp = strTemp.toInt();
      snprintf(msg,MAX_ARRAY, "Temp ambient a %d", val_temp );
      client.publish(CanalPub,msg);
      peticio = false;
      set_temp(val_temp);
    } else {
    if ( (char)payload[0] == 'X' && (char)payload[1] == 'P' ) {
      int val_pot=0;
      String strPot;
      strPot += (char)payload[2];
      val_pot = strPot.toInt();
      snprintf (msg, MAX_ARRAY, "Potencia nova %d", val_pot );
      client.publish(CanalPub,msg);
      peticio = false;
      set_temp(val_pot);
    } else {
    if ( (char)payload[0] == 'T' ) {
      snprintf (msg, MAX_ARRAY, "Temp %d",  cons_temp() );
      peticio = false;
      client.publish(CanalPub, msg);
    } else {
      if ( (char)payload[0] == 'S' ) {
      snprintf (msg, MAX_ARRAY, "Estat %s",  miss_estat[cons_status()] );
      peticio = false;
      client.publish(CanalPub, msg);
    } else {
      if ( (char)payload[0] == '?' ) {
      snprintf (msg, MAX_ARRAY, "Estat %s,Temp %d, Pot %d, Temp Amb. %d",  miss_estat[cons_status()],cons_temp(), cons_pot(), cons_tempamb() );
      peticio = false;
      client.publish(CanalPub, msg);
    } else {
      peticio = false;
      client.publish(CanalPub,"Command not supported: ON, XT, XP, OF, T ,?  o S");
    }
    }
    }
   }
   }
  }
  }
  }
}
//----------------------------------------------
void reconnect() {
  int cont_rec = 0;
  // Loop until we're reconnected
  while (!client.connected()) {
    // Attempt to connect
    if (client.connect(nom_client)) {
      // Once connected, publish an announcement...
      client.publish(CanalPub,msg_hello);
      // ... and resubscribe
      client.subscribe(CanalSub);
    } else {
      // Failed 
      // Wait 10 seconds before retrying
      delay(10000);
    } 
    if ( cont_rec > 50 ) {
      ESP.reset();
    }
    cont_rec++;
  }
}


// Funcio repetitiva ----------------------------
void loop() {
   if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  long now = millis();
  if (now - lastMsg > 12000000) {
    lastMsg = now;
    ++value;
    snprintf (msg, MAX_ARRAY, "Estufa connectada");
    client.publish(CanalPub, msg);
  }
  
}

