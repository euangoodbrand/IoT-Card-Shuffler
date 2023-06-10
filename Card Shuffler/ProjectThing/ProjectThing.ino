// Import required libraries
#include <WiFi.h>
#include <WebServer.h>
#include <aREST.h>
#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include <HTTPClient.h>
#include <Update.h>

// Define the firmware version and the server from which to fetch updates
#define FIRMWARE_VERSION 1
#define FIRMWARE_SERVER_IP_ADDR "172.20.10.7"
#define FIRMWARE_SERVER_PORT    "8000"

// HC-SR04 Sensor declarations
#define trigPin 17  // connected to A2_I34
#define echoPin 16  // connected to A1_DAC1
#define led 10  // some GPIO pin for LED1
#define led2 6  // some GPIO pin for LED2

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield();

// And connect 2 DC motors to port M3 & M4 !
Adafruit_DCMotor *L_MOTOR = AFMS.getMotor(4);
Adafruit_DCMotor *R_MOTOR = AFMS.getMotor(3);

// Create aREST instance
aREST rest = aREST();

// Personal WiFi parameters (0verridden by network provisioning)
const char* ssid = "iPhone";
const char* password = "password555";

// Access point (AP) settings
const char *apSSID = "MyESP32AP";
const char *apPassword = "mypassword";

WebServer server(80);

bool startOTAUpdate = false;

// Function to stop the motors
int stop(String message);

// Function to run the motors in opposite directions at a constant speed
int oppositeConstant(String message);

// Function to run the motors alternatively at a constant speed
int alternating(String message);

// Function to run the motors randomly at a constant speed
int randomMotion(String message);

// Function to perform a custom shuffling operation
int customShuffle(String command);

// Function to perform OTA firmware update
void performOTAUpdate();

/**
 * @brief Stops both motors.
 *
 * This function is designed to stop the operation of both motors. It sets the speed of both motors to 0
 * and releases them, effectively bringing them to a stop.
 *
 * @param message An input string message, not used in the current function context.
 * @return Returns 1 indicating successful stop of motors.
 */
int stop(String message) {
  Serial.println("Stopping");
  // Stop both motors
  L_MOTOR->setSpeed(0);
  L_MOTOR->run(RELEASE);

  R_MOTOR->setSpeed(0);
  R_MOTOR->run(RELEASE);

  return 1;
}

/**
 * @brief Handle root URL for displaying available WiFi networks and a form to enter credentials.
 *
 * This function is the request handler for the root URL ("/"). It scans and retrieves the available WiFi networks
 * and displays them in an HTML page. It also provides a form for users to enter their WiFi credentials and submit them.
 * Upon submission, the handleConnect() function is called.
 */
void handleRoot() {
  String html = "<html><head><title>ESP32 Provisioning</title></head>";
  html += "<body><h1>Available WiFi Networks:</h1>";

  // Scan for available WiFi networks
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

  // Display the WiFi credentials form
  html += "<h2>Enter your WiFi credentials:</h2>";
  html += "<form method=\"post\" action=\"/connect\">";
  html += "SSID: <input type=\"text\" name=\"ssid\"><br>";
  html += "Password: <input type=\"password\" name=\"password\"><br>";
  html += "<input type=\"submit\" value=\"Connect\">";
  html += "</form>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

/**
 * @brief Handle the connection request with the provided WiFi credentials.
 *
 * This function is the request handler for the connection URL ("/connect") after submitting the WiFi credentials form.
 * It attempts to connect to the specified WiFi network using the provided SSID and password. If the connection is successful,
 * it displays the connected network information. If the connection fails, it redirects back to the root URL ("/").
 */
void handleConnect() {
  String ssid = server.arg("ssid");
  String password = server.arg("password");

  // Switch to STA mode and begin connecting to the specified WiFi network
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());

  unsigned long startTime = millis();
  unsigned long connectionTimeout = 20000; // 20 seconds timeout

  // Wait until the connection is established or the timeout is reached
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - startTime > connectionTimeout) {
      Serial.println("Connection timed out.");
      server.sendHeader("Location", "/", true);
      server.send(302, "text/plain", "Failed to connect to WiFi. Redirecting...");
      return;
    }
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  // Display the IP address
  Serial.print("Successfully connected to the WiFi network. IP Address: ");
  Serial.println(WiFi.localIP());

  // Display the connected network information
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

/**
 * @brief Set up the device on startup.
 *
 * This function is called once on system startup. It initializes the Serial interface, the motor shield,
 * ultrasonic sensor, LEDs, functions for REST calls, WiFi connection, and server. It also sets the name and ID
 * of the device for REST calls and checks for firmware updates.
 */
