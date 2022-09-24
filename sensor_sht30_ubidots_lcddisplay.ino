#include <LiquidCrystal_I2C.h>//Library LCD i2c
//inisialisasi nomor pin LCD
LiquidCrystal_I2C lcd(0x27, 20, 4);
#include <analogWrite.h>

#include <WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include "Wire.h"
#include "SHT31.h"

String tempStr;
String humStr;

WiFiClient ubidots;
PubSubClient client(ubidots);

#define WIFISSID "KMTek" // Nama Hotspot / WiFi
#define PASSWORD "KMTek123" // Password WiFi
#define TOKEN "BBFF-eI1EqsfXanOul1dORfcIWmE412Z2fQ"   // Ubidots TOKEN
#define MQTT_CLIENT_NAME "SENSOR SHT30" //mqtt client name / Device name/ custom

#define VARIABLE_LABEL_1 "temperature" // Nama Variabel 1
#define VARIABLE_LABEL_2 "humidity" // Nama Variabel 2
#define DEVICE_LABEL "sensor-sht30" // Api Label / Device Label in Ubidots


char mqttBroker[]  = "industrial.api.ubidots.com";
char payload[200];
char topic[150];

char str_val_1[6];
char str_val_2[6];

int Con = 100;

uint32_t start;
uint32_t stop;

SHT31 sht;


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");

    // Attemp to connect
    if (client.connect(MQTT_CLIENT_NAME, TOKEN, "")) {
      Serial.println("Connected To Ubidots");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      // Wait 2 seconds before retrying
      delay(2000);
    }
  }
}
void setup()
{

  analogWrite(20, Con);
  lcd.begin( );
  WiFi.begin(WIFISSID, PASSWORD);
  Serial.begin(115200);
  Wire.begin();

  //connecting to wifi
  Serial.println();
  Serial.print("Wait for WiFi...");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  client.setServer(mqttBroker, 1883);
  client.setCallback(callback);

  sht.begin(0x44);    //Sensor I2C Address

  Wire.setClock(100000);
  uint16_t stat = sht.readStatus();
  Serial.print(stat, HEX);
  Serial.println();
}

void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  sht.read();

  float temperature = (sht.getTemperature());
  float humidity = (sht.getHumidity());

  Serial.print("Temperature:");
  Serial.print(sht.getTemperature(), 1);
  Serial.print("\t");
  Serial.print("Humidity:");
  Serial.println(sht.getHumidity(), 1);
  delay(1000 );

  lcd.setCursor(1, 0);
  lcd.print("Temperature: ");
  lcd.setCursor(3, 1);
  lcd.print(sht.getTemperature());
  lcd.print("^Celsius");
  lcd.setCursor(1, 2);
  lcd.print("Humidity:");
  lcd.setCursor(3, 3);
  lcd.print(sht.getHumidity());
  lcd.print("%");

  dtostrf(temperature, 4, 2, str_val_1);
  dtostrf(humidity, 4, 2, str_val_2);


  sprintf(topic, "%s", "");
  sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL);

  sprintf(payload, "%s", ""); // Cleans the payload content
  sprintf(payload, "{\"%s\":", VARIABLE_LABEL_1);
  sprintf(payload, "%s {\"value\": %s}", payload, str_val_1);
  sprintf(payload, "%s, \"%s\":", payload, VARIABLE_LABEL_2);
  sprintf(payload, "%s {\"value\": %s}", payload, str_val_2);
  sprintf(payload, "%s}", payload);

  Serial.println("Publishing data to Ubidots Cloud");
  client.publish(topic, payload);
  client.loop();
  delay(1000);


}
