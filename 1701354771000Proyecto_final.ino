#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <PubSubClient.h>

int s_analogica_mq135=34;
float sensorValue;
float voltage;
float ppm;
int LEDred = 4;
int LEDgreen = 5;
int LEDyellow = 18;
int boton = 2;
int espera = 1;
String estado = " ";
int giro = 0;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display (SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

char ssid[] = "Tec-Contingencia";
char pass[] = "NULL";

const char* server = "mqtt3.thingspeak.com";

//#define USESECUREMQTT
#ifdef USESECUREMQTT
  #include <WiFiClientSecure.h>
  #define mqttPort 8883
  WiFiClientSecure client; 
#else
  #include <WiFi.h>
  #define mqttPort 1883
  WiFiClient client;
#endif

const char mqttUserName[]   = "AyoFOTI4DwoVLwQ4HiYaDzc"; 
const char clientID[]       = "AyoFOTI4DwoVLwQ4HiYaDzc";
const char mqttPass[]       = "GnrnG1/HCNWQRtjjoD4FpW86";

#define channelID 2334677

const char * PROGMEM thingspeak_ca_cert = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDxTCCAq2gAwIBAgIQAqxcJmoLQJuPC3nyrkYldzANBgkqhkiG9w0BAQUFADBs\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n" \
"d3cuZGlnaWNlcnQuY29tMSswKQYDVQQDEyJEaWdpQ2VydCBIaWdoIEFzc3VyYW5j\n" \
"ZSBFViBSb290IENBMB4XDTA2MTExMDAwMDAwMFoXDTMxMTExMDAwMDAwMFowbDEL\n" \
"MAkGA1UEBhMCVVMxFTATBgNVBAoTDERpZ2lDZXJ0IEluYzEZMBcGA1UECxMQd3d3\n" \
"LmRpZ2ljZXJ0LmNvbTErMCkGA1UEAxMiRGlnaUNlcnQgSGlnaCBBc3N1cmFuY2Ug\n" \
"RVYgUm9vdCBDQTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMbM5XPm\n" \
"+9S75S0tMqbf5YE/yc0lSbZxKsPVlDRnogocsF9ppkCxxLeyj9CYpKlBWTrT3JTW\n" \
"PNt0OKRKzE0lgvdKpVMSOO7zSW1xkX5jtqumX8OkhPhPYlG++MXs2ziS4wblCJEM\n" \
"xChBVfvLWokVfnHoNb9Ncgk9vjo4UFt3MRuNs8ckRZqnrG0AFFoEt7oT61EKmEFB\n" \
"Ik5lYYeBQVCmeVyJ3hlKV9Uu5l0cUyx+mM0aBhakaHPQNAQTXKFx01p8VdteZOE3\n" \
"hzBWBOURtCmAEvF5OYiiAhF8J2a3iLd48soKqDirCmTCv2ZdlYTBoSUeh10aUAsg\n" \
"EsxBu24LUTi4S8sCAwEAAaNjMGEwDgYDVR0PAQH/BAQDAgGGMA8GA1UdEwEB/wQF\n" \
"MAMBAf8wHQYDVR0OBBYEFLE+w2kD+L9HAdSYJhoIAu9jZCvDMB8GA1UdIwQYMBaA\n" \
"FLE+w2kD+L9HAdSYJhoIAu9jZCvDMA0GCSqGSIb3DQEBBQUAA4IBAQAcGgaX3Nec\n" \
"nzyIZgYIVyHbIUf4KmeqvxgydkAQV8GK83rZEWWONfqe/EW1ntlMMUu4kehDLI6z\n" \
"eM7b41N5cdblIZQB2lWHmiRk9opmzN6cN82oNLFpmyPInngiK3BD41VHMWEZ71jF\n" \
"hS9OMPagMRYjyOfiZRYzy78aG6A9+MpeizGLYAiJLQwGXFK3xPkKmNEVX58Svnw2\n" \
"Yzi9RKR/5CYrCsSXaQ3pjOLAEFe4yHYSkVXySGnYvCoCWw9E1CAx2/S6cCZdkGCe\n" \
"vEsXCS+0yx5DaMkHJ8HSXPfqIbloEpw8nL+e/IBcm2PN7EeqJSdnoDfzAIJ9VNep\n" \
"+OkuE6N36B9K\n" \
"-----END CERTIFICATE-----\n";

int status = WL_IDLE_STATUS; 

PubSubClient mqttClient( client );

int connectionDelay    = 4;    // Delay (s) between trials to connect to WiFi
long lastPublishMillis = 0;    // To hold the value of last call of the millis() function
int updateInterval     = 5;   // Sensor readings are published every 15 seconds o so.
int tiempo_espera      = 15;    // Freeze time for button.

void initDisplay();
void showInDisplay(float ppm);
void connectWifi();
void mqttConnect();
void mqttSubscribe( long subChannelID );
void mqttSubscriptionCallback( char* topic, byte* payload, unsigned int length );
void mqttPublish(long pubChannelID, String message);

void setup() {
  Serial.begin( 115200 );
  delay(3000);     
  initDisplay();
  connectWifi();
  mqttClient.setServer( server, mqttPort ); 
  mqttClient.setCallback( mqttSubscriptionCallback );
  mqttClient.setBufferSize( 2048 );

  #ifdef USESECUREMQTT
    client.setCACert(thingspeak_ca_cert);
  #endif

  pinMode(LEDred, OUTPUT);
  pinMode(LEDgreen, OUTPUT);
  pinMode(LEDyellow, OUTPUT);
  pinMode(boton, INPUT);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWifi();
  }
  
  if (!mqttClient.connected()) {
    mqttConnect(); 
    mqttSubscribe(channelID);
  }

  mqttClient.loop(); 
  
  if(((millis()-lastPublishMillis)>updateInterval*1000) || ((digitalRead(boton) == HIGH) && (espera == 1))){
    updateInterval = 60;
    espera = 2;
    giro += 1;
    digitalWrite (LEDyellow, LOW);
    sensorValue = analogRead(s_analogica_mq135);
    voltage = (sensorValue / 1024.0) * 3.3;
    ppm = (voltage) / 0.0089;
    
    Serial.print("Ciclo: ");
    Serial.println(giro);
    Serial.print("Valor del Sensor: ");
    Serial.println(sensorValue);
    Serial.print("Concentración de CO2 (PPM): ");
    Serial.println(ppm);  

        // Aire
    if(ppm >= 300 && ppm <= 550){
      estado = "Normal";
      digitalWrite (LEDgreen, HIGH);
      digitalWrite (LEDred, LOW);
    }
    // Dioxido de carbono
    else if(ppm > 550){
      estado = "Alta";
      digitalWrite (LEDred, HIGH);
      digitalWrite (LEDgreen, LOW);
    }
    else{
      estado = "Baja";
      digitalWrite (LEDred, HIGH);
      digitalWrite (LEDgreen, LOW);
    }

    showInDisplay(ppm);
    Serial.print("Estado: Concentración ");
    Serial.println(estado);
    
    //mqttPublish( channelID, String("field1=")+String(ppm));
    //mqttPublish( channelID, String("field2=")+String(ppm));
    //mqttPublish( channelID, String("field3=")+String(ppm));
    mqttPublish( channelID, String("field4=")+String(ppm));
    
    lastPublishMillis = millis();

    delay(tiempo_espera*1000);
    espera = 1;
    digitalWrite (LEDyellow, HIGH);
  }
}

