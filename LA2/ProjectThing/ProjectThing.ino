// Import required libraries
#include <WiFi.h>
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

// WiFi parameters
const char* ssid = "iPhone";
const char* password = "password555";

// The port to listen for incoming TCP connections 
#define LISTEN_PORT           80

// Create an instance of the server
WiFiServer server(LISTEN_PORT);

// Function
int stop(String message);
int oppositeConstant(String message);
int alternating(String message);
int randomMotion(String message);
// int detectJamming();
void performOTAUpdate();


int stop(String command) {
  Serial.println("Stopping");
  // Stop both motors
  L_MOTOR->setSpeed(0);
  L_MOTOR->run(RELEASE);

  R_MOTOR->setSpeed(0);
  R_MOTOR->run(RELEASE);

  return 1;
}




void setup(void)
{  
  // Start Serial
  Serial.begin(115200);

  // Init motor shield
  AFMS.begin();  

  // Init Ultrasonic Sensor
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(led, OUTPUT);
  pinMode(led2, OUTPUT);

  // Functions          
  rest.function("stop", stop);
  rest.function("oppositeConstant", oppositeConstant);
  rest.function("alternating", alternating);
  rest.function("randomMotion", randomMotion);
  // rest.function("detectJamming", detectJamming);
      
  // Give name and ID to device
  rest.set_id("1");
  rest.set_name("card_shuffler");
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
 
  // Start the server
  server.begin();
  Serial.println("Server started");
  
  // Print the IP address
  Serial.println(WiFi.localIP());

  // Check for updates after setting up the WiFi
  performOTAUpdate();
  
}

void loop() {
  
  // Handle REST calls
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  while(!client.available()){
    delay(1);
  }
  rest.handle(client);

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
  
  if (distance < 4) {  // This is where the jamming detection happens
    digitalWrite(led,HIGH); // Red LED turns on when a jam is detected
    digitalWrite(led2,LOW);
    stop("jam");  // Stop all motors if a jam is detected
  }
  else {
    digitalWrite(led,LOW);
    digitalWrite(led2,HIGH); // Green LED turns on when there's no jam
  }
  delay(500); // Wait for half a second before next detection
 
}


int oppositeConstant(String command) {
  Serial.println("Constant");

  int commaIndex = command.indexOf(',');
  String speedStr = command.substring(0, commaIndex);
  String numCardsStr = command.substring(commaIndex + 1);

  int speed = speedStr.toInt();
  int numCards = numCardsStr.toInt();
  int runTime = numCards * 1000;

  
  // Run both motors in opposite directions for the calculated time
  L_MOTOR->setSpeed(speed);
  L_MOTOR->run(BACKWARD);
 
  R_MOTOR->setSpeed(speed);
  R_MOTOR->run(BACKWARD);

  delay(runTime);  // wait for the calculated time

  // Then stop the motors
  L_MOTOR->setSpeed(0);
  L_MOTOR->run(RELEASE);

  R_MOTOR->setSpeed(0);
  R_MOTOR->run(RELEASE);

  return 1;
}


int alternating(String command) {
  Serial.println("Alternating");
  int commaIndex = command.indexOf(',');
  String speedStr = command.substring(0, commaIndex);
  String numCardsStr = command.substring(commaIndex + 1);

  int speed = speedStr.toInt();
  int numCards = numCardsStr.toInt();
  int runTime = numCards * 1000;

  unsigned long startTime = millis();

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

int randomMotion(String command) {
  Serial.println("Random");
  int commaIndex = command.indexOf(',');
  String speedStr = command.substring(0, commaIndex);
  String numCardsStr = command.substring(commaIndex + 1);

  int speed = speedStr.toInt();
  int numCards = numCardsStr.toInt();
  int runTime = numCards * 1000;

  unsigned long startTime = millis();
  unsigned long elapsedTime = 0;

  while (elapsedTime < runTime) {
    int randomMotor = random(0, 2); // generates either 0 or 1

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
