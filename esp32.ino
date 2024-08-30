#include <DHT.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>

// Define the GPIO pin where the DHT11 data pin is connected
#define DHTPIN 4

// Define the type of DHT sensor
#define DHTTYPE DHT11

// Create an instance of the DHT class
DHT dht(DHTPIN, DHTTYPE);

// Wi-Fi credentials
const char* ssid = "sanjay";
const char* password = "sanju1234";

// Create a web server on port 80
WebServer server(80);

// Create a WebSocket server on port 81
WebSocketsServer webSocket = WebSocketsServer(81);

unsigned long lastSendTime = 0;
const unsigned long interval = 100; // 2 seconds interval

void setup() {
  Serial.begin(115200);
  dht.begin();
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to Wi-Fi");
  Serial.println(WiFi.localIP());

  // Define routes
  server.on("/", HTTP_GET, []() {
    String html = "<!DOCTYPE html><html><head><title>Temperature & Humidity</title>";
    html += "<script>";
    html += "var socket = new WebSocket('ws://' + window.location.hostname + ':81');";
    html += "socket.onmessage = function(event) {";
    html += "  var data = JSON.parse(event.data);";
    html += "  document.getElementById('humidity').textContent = data.humidity + ' %';";
    html += "  document.getElementById('temperature').textContent = data.temperature + ' °C';";
    html += "};";
    html += "socket.onopen = function(event) {";
    html += "  console.log('WebSocket is open now.');";
    html += "};";
    html += "socket.onclose = function(event) {";
    html += "  console.log('WebSocket is closed now.');";
    html += "};";
    html += "</script>";
    html += "</head><body>";
    html += "<h1>Temperature & Humidity</h1>";
    html += "<p>Humidity: <span id='humidity'>Loading...</span></p>";
    html += "<p>Temperature: <span id='temperature'>Loading...</span></p>";
    html += "</body></html>";
    server.send(200, "text/html", html);
  });

  // Start the server
  server.begin();
  
  // Start WebSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  server.handleClient();
  webSocket.loop();

  unsigned long currentMillis = millis();
  if (currentMillis - lastSendTime >= interval) {
    lastSendTime = currentMillis;

    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    if (!isnan(humidity) && !isnan(temperature)) {
      String message = "{\"humidity\":" + String(humidity) + ",\"temperature\":" + String(temperature) + "}";
      webSocket.broadcastTXT(message);
    }
  }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  // Handle WebSocket events here if needed
}
