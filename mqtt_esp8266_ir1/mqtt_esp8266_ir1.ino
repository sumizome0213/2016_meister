/*
 Basic ESP8266 MQTT example

 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.

 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off

 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.

 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"

*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <IRremoteESP8266.h>

// Update these with values suitable for your network.

//const char* ssid = "Buffalo-G-8A04";
//const char* password = "6rwt8p7sn4pmv";
//const char* ssid = "spc24";
//const char* password = "mokemoke";
const char* ssid = "WHR-HP-G";
const char* password = "";
const char* mqtt_server = "192.168.11.30";

WiFiClient espClient;
PubSubClient client(espClient);

char outTopic[50] = "ir/ir1_out"; //出力するTopic名
char inTopic[50] = "ir/ir1_in"; //待機するTopi名
char statusTopic[50] = "ir/ir1_status"; //ステータスを出力するTopic名

long lastMsg = 0;
char msg[50];
float val = 0;
char strVal[10];

const char* Filename = "/IRs";

IRsend irsend(5);

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

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
  Serial.print(inTopic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  int num = int((char)payload[1]) + int((char)payload[2])*10;

  rsend.sendNEC(num,32);

  /*// 受付トピックの動作
  if ((char)payload[0] == '1') {
    irsend.sendNEC(0x1D7C03F,32); //ir送信
    //digitalWrite(BUILTIN_LED, LOW); //LED on
    delay(3000);
    
  }*/
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
      client.publish(statusTopic, "reconnected");
      // ... and resubscribe
      client.subscribe(inTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}



void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  irsend.begin();
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  //電池電圧送信
  long now = millis();
  if (now - lastMsg > 60000) { //一分間に一度
    lastMsg = now;
    val = analogRead(A0) * 6 /1024.0; //6vを3.3Vに分圧した値を正規の値に戻す
    sprintf(msg, "VCC = %sV", dtostrf(val, 4, 2, strVal));
    Serial.println(msg);
    sprintf(msg, "%s", dtostrf(val, 4, 2, strVal));
    client.publish(statusTopic, msg);
  }

}