void initDisplay()
{
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();
  display.setTextColor(WHITE);
}

void showInDisplay(float ppm)
{
  // Clear display
  display.clearDisplay();
  // display temperature
  display.setTextSize(1);
  display.setCursor(0,0);
  display.print("Concentracion CO2: ");
  display.setTextSize(2);
  display.setCursor(0,10);
  display.print(ppm);
  display.print(" ");
  display.setTextSize(1);
  display.cp437(true);
  //display.write(167);
  display.setTextSize(2);
  display.print("ppm"); 
  display.setTextSize(0.3);
  display.setCursor(0,40);
  display.print("Concentracion: ");
  display.print(estado);
  display.print("\nCiclo: ");
  display.print(giro);
  display.display(); 
}

// Function to connect to WiFi.
void connectWifi()
{
  Serial.println( "Connecting to Wi-Fi..." );
  // Loop until WiFi connection is successful
    while ( WiFi.status() != WL_CONNECTED ) {
    //WiFi.begin( ssid, pass );
    WiFi.begin(ssid);
    delay( connectionDelay*1000 );
    Serial.println( WiFi.status() ); 
  }
  Serial.println( "Connected to Wi-Fi." );
}

// Function to connect to MQTT server.
void mqttConnect() {
  // Loop until the client is connected to the server.
  while ( !mqttClient.connected() )
  {
    // Connect to the MQTT broker.
    if ( mqttClient.connect( clientID, mqttUserName, mqttPass ) ) {
      Serial.print( "MQTT to " );
      Serial.print( server );
      Serial.print (" at port ");
      Serial.print( mqttPort );
      Serial.println( " successful." );
    } else {
      Serial.print( "MQTT connection failed, rc = " );
      // See https://pubsubclient.knolleary.net/api.html#state for the failure code explanation.
      Serial.print( mqttClient.state() );
      Serial.println( " Will try the connection again in a few seconds" );
      delay( connectionDelay*1000 );
    }
  }
}

// Function to subscribe to ThingSpeak channel for updates.
void mqttSubscribe( long subChannelID ){
  String myTopic = "channels/"+String( subChannelID )+"/subscribe";
  mqttClient.subscribe(myTopic.c_str());
}

// Function to handle messages from MQTT subscription to the ThingSpeak broker.
void mqttSubscriptionCallback( char* topic, byte* payload, unsigned int length ) {
  // Print the message details that was received to the serial monitor.
  Serial.print("\nMessage arrived from ThinksSpeak broker [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println("\n");
}

// Function to publish messages to a ThingSpeak channel.
void mqttPublish(long pubChannelID, String message) {
  String topicString ="channels/" + String( pubChannelID ) + "/publish";
  mqttClient.publish( topicString.c_str(), message.c_str() );
}