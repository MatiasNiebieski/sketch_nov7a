#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <DHT.h>


#define PINLED1 14
#define PINLED2 27
#define PINLED3 26
#define PINLED4 25
#define PINLED5 33


#define PINDHT 15
#define DHTTYPE DHT11  

#define BUTTON_UP 32
#define BUTTON_DOWN 35

// Replace with your network credentials
const char* ssid = "IoTB";
const char* password = "inventaronelVAR";

// Initialize DHT sensor
DHT dht(PINDHT, DHTTYPE);

// Variables for humidity thresholds
int lowerThreshold = 30;
int upperThreshold = 70;

// Create WebServer object on port 80
WebServer server(80);

// HTML webpage to be served (without buttons)
const char* html = R"=====(
<!DOCTYPE html>
<html>
<head>
<title>DHT Lectura del sensor</title>
<style>
body {
  font-family: sans-serif;
  text-align: center;
}
h1 {
  color: #000;
}
.container {
  display: flex;
  justify-content: center;
  align-items: center;
  flex-direction: column;
  margin-top: 30px;
}
.reading {
  margin: 30px 0;
  font-size: 20px;
}
.leds {
  display: flex;
  justify-content: center;
}
.led {
  width: 30px;
  height: 30px;
  border-radius: 30%;
  margin: 0 5px;
  background-color: #fff; 
}
.led.on {
  background-color: #f5f5dc; 
}
</style>
</head>
<body>
  <h1>DHT Sensor Readings</h1>
  <div class="container">
    <div class="reading" id="humidity">Humidity: -- %</div>
    <div class="leds">
      <div class="led" id="led1"></div>
      <div class="led" id="led2"></div>
      <div class="led" id="led3"></div>
      <div class="led" id="led4"></div>
      <div class="led" id="led5"></div>
    </div>
    <div>
      Lower Threshold: <span id="lowerThreshold">--</span>
      Upper Threshold: <span id="upperThreshold">--</span>
    </div>
  </div>
  <script>
    // Replace with your ESP's IP address 
    const espIpAddress = "10.8.5.181"; // Put your ESP's IP address here
    function updateReadings(data) { 
      document.getElementById("humidity").innerText = `Humidity: ${data.humidity} %`;
      document.getElementById("lowerThreshold").innerText = data.lower;
      document.getElementById("upperThreshold").innerText = data.upper;
      const leds = [
        document.getElementById("led1"),
        document.getElementById("led2"),
        document.getElementById("led3"),
        document.getElementById("led4"),
        document.getElementById("led5")
      ];
      leds.forEach((led, index) => {
        if (data.humidity >= data.lower + index * 10) {
          led.classList.add("on");
        } else {
          led.classList.remove("on");
        }
      });
    }
    setInterval(() => {
      fetch(`http://${espIpAddress}/readings`)
        .then(response => response.json())
        .then(data => updateReadings(data)); // Pass the entire data object
    }, 2000);
  </script>
</body>
</html>
)=====";

// Function to handle the root path
void handleRoot() {
  server.send(200, "text/html", html);
}

// Function to handle readings
void handleReadings() {
  float h = dht.readHumidity();
  String json = "{";
  json += "\"humidity\":" + String(h) + ",";
  json += "\"lower\":" + String(lowerThreshold) + ",";
  json += "\"upper\":" + String(upperThreshold);
  json += "}";
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  // Set LED pins as output
  pinMode(PINLED1, OUTPUT);
  pinMode(PINLED2, OUTPUT);
  pinMode(PINLED3, OUTPUT);
  pinMode(PINLED4, OUTPUT);
  pinMode(PINLED5, OUTPUT);

  // Set button pins as input with pullup (optional)
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);

  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Display IP address
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); 

  // Define web server routes
  server.on("/", handleRoot);
  server.on("/readings", handleReadings);

  // Start server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient(); // Handle web server requests

  delay(200);
  // Read humidity
  float h = dht.readHumidity();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Output readings to serial monitor
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.println(" %");

  // Print current thresholds to serial monitor
  Serial.print("Lower threshold: ");
  Serial.println(lowerThreshold);
  Serial.print("Upper threshold: ");
  Serial.println(upperThreshold);

  // You can keep the physical button functionality if needed
  if (digitalRead(BUTTON_UP) == LOW && upperThreshold < 100) {
    lowerThreshold += 5;  
    upperThreshold += 5;  
    delay(200); 
  }
  if (digitalRead(BUTTON_DOWN) == LOW && lowerThreshold > 0) {
    lowerThreshold -= 5;  
    upperThreshold -= 5;  
    delay(200); 
  }

  // Turn on LEDs based on humidity 
  if (h < lowerThreshold) {
    digitalWrite(PINLED1, LOW);
    digitalWrite(PINLED2, LOW);
    digitalWrite(PINLED3, LOW);
    digitalWrite(PINLED4, LOW);
    digitalWrite(PINLED5, LOW);
  } else if (h >= lowerThreshold && h < lowerThreshold + 10) {
    digitalWrite(PINLED1, HIGH);
    digitalWrite(PINLED2, LOW);
    digitalWrite(PINLED3, LOW);
    digitalWrite(PINLED4, LOW);
    digitalWrite(PINLED5, LOW);
  } else if (h >= lowerThreshold + 10 && h < lowerThreshold + 20) {
    digitalWrite(PINLED1, HIGH);
    digitalWrite(PINLED2, HIGH);
    digitalWrite(PINLED3, LOW);
    digitalWrite(PINLED4, LOW);
    digitalWrite(PINLED5, LOW);
  } else if (h >= lowerThreshold + 20 && h < lowerThreshold + 30) {
    digitalWrite(PINLED1, HIGH);
    digitalWrite(PINLED2, HIGH);
    digitalWrite(PINLED3, HIGH);
    digitalWrite(PINLED4, LOW);
    digitalWrite(PINLED5, LOW);
  } else if (h >= lowerThreshold + 30 && h < lowerThreshold + 40) {
    digitalWrite(PINLED1, HIGH);
    digitalWrite(PINLED2, HIGH);
    digitalWrite(PINLED3, HIGH);
    digitalWrite(PINLED4, HIGH);
    digitalWrite(PINLED5, LOW);
  } else { // h >= upperThreshold
    digitalWrite(PINLED1, HIGH);
    digitalWrite(PINLED2, HIGH);
    digitalWrite(PINLED3, HIGH);
    digitalWrite(PINLED4, HIGH);
    digitalWrite(PINLED5, HIGH);
  }
}