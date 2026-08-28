#include <PubSubClient.h>
#include <DHT.h>
#include <ESP8266WiFi.h>

#define DHTPIN D7// Digital pin connected to the DHT sensor

#define DHTTYPE DHT11

const char* ssid = "BACKBURNER";
const char* password = "BandungBondowoso";
const char* mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

DHT dht(DHTPIN, DHTTYPE);

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  // untuk membaca charnya apa messagenya karena di c++ itu dia gabisa berupa string
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("iot/esp32/dht11", "hello world");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" failed connect to mqtt, try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void setup() {
  Serial.begin(115200);
  setup_wifi();
  dht.begin();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;
    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }
    // untuk membaca suhu dan merubahnya dalam bentuk string 
    String suhuString = String(t);
    // untuk membaca kelembapan dan merubahnya dalam bentruk string
    String humidityString = String(h);

      ++value;
      // snprintf (msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
      Serial.print("Publish message: ");
      Serial.println(suhuString);
      Serial.println(humidityString);
      client.publish("iot/esp32/dht11/suhu", suhuString.c_str());
      client.publish("iot/esp32/dht11/hum",humidityString.c_str());
  }

}