void setup() {
  // Start Serial communication at a baud rate of 115200
  Serial.begin(115200);

  // Initialize the motor shield
  AFMS.begin();

  // Initialize Ultrasonic Sensor
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(led, OUTPUT);
  pinMode(led2, OUTPUT);

  // Register functions for handling REST calls
  rest.function("stop", stop);
  rest.function("oppositeConstant", oppositeConstant);
  rest.function("alternating", alternating);
  rest.function("randomMotion", randomMotion);
  rest.function("customShuffle", customShuffle);

  // Set the name and ID for the device
  rest.set_id("1");
  rest.set_name("card_shuffler");

  // Connect to the WiFi network using the specified SSID and password
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Wait until connected to the WiFi network
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server on port 80
  server.begin();
  Serial.println("Server started");

  // Print the local IP address
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Perform OTA update check
  performOTAUpdate();
}

/**
 * @brief Main program loop, executed repeatedly while the device is powered on.
 *
 * This loop handles incoming REST calls, performs ultrasonic jamming detection, and controls the system LEDs
 * based on the detected distance. If a jam is detected (distance < 4 cm), it stops all motors and turns on the red LED.
 * If no jam is detected, the green LED is lit.
 */
void loop() {
  // Handle REST calls
  server.handleClient();

  // Check for OTA updates
  if (startOTAUpdate) {
    startOTAUpdate = false;
    performOTAUpdate();
  }


  // Ultrasonic jamming detection
  long duration, distance;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration/2) / 29.1;

  // Print distance to the serial monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // Jamming detection
  if (distance < 4) {
    digitalWrite(led, HIGH); // Turn on red LED when a jam is detected
    digitalWrite(led2, LOW);
    stop("jam");  // Stop all motors if a jam is detected
  } else {
    digitalWrite(led, LOW);
    digitalWrite(led2, HIGH); // Turn on green LED when there's no jam
  }
  delay(500); // Wait for half a second before next detection
}


/**
 * @brief This function runs two motors in the opposite direction at a constant speed for a given duration.
 *
 * @param command A string that contains the desired speed and number of cards to be shuffled, separated by a comma.
 *
 * @return Returns 1 upon successful completion.
 */
int oppositeConstant(String command) {
  // Print to the console
  Serial.println("Constant");

  // Split the input string at the comma
  int commaIndex = command.indexOf(',');
  String speedStr = command.substring(0, commaIndex);
  String numCardsStr = command.substring(commaIndex + 1);

  // Convert the split strings to integers
  int speed = speedStr.toInt();
  int numCards = numCardsStr.toInt();
  int runTime = numCards * 1000;

  // Run both motors in opposite directions for the calculated time
  L_MOTOR->setSpeed(speed);
  L_MOTOR->run(BACKWARD);

  R_MOTOR->setSpeed(speed);
  R_MOTOR->run(BACKWARD);

  // Wait for the calculated time
  delay(runTime);

  // Then stop the motors
  L_MOTOR->setSpeed(0);
  L_MOTOR->run(RELEASE);

  R_MOTOR->setSpeed(0);
  R_MOTOR->run(RELEASE);

  return 1;
}

/**
 * @brief This function runs two motors alternatively at a constant speed for a given duration.
 *
 * @param command A string that contains the desired speed and number of cards to be shuffled, separated by a comma.
 *
 * @return Returns 1 upon successful completion.
 */
int alternating(String command) {
  // Print to the console
  Serial.println("Alternating");

  // Split the input string at the comma
  int commaIndex = command.indexOf(',');
  String speedStr = command.substring(0, commaIndex);
  String numCardsStr = command.substring(commaIndex + 1);

  // Convert the split strings to integers
  int speed = speedStr.toInt();
  int numCards = numCardsStr.toInt();
  int runTime = numCards * 1000;

  // Record the start time
  unsigned long startTime = millis();

  // Run the motors alternatively until the runTime is over
  while (millis() - startTime < runTime) {
    L_MOTOR->setSpeed(speed);
    L_MOTOR->run(FORWARD);
    R_MOTOR->setSpeed(0);
    R_MOTOR->run(RELEASE);

    delay(runTime / 2); // half of runTime for one motor

    R_MOTOR->setSpeed(speed);
    R_MOTOR->run(BACKWARD);
    L_MOTOR->setSpeed(0);
    L_MOTOR->run(RELEASE);

    delay(runTime / 2); // half of runTime for the other motor
  }

  // Make sure to stop the motors after the loop
  L_MOTOR->setSpeed(0);
  L_MOTOR->run(RELEASE);
  R_MOTOR->setSpeed(0);
  R_MOTOR->run(RELEASE);

  return 1;
}

/**
 * @brief This function runs either of the two motors randomly at a constant speed for a given duration.
 *
 * @param command A string that contains the desired speed and number of cards to be shuffled, separated by a comma.
 *
 * @return Returns 1 upon successful completion.
 */
