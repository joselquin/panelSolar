// PanelSolar - Pruebas de alimentación con panel solar con ESP8266
// mar2021
//
// JLQG(2021)
//
// 


#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <time.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>

#define version "1.01_210320"
#define ssid "RedPiscina"       // Sustituir por el SSID de tu red Wifi
#define password "pwd_wifi"     // Sustituir por la clave de tu red Wifi


const char *mqtt_server = "mqtt_host.com";  // Sustituir por el host de tu servidor MQTT
const char *mqtt_user = "user_mqtt";        // Sustituir por tus credenciales MQTT
const char *mqtt_pass = "password_mqtt";    // Sustituir por tus credenciales MQTT
String clientId = "access_termo_";
const char * panel_topic = "panel";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -3600;
const int   daylightOffset_sec = 3600;

int last_measure = 0;
const int measure_interval = 10000;
const int voltagePin = 34;

WiFiClient espClient;
WiFiClientSecure secureClient;
PubSubClient client(espClient);  // Usa la conexión WiFi para transmisión MQTT

void setup_wifi();
void voltage_measure();
String printLocalTime();
void reconnect();
void callback(char* topic, byte* payload, unsigned int length);
void envio_mqtt(float);

//***** SETUP
void setup() {
    Serial.begin(115200);
    setup_wifi();

    // Init and get local time
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    printLocalTime();

    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);

    delay(200);
}

//***** BUCLE PRINCIPAL
void loop() {

    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    long now = millis();
    if (now - last_measure > measure_interval) {
        last_measure = now;
        voltage_measure();
    }
}

String printLocalTime(){
  struct tm timeinfo;
  char timeStringBuff[50]; //50 chars should be enough
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    String asString(timeStringBuff);
    return timeStringBuff;
  }
  
  strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
  //print like "const char*"
  Serial.println(timeStringBuff);

  return timeStringBuff;
}

//***** MAPEA VALORES CON DECIMALES
float mapa(int x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


//***** CONEXION WIFI
void setup_wifi(){
	delay(10);
	// Nos conectamos a nuestra red Wifi
	Serial.println();
	Serial.print("Conectando a ");
	Serial.println(ssid);

	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println("");
	Serial.println("Conectado a red WiFi!");
	Serial.println("Dirección IP: ");
	Serial.println(WiFi.localIP());
}

//***** LECTURA DEL NIVEL DE TENSION
void voltage_measure() {
    int rawLevel = analogRead(voltagePin);
    float voltageLevel = mapa(rawLevel, 0.0f, 4095.0f, 0.0f, 5.2f);
    if (voltageLevel<4.7 && voltageLevel>0.5) {
        voltageLevel += (5.2 - voltageLevel) * 0.2;       // Recalibración
    } else {
        Serial.print("** ¡Saturación!\n");
    }
    //float voltageLevel = analogRead(A0);
    Serial.print("Tensión: ");
    Serial.println(voltageLevel,2);
    Serial.print("rawLevel: ");
    Serial.println(rawLevel);
    Serial.println("\n");

    envio_mqtt(voltageLevel);
}

//***** ENVIA DATO POR MQTT
void envio_mqtt(float voltage_measure) {
    String fecha = printLocalTime();
    String strValor = String(voltage_measure);
    strValor += ";";
    strValor += fecha;
    Serial.print("Enviando valor a base de datos por MQTT: ");
    Serial.println(strValor);
    Serial.println("\n");
    char chrValor[strValor.length() + 1];
    strcpy(chrValor, strValor.c_str());
    client.publish("panel/valores", chrValor);
}

//***** CONEXION MQTT
void reconnect() {
	while (!client.connected()) {
		Serial.print("Intentando conexión Mqtt...");
		// Creamos un cliente ID
		
		clientId += String(random(0xffff), HEX);
		// Intentamos conectar
		if (client.connect(clientId.c_str(),mqtt_user,mqtt_pass)) {
			Serial.println("Conectado!");
            client.publish(panel_topic, "Panel solar conectado");
			// Nos suscribimos
			client.subscribe(panel_topic);

		} else {
			Serial.print("falló :( con error -> ");
			Serial.println(client.state());
			Serial.println("ClienteId: " + clientId);
			Serial.println(" Intentamos de nuevo en 5 segundos");

			delay(5000);
		}
	}
}

//***** RECEPCION MENSAJES MQTT
void callback(char* topic, byte* payload, unsigned int length){
	String incoming = "";
	Serial.print("Mensaje recibido desde -> ");
	Serial.print(topic);
	Serial.println("");
	for (int i = 0; i < length; i++) {
		incoming += (char)payload[i];
	}
	incoming.trim();
	Serial.println("Mensaje -> " + incoming);
}

