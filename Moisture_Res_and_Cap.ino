#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <PubSubClient.h>

#include <WiFi.h>

int REFRESH = 10000;

const char* ssid = "CORK-WIFI";
const char* password = "XXXXX"; // replace with password from privatedetails.txt
const int mqttPort = 1883;
const char* mqttServer = "172.16.9.60";



String sensorTypeC = "CapMoisture";
String sensorTypeR = "ResMoisture";

////////////////////////////////////////////////////////////////////
// Resisitive and Capacitive Moisture Meter for cork measurement  //
// Greg Hirson, Harv 81 Group                                     //
// ghirson@corksupplyusa.com                                      //
////////////////////////////////////////////////////////////////////

String sensorIDR = "4";
String sensorIDC =  "5";
const char* topicR = "office/misc/GregPlantR";
const char* topicC = "office/misc/GregPlantC";


int resPin = 34;
int capPin = 32;

int r = 0;
int c = 0;

int ledPin = 2;
int buttonPin = 0;

int lowR = 4095;
int highR = 1250;

int lowC = 2685;
int highC = 1250;

int cal = 1;

long previousMillis = 0;
int interval = 2000;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {


  pinMode(resPin, INPUT);
  pinMode(capPin, INPUT);

  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);

  // use boot button for calibration

  pinMode(buttonPin, INPUT_PULLUP);

  Serial.println("********************");
  Serial.println("Soil Moisture Sensors");
  Serial.println("Harv 81 Group");
  Serial.println("Greg Hirson");
  Serial.println("********************");

  //try to connect to WIFI
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("[WiFi] Connecting to ");
  Serial.println(ssid);
  Serial.print("Connected at :");
  Serial.println(WiFi.localIP());

  digitalWrite(ledPin, HIGH);
  delay(1000);
  digitalWrite(ledPin, LOW);
  delay(1000);
  digitalWrite(ledPin, HIGH);
  delay(1000);
  digitalWrite(ledPin, LOW);

  delay(3000);

  client.setServer(mqttServer, mqttPort);
}

void loop() {

  // check for calibration on boot button

  // connect to MQTT
  if (!client.connected()) {
    reconnect();
  }

  // connect to WiFi
  long currentMillis = millis();
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis) >= interval) {
    Serial.print(millis());
    Serial.println(" Reconnecting to WiFi...");
    ESP.restart();
    //    WiFi.disconnect();
    //    WiFi.reconnect();
    previousMillis = currentMillis;
  }

  cal = digitalRead(buttonPin);

  if (!cal) {
    digitalWrite(ledPin, HIGH);
    //begin calibration
    calibrate();
    //end calibration
    blink(ledPin, 10, 10);
  }


  //blink(ledPin, 100, 2);

  r = analogRead(resPin);
  c = analogRead(capPin);

  Serial.print("Resistive reading: ");
  Serial.print(r);
  Serial.print(" , Moisture %: ");
  Serial.println(map(r, lowR, highR, 0, 100));

  Serial.print("Capacitive reading: ");
  Serial.print(c);
  Serial.print(" , Moisture %: ");
  Serial.println(map(c, lowC, highC, 0, 100));


  StaticJsonDocument<128> docR;
  char outputR[128];

  docR["sensor"] = sensorTypeR;
  docR["sensorID"] = sensorIDR;
  docR["raw"] = r;
  docR["moisture"] = map(r, lowR, highR, 0, 100);

  serializeJson(docR, outputR);

  Serial.println(outputR);


  StaticJsonDocument<128> docC;
  char outputC[128];

  docC["sensor"] = sensorTypeC;
  docC["sensorID"] = sensorIDC;
  docC["raw"] = c;
  docC["moisture"] = map(c, lowC, highC, 0, 100);

  serializeJson(docC, outputC);

  Serial.println(outputC);

  delay(REFRESH);

}

void calibrate() {
  Serial.println("Calibrating....");

  Serial.println("Place probe(s) in air, make sure they are dry.");
  Serial.println("Waiting 20 seconds...");
  delay(5000);
  Serial.println("Waiting 15 seconds...");
  delay(5000);
  Serial.println("Waiting 10 seconds...");
  delay(5000);
  Serial.println("Waiting 5 seconds...");
  delay(5000);
  Serial.print("Measuring air with resistive sensor...");
  lowR = analogRead(resPin);
  Serial.println(lowR);
  Serial.print("Measuring air with capacitive sensor...");
  lowC = analogRead(capPin);
  Serial.println(lowC);
  Serial.println("Dry calibration complete");

  delay(1000);

  Serial.println("Begin wet calibration");
  Serial.println("Place probe(s) in water, make sure they are submerged to top of sensor.");
  Serial.println("Waiting 20 seconds...");
  delay(5000);
  Serial.println("Waiting 15 seconds...");
  delay(5000);
  Serial.println("Waiting 10 seconds...");
  delay(5000);
  Serial.println("Waiting 5 seconds...");
  delay(5000);
  Serial.print("Measuring water with resistive sensor...");
  highR = analogRead(resPin);
  Serial.println(highR);
  Serial.print("Measuring waterr with capacitive sensor...");
  highC = analogRead(capPin);
  Serial.println(highC);
  Serial.println("Wet calibration complete.");
  Serial.println("Calibration complete");
}

void blink(int pin, int dy, int times) {

  for (int i = 0; i < times; i++) {
    digitalWrite(pin, HIGH);
    delay(dy);
    digitalWrite(pin, LOW);
    delay(dy);
  }

}

void reconnect() {
  // Loop until reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create Client
    String clientID = "ESP32Client-";
    clientID += String(random(0xffff), HEX);;

    if (client.connect(clientID.c_str())) {
      Serial.println("connected");
      digitalWrite(ledPin, HIGH);
    } else {
      Serial.print("failed, rc=");
      Serial.println(client.state());
      digitalWrite(ledPin, LOW);
      delay(2000);
    }
  }
}
