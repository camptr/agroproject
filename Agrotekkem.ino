#include <WiFi.h>
#include <ThingSpeak.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>
#include <elapsedMillis.h>
#include <WebServer.h>

#define DHTPIN 4         // Pin sensor DHT
#define DHTTYPE DHT21    // Tipe sensor DHT21
#define SOIL_PIN 36      // Pin sensor soil moisture
#define RELAY1_PIN 12    // Pin relay 1
#define RELAY2_PIN 25    // Pin relay 2
#define RELAY3_PIN 1     // Pin relay 3

// WiFi credentials
const char* ssid = "UGMURO-IoT";
const char* password = "Esteh5000";

// ThingSpeak settings
const unsigned long channelID = 3018516;
const char* writeAPIKey = "ND1Z0ZMDEIPSVPSW";

// Interval timing
unsigned long thingSpeakInterval = 15000;
unsigned long sensorInterval = 500;
unsigned long displayInterval = 1000;
unsigned long webserverInterval = 2000;

// Sensor variables
float temperature = 0;
float humidity = 0;
int soilPercentage = 0;

// Objects initialization
DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 20, 4);
elapsedMillis thingSpeakMillis;
elapsedMillis sensorMillis;
elapsedMillis displayMillis;
WebServer server(80);
WiFiClient client;

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  
  // Initialize LCD
  lcd.begin(20, 4);
  lcd.backlight();
  showWelcomeMessage();
  
  // Initialize sensors
  pinMode(SOIL_PIN, INPUT);
  dht.begin();
  
  // Connect to WiFi
  connectToWiFi();
  
  // Initialize ThingSpeak
  ThingSpeak.begin(client);
  
  // Initialize web server
  setupWebServer();
  
  // Prepare LCD for monitoring display
  setupLCD();
}

void loop() {
  // Handle sensor readings
  if (sensorMillis >= sensorInterval) {
    readSensors();
    sensorMillis = 0;
  }
  
  // Handle LCD display updates
  if (displayMillis >= displayInterval) {
    updateLCD();
    displayMillis = 0;
  }
  
  // Handle ThingSpeak updates
  if (thingSpeakMillis >= thingSpeakInterval) {
    sendToThingSpeak();
    thingSpeakMillis = 0;
  }
  
  // Handle web server clients
  server.handleClient();
}

// WiFi connection function
void connectToWiFi() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting to WiFi");
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    lcd.print(".");
  }
  
  Serial.println("\nConnected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connected");
  lcd.setCursor(0, 1);
  lcd.print("IP:");
  lcd.print(WiFi.localIP());
  delay(3000);
}

// Web server setup function
void setupWebServer() {
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
  Serial.println("HTTP server started");
}

// LCD setup function
void setupLCD() {
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("Monitoring");
  lcd.setCursor(0, 1);
  lcd.print("Suhu   : ");
  lcd.setCursor(0, 2);
  lcd.print("K.Udara: ");
  lcd.setCursor(0, 3);
  lcd.print("K.Tanah: ");
}

// Welcome message function
void showWelcomeMessage() {
  lcd.setCursor(3, 0);
  lcd.print("Selamat Datang!");
  lcd.setCursor(0, 1);
  lcd.print("WS Agroteknologi IoT");
  lcd.setCursor(3, 3);
  lcd.print("-- UG MURO --");
  delay(5000);
}

// Sensor reading function
void readSensors() {
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  
  int soilValue = analogRead(SOIL_PIN);
  soilPercentage = map(soilValue, 4095, 0, 0, 100);
  soilPercentage = constrain(soilPercentage, 0, 100);
  
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print("°C\tHumidity: ");
  Serial.print(humidity);
  Serial.print("%\tSoil: ");
  Serial.print(soilPercentage);
  Serial.println("%");
}

// LCD update function
void updateLCD() {
  lcd.setCursor(9, 1);
  lcd.print("       ");
  lcd.setCursor(9, 1);
  lcd.print(temperature);
  lcd.setCursor(16, 1);
  lcd.print(char(223));
  lcd.print("C");

  lcd.setCursor(9, 2);
  lcd.print("       ");
  lcd.setCursor(9, 2);
  lcd.print(humidity);
  lcd.setCursor(17, 2);
  lcd.print("%");

  lcd.setCursor(9, 3);
  lcd.print("       ");
  lcd.setCursor(9, 3);
  lcd.print(soilPercentage);
  lcd.setCursor(17, 3);
  lcd.print("%");
}

// ThingSpeak upload function
void sendToThingSpeak() {
  ThingSpeak.setField(1, temperature);
  ThingSpeak.setField(2, humidity);
  ThingSpeak.setField(3, soilPercentage);

  int statusCode = ThingSpeak.writeFields(channelID, writeAPIKey);
  
  if (statusCode == 200) {
    Serial.println("ThingSpeak update successful");
  } else {
    Serial.println("ThingSpeak update failed. HTTP error code: " + String(statusCode));
  }
}

// Web server root handler
void handleRoot() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<title>Monitoring Agroteknologi</title>";
  html += "<meta http-equiv=\"refresh\" content=\"2\">";
  html += "<style>";
  html += "body {font-family: Arial, sans-serif; margin: 20px; background-color: #f5f5f5;}";
  html += ".container {background-color: white; padding: 20px; border-radius: 10px; box-shadow: 0 0 10px rgba(0,0,0,0.1);}";
  html += ".data-row {display: flex; justify-content: space-between; margin-bottom: 10px;}";
  html += ".data-label {font-weight: bold; color: #2e7d32;}";
  html += "</style></head><body>";
  html += "<div class=\"container\">";
  html += "<h1>Monitoring Agroteknologi IoT</h1>";
  html += "<div class=\"data-row\"><span class=\"data-label\">Suhu:</span><span>" + String(temperature) + " °C</span></div>";
  html += "<div class=\"data-row\"><span class=\"data-label\">Kelembaban Udara:</span><span>" + String(humidity) + " %</span></div>";
  html += "<div class=\"data-row\"><span class=\"data-label\">Kelembaban Tanah:</span><span>" + String(soilPercentage) + " %</span></div>";
  html += "<p><small>IP: " + WiFi.localIP().toString() + "</small></p>";
  html += "</div></body></html>";
  
  server.send(200, "text/html", html);
}

// Web server data API handler
void handleData() {
  String json = "{";
  json += "\"temperature\":" + String(temperature) + ",";
  json += "\"humidity\":" + String(humidity) + ",";
  json += "\"soil_moisture\":" + String(soilPercentage);
  json += "}";
  
  server.send(200, "application/json", json);
}