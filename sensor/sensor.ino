#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <SSLClient.h>

#include <PubSubClient.h>

#define DEEP_SLEEP false
#define TIME_TO_SLEEP 1

#define ENCRYPTED false

#define SerialMon Serial
#define SerialSensor Serial1
#define SerialAT  Serial2

const char WIFI_SSID[] = "IoTLab";
const char WIFI_PASSWORD[] = "";
//const char WIFI_PASSWORD[] = "I0tN3w0rk!";

const char MQTT_SERVER[] = "192.168.200.1";

const char CLIENT_ID[] = "1";
const char MQTT_USERNAME[] = "";
const char MQTT_PASSWORD[] = "";

#if ENCRYPTED
SSLClient secure_layer(&tcpClient);
#endif

#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */

#define MAX_WAIT_FOR_CONNECTION 20 /* This is 20 * 500ms or 10 seconds */

// MQTT port used to communicate with the server, 1883 is the default unencrypted MQTT port,
// whereas 8883 would be the default encrypted SSL MQTT port
#if ENCRYPTED
int MQTT_PORT = 8883;
#else
int MQTT_PORT = 1883;
#endif

constexpr uint32_t SERIAL_DEBUG_BAUD = 115200U;

constexpr char CONNECTING_MSG[] = "Connecting to: (%s) with token (%s)\n";
constexpr char TEMPERATURE_KEY[] = "temperature";

#if ENCRYPTED
WiFiClientSecure espClient;
#else
WiFiClient espClient;
#endif

// Initalize the Mqtt client instance
PubSubClient mqttClient;

void log(String info) {
    SerialMon.println(info);
}

void go_to_sleep(){
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    log("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +
    " Seconds");
    log("Going to sleep now");
    SerialMon.flush(); 
    esp_deep_sleep_start();
}

/* ------------- WiFi Networking ------------------*/
/// @brief Initalizes WiFi connection,
// will put the device to sleep if connection fails more than MAX_WAIT_FOR_CONNECTION
void InitWiFi() {
  int i = 0;
  log(F("Connecting to AP ..."));
  // Attempting to establish a connection to the given WiFi network
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    // Delay 500ms until a connection has been successfully established
    delay(500);
    log(F("."));
    if(i++ > MAX_WAIT_FOR_CONNECTION){
      go_to_sleep();
    }
  }
  log(F("Connected to AP"));
#if ENCRYPTED
  espClient.setCACert(ROOT_CERT);
#endif
}

/// @brief Reconnects the WiFi uses InitWiFi if the connection has been removed
/// @return Returns true as soon as a connection has been established again
bool reconnect() {
  // Check to ensure we aren't connected yet
  const wl_status_t status = WiFi.status();
  if (status == WL_CONNECTED) {
    return true;
  }

  // If we aren't establish a new connection to the given WiFi network
  InitWiFi();
  return true;
}

void setup() {
  SerialMon.begin(115200);
  // If analog input pin 0 is unconnected, random analog
  // noise will cause the call to randomSeed() to generate
  // different seed numbers each time the sketch runs.
  // randomSeed() will then shuffle the random function.
  randomSeed(analogRead(0));
  delay(1000);

  InitWiFi();
  mqttClient.setClient(espClient);
}

void mqtt_publish_reading(time_t time_stamp, const char *topic_key, uint32_t value){
  String topic = "iotlab/sensor/";
  topic.concat(topic_key);
  String payload = "{";
  payload.concat("\"id\": ");
  payload.concat(CLIENT_ID);
  payload.concat(",\"value\": ");
  payload.concat(value);
  payload.concat("}");
  log("Publish: " + payload);
  mqttClient.publish(topic.c_str(), payload.c_str());
}

void loop() {
  unsigned long start_time = millis();
  time_t time_stamp = 0; 


  if (!reconnect()) {
    return;
  }

  if(!mqttClient.connected()){
    mqttClient.setBufferSize(512);
    mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
    mqttClient.setKeepAlive(60);
    

    if ((MQTT_USERNAME == "") || (MQTT_PASSWORD == ""))
    {
      if (!mqttClient.connect(CLIENT_ID)) {
        log(F("Failed to connect"));
        return;
      }
    }
    else {
      if (!mqttClient.connect(CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD)) {
        log(F("Failed to connect"));
        return;
      }
    }
    log(F("MQTT Connected"));
  }

  mqtt_publish_reading(time_stamp, TEMPERATURE_KEY, random(30,100));

  // //report signal
  // int8_t rssi = WiFi.RSSI();
  // log(F("STA RSSI: "));
  // log(rssi);
  // log(F("dBm"));

  while(!mqttClient.loop()){
    delay(500);
  }
  
  mqttClient.disconnect();
  
  if (DEEP_SLEEP){
    go_to_sleep();
  } else {
    delay(TIME_TO_SLEEP * 1000);
  }
}
