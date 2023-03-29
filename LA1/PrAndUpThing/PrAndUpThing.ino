// PrAndUpThing.ino

#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <Update.h>

#define FIRMWARE_VERSION 4
#define FIRMWARE_SERVER_IP_ADDR "172.20.10.7"
#define FIRMWARE_SERVER_PORT    "8000"

const char *apSSID = "MyESP32AP";
const char *apPassword = "mypassword";

WebServer server(80);

bool startOTAUpdate = false;

void handleRoot() {
  String html = "<html><head><title>ESP32 Provisioning</title></head>";
  html += "<body><h1>Available WiFi Networks:</h1>";

  int n = WiFi.scanNetworks();
  if (n == 0) {
    html += "<p>No networks found.</p>";
  } else {
    html += "<ul>";
    for (int i = 0; i < n; i++) {
      html += "<li>";
      html += WiFi.SSID(i);
      html += "</li>";
    }
    html += "</ul>";
  }

  html += "<h2>Enter your WiFi credentials:</h2>";
  html += "<form method=\"post\" action=\"/connect\">";
  html += "SSID: <input type=\"text\" name=\"ssid\"><br>";
  html += "Password: <input type=\"password\" name=\"password\"><br>";
  html += "<input type=\"submit\" value=\"Connect\">";
  html += "</form>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleConnect() {
  String ssid = server.arg("ssid");
  String password = server.arg("password");

  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid.c_str(), password.c_str());

  unsigned long startTime = millis();
  unsigned long connectionTimeout = 20000; // 20 seconds timeout

  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - startTime > connectionTimeout) {
      Serial.println("Connection timed out.");
      server.send(400, "text/html", "Failed to connect to WiFi. Please try again.");
      return;
    }
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  String html = "<html><head><title>ESP32 Provisioning</title></head>";
  html += "<body><h1>Connected to WiFi:</h1>";
  html += "<p>SSID: ";
  html += ssid;
  html += "</p><p>IP address: ";
  html += WiFi.localIP().toString();
  html += "</p>";
  html += "<p>Press the button connected to the breadboard to start the OTA update.</p>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void onButtonPress() {
  startOTAUpdate = true;
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(apSSID, apPassword);

  server.on("/", handleRoot);
  server.on("/connect", HTTP_POST, handleConnect);
  server.begin();

  // Set up GPIO pin for the switch
  pinMode(5, INPUT_PULLUP);

  // Attach interrupt to trigger when the switch is pressed
  attachInterrupt(digitalPinToInterrupt(5), onButtonPress, FALLING);
}

void loop() {
  server.handleClient();

  if (startOTAUpdate) {
    startOTAUpdate = false; // Reset the flag
    performOTAUpdate(); // Start the OTA update
  }
}

// Keep the performOTAUpdate and doCloudGet functions


void performOTAUpdate() {
  Serial.println("Starting performOTAUpdate function...");
  HTTPClient http;
  int respCode;
  int highestAvailableVersion = -1;

  // Read the version file from the cloud
  respCode = doCloudGet(&http, "version.txt");
  if (respCode > 0) {
    highestAvailableVersion = atoi(http.getString().c_str());
  } else {
    Serial.printf("Couldn't get version! Return code: %d\n", respCode);
  }
  http.end(); // Free resources

  // Check if the firmware needs updating
  if (respCode < 0) {
    return;
  } else if (FIRMWARE_VERSION >= highestAvailableVersion) {
    Serial.printf("Firmware is up to date\n");
    return;
  }

  // Do a firmware update
  Serial.printf(
    "Upgrading firmware from version %d to version %d\n",
    FIRMWARE_VERSION, highestAvailableVersion
  );

  // Do a GET for the .bin
  String binName = String(highestAvailableVersion);
  binName += ".bin";
  respCode = doCloudGet(&http, binName);
  int updateLength = http.getSize();

  if (respCode > 0 && respCode != 404) {
    Serial.printf(".bin code/size: %d; %d\n\n", respCode, updateLength);
  } else {
    Serial.printf("Failed to get .bin! Return code is: %d\n", respCode);
    http.end();
    return;
  }

  WiFiClient stream = http.getStream();
  if (Update.begin(updateLength)) {
    Serial.printf("Starting OTA update...\n");
    if (Update.writeStream(stream) == updateLength) {
      Serial.printf("Successfully written %d bytes. Finishing the update...\n", updateLength);
      if (Update.end()) {
        Serial.println("OTA update successfully finished. Rebooting...");
        ESP.restart();
      } else {
        Serial.printf("Error finishing the update: %d\n", Update.getError());
      }
    } else {
      Serial.printf("Error during the update: %d\n", Update.getError());
    }
  } else {
    Serial.printf("Not enough space to start OTA update\n");
  }
  stream.flush();
}

int doCloudGet(HTTPClient *http, String fileName) {
  String url =
    String("http://") + FIRMWARE_SERVER_IP_ADDR + ":" +
    FIRMWARE_SERVER_PORT + "/" + fileName;
  Serial.printf("Getting %s\n", url.c_str());

  http->begin(url);
  http->addHeader("User-Agent", "ESP32");
  return http->GET();
}
