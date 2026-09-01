#include <PubSubClient.h>
#include <DHT.h>
#include <ESP8266WiFi.h>

#define DHTPIN D7// Digital pin connected to the DHT sensor
#define PINRELAY D5// Digital pin untuk mengontrol relay

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
  String pesan = "";
  String topikMasuk = String(topic);
  // untuk membaca charnya apa messagenya karena di c++ itu dia gabisa berupa string
  for (int i = 0; i < length; i++) {
    pesan += (char)payload[i];
  }
  Serial.print("pesan masuk: " + pesan);
  Serial.println();
  
  // if else jika on relay high dan sebaliknya
  if (pesan == "ON"){
    digitalWrite(PINRELAY, HIGH);
  }
  else if(pesan == "OFF"){
    digitalWrite(PINRELAY, LOW);
  }
  // mendapatkan pesan dari request semisal di dapati pin d5 sedang high maka dikirimlah on ke mqtt dan sebaliknya
  if (strcmp(topic, "iot/esp32/relay/status") == 0){
    if(pesan == "REQUEST"){
      int kondisiSekarang = digitalRead(PINRELAY);
      if(kondisiSekarang == HIGH){
        client.publish("iot/esp32/relay/status", "ON");
      }
      else{
        client.publish("iot/esp32/relay/status", "OFF");
      }
    }
  }

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
      // subscribe ke relay untuk mendapat info dari javascript klo relay harus on
      client.subscribe("iot/esp32/relay");
      // tambahan subscribe ke iot esp32 status
      client.subscribe("iot/esp32/relay/status");
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
  pinMode(PINRELAY, OUTPUT);
  digitalWrite(PINRELAY, LOW);
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

    // membuat String untuk format JSON jadi agar nanti 1 Topic saja

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }
    // untuk membaca suhu dan merubahnya dalam bentuk string 
    String suhuString = String(t);
    // untuk membaca kelembapan dan merubahnya dalam bentruk string
    String humidityString = String(h);

    String pesanJson = "{\"suhu\":" + String(t) + ",\"hum\":" + String(h) +"}";

      ++value;
      // snprintf (msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
      Serial.println("Publish message: ");
      Serial.println("suhu: " + suhuString);
      Serial.println("Humidity: " + humidityString);

      client.publish("iot/esp32/dht11/data", pesanJson.c_str());
      // ada update menggunakan 1 client.publish saja (menghemat topic)
      // client.publish("iot/esp32/dht11/suhu", suhuString.c_str());
      // client.publish("iot/esp32/dht11/hum",humidityString.c_str());
  }

}
