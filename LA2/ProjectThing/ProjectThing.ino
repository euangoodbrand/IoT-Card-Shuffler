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

// Functions
int stop(String message);
int oppositeConstant(String message);
int alternating(String message);
int randomMotion(String message);
int customShuffle(String command);
void performOTAUpdate();


/**
 * @brief Stops both motors.
 * 
 * This function is designed to stop the operation of both motors. It sets the speed of both motors to 0 
 * and releases them, effectively bringing them to a stop.
 *
 * @param command An input string command, not used in the current function context.
 * @return Returns 1 indicating successful stop of motors.
 */
int stop(String command) {
  // Print a message to the serial monitor
  Serial.println("Stopping");

  // Stop the left motor by setting its speed to 0 and releasing it
  L_MOTOR->setSpeed(0);
  L_MOTOR->run(RELEASE);

  // Stop the right motor by setting its speed to 0 and releasing it
  R_MOTOR->setSpeed(0);
  R_MOTOR->run(RELEASE);

  // Return 1 to indicate that the motors were successfully stopped
  return 1;
}

/**
 * @brief Sets up the device on startup.
 *
 * This function is called once on system startup. It initializes the Serial interface, the motor shield, 
 * ultrasonic sensor, LEDs, functions for REST calls, WiFi connection, and server. It also sets the name and ID 
 * of the device for REST calls and checks for firmware updates. 
 */
void setup(void)
{  
  // Initialize Serial communication at a baud rate of 115200
  Serial.begin(115200);

  // Initialize the motor shield
  AFMS.begin();  

  // Set the pin mode for the ultrasonic sensor pins and LED pins
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
      
  // Set the ID and name for the device, these values will be used for the REST calls
  rest.set_id("1");
  rest.set_name("card_shuffler");
  
  // Connect to the WiFi network using the given ssid and password
  WiFi.begin(ssid, password);
  // Wait until the connection is established
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
 
  // Start the server for incoming REST calls
  server.begin();
  Serial.println("Server started");
  
  // Print the local IP address to the Serial monitor
  Serial.println(WiFi.localIP());

  // Perform an OTA update check after the WiFi connection is established
  performOTAUpdate();  
}


/**
 * @brief Main program loop, executed repeatedly while the device is powered on.
 *
 * This loop checks for incoming REST calls, performs ultrasonic jamming detection, and controls the system LEDs
 * according to the detected distance. If a jam is detected (distance < 4 cm), it stops all motors and turns on the red LED.
 * If no jam is detected, the green LED is lit.
 */
void loop() {
  
  // Handle incoming REST calls
  WiFiClient client = server.available();
  if (!client) {
    // No client connected, return from the loop
    return;
  }
  // Wait until client data is available
  while(!client.available()){
    delay(1);
  }
  // Handle the REST call
  rest.handle(client);

  // Ultrasonic jamming detection process
  long duration, distance;
  // Send a low pulse to the trigPin
  digitalWrite(trigPin, LOW);  
  delayMicroseconds(2);

  // Send a high pulse to the trigPin
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10); 
  digitalWrite(trigPin, LOW);

  // Measure the duration of the pulse on the echoPin
  duration = pulseIn(echoPin, HIGH);

  // Calculate the distance using the speed of sound ( 29.1 μs/cm )
  distance = (duration/2) / 29.1;
  
  // Print the calculated distance to the serial monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  
  // Jamming detection happens here
  if (distance < 4) {
    // If a jam is detected (distance < 4cm), turn on the red LED
    digitalWrite(led,HIGH); 
    digitalWrite(led2,LOW);

    // Stop all motors if a jam is detected
    stop("jam");  
  }
  else {
    // If no jam is detected, turn on the green LED
    digitalWrite(led,LOW);
    digitalWrite(led2,HIGH); 
  }
  // Wait for half a second before the next detection
  delay(500);
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
 * @brief This function checks for firmware updates and performs an over-the-air (OTA) update if one is available.
 *
 * It connects to a specified server to check for updates, downloads the new firmware if available, 
 * and then applies the update. If the update is successful, the system will restart. 
 * Error messages will be logged to the console if any step fails.
 *
 * Please note, for the update to begin, the new firmware size must be less than the available program space.
 */
void performOTAUpdate() {
  // Print to the console
  Serial.println("Checking for firmware updates...");

  // Create an HTTPClient object
  HTTPClient http;

  // Specify the URL for the firmware update
  String firmwareUrl = String("http://") + FIRMWARE_SERVER_IP_ADDR + ":" + FIRMWARE_SERVER_PORT + "/firmware.bin";

  // Start a connection to the server
  http.begin(firmwareUrl);
  int httpCode = http.GET();

  // Check the HTTP response code
  if (httpCode > 0) {
    if(httpCode == HTTP_CODE_OK) {
      // Get the size of the new firmware
      int contentLength = http.getSize();

      // Start the OTA Update
      if (Update.begin(contentLength)) {
        // Get a pointer to the HTTP response stream
        WiFiClient * stream = http.getStreamPtr();

        // Write the new firmware to the flash memory
        if (Update.writeStream(*stream) == contentLength) {
          // Finalize the update and restart the system if it was successful
          if (Update.end(true)) {
            Serial.println("OTA update success!");
            ESP.restart();
          } else {
            // Log an error message if the update failed
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

  // End the HTTP connection
  http.end();
}