int randomMotion(String command) {
  // Print to the console
  Serial.println("Random");

  // Split the input string at the comma
  int commaIndex = command.indexOf(',');
  String speedStr = command.substring(0, commaIndex);
  String numCardsStr = command.substring(commaIndex + 1);

  // Convert the split strings to integers
  int speed = speedStr.toInt();
  int numCards = numCardsStr.toInt();
  int runTime = numCards * 1000;

  // Record the start time and initialize elapsed time
  unsigned long startTime = millis();
  unsigned long elapsedTime = 0;

  // Run the motors randomly until the runTime is over
  while (elapsedTime < runTime) {
    int randomMotor = random(0, 2); // generates either 0 or 1

    // Determine which motor to run based on the random number
    if (randomMotor == 0) {
      L_MOTOR->setSpeed(speed);
      L_MOTOR->run(FORWARD);
      R_MOTOR->setSpeed(0);
      R_MOTOR->run(RELEASE);
    } else {
      R_MOTOR->setSpeed(speed);
      R_MOTOR->run(BACKWARD);
      L_MOTOR->setSpeed(0);
      L_MOTOR->run(RELEASE);
    }

    delay(1000); // wait for 1 second each iteration, you can adjust this depending on your requirement
    elapsedTime = millis() - startTime;
  }

  // Make sure to stop the motors after the loop
  L_MOTOR->setSpeed(0);
  L_MOTOR->run(RELEASE);
  R_MOTOR->setSpeed(0);
  R_MOTOR->run(RELEASE);

  return 1;
}

/**
 * @brief This function runs two motors with a custom shuffling operation for a given speed and number of cards.
 *
 * @param command A string that contains the desired speed, total number of cards,
 * number of left cards, and number of right cards to be shuffled, separated by commas.
 *
 * @return Returns 1 upon successful completion.
 */
int customShuffle(String command) {
  // Print to the console
  Serial.println("Custom Shuffling");

  // Split the input string at the commas
  int commaIndex = command.indexOf(',');
  String speedStr = command.substring(0, commaIndex);
  String cardsStr = command.substring(commaIndex + 1);

  commaIndex = cardsStr.indexOf(',');
  String numCardsStr = cardsStr.substring(0, commaIndex);
  String leftRightCardsStr = cardsStr.substring(commaIndex + 1);

  commaIndex = leftRightCardsStr.indexOf(',');
  String numLeftCardsStr = leftRightCardsStr.substring(0, commaIndex);
  String numRightCardsStr = leftRightCardsStr.substring(commaIndex + 1);

  // Convert the split strings to integers
  int shufflingSpeed = speedStr.toInt();
  int numCards = numCardsStr.toInt();
  int numLeftCards = numLeftCardsStr.toInt();
  int numRightCards = numRightCardsStr.toInt();

  int currentShuffle = 0;
  while (currentShuffle < numCards) {
    // Left Motor Shuffle
    if (currentShuffle < numLeftCards) {
      L_MOTOR->setSpeed(shufflingSpeed);
      L_MOTOR->run(FORWARD);
      delay(1000);
      L_MOTOR->run(RELEASE);
    }

    delay(1000);  // Delay time between running motors

    // Right Motor Shuffle
    if (currentShuffle < numRightCards) {
      R_MOTOR->setSpeed(shufflingSpeed);
      R_MOTOR->run(FORWARD);
      delay(1000);
      R_MOTOR->run(RELEASE);
    }

    currentShuffle++;
  }

  return 1;
}

/**
 * @brief Perform OTA update check and update if a new firmware version is available.
 *
 * This function performs an OTA (Over-The-Air) update check by connecting to the specified update server.
 * If a new firmware version is available, it downloads and installs the update. The update server URL,
 * firmware version, and firmware file name are defined in the global constants at the beginning of the sketch.
 */
void performOTAUpdate() {
  Serial.println("Checking for firmware updates...");
  HTTPClient http;

  // Specify the URL for the firmware update
  String firmwareUrl = String("http://") + FIRMWARE_SERVER_IP_ADDR + ":" + FIRMWARE_SERVER_PORT + "/firmware.bin";

  http.begin(firmwareUrl);
  int httpCode = http.GET();
  if (httpCode > 0) {
    if(httpCode == HTTP_CODE_OK) {
      int contentLength = http.getSize();

      // Start OTA Update
      if (Update.begin(contentLength)) {
        WiFiClient * stream = http.getStreamPtr();
        if (Update.writeStream(*stream) == contentLength) {
          if (Update.end(true)) {
            Serial.println("OTA update success!");
            ESP.restart();
          } else {
            Serial.printf("Error: %s\n", Update.errorString());
          }
        } else {
          Serial.println("Unable to read the stream for OTA update.");
        }
      } else {
        Serial.println("Not enough space to start OTA update.");
      }
    } else {
      Serial.printf("Firmware download failed with HTTP code: %d\n", httpCode);
    }
  } else {
    Serial.printf("Unable to connect to the server: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
}

