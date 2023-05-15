// Import required libraries
#include <WiFi.h>
#include <aREST.h>
#include <Wire.h>
#include <Adafruit_MotorShield.h>

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
  // Run both motors in opposite directions constantly
  L_MOTOR->setSpeed(200);
  L_MOTOR->run(FORWARD);
 
  R_MOTOR->setSpeed(200);
  R_MOTOR->run(BACKWARD);

  return 1;
}

int alternating(String command) {
  // Run each motor one after the other in opposite directions on a loop for 1 second each
  for (int i = 0; i < 5; i++) { // adjust the number of iterations as needed
    Serial.println("Alternating");
    L_MOTOR->setSpeed(200);
    L_MOTOR->run(FORWARD);
    R_MOTOR->setSpeed(0);
    R_MOTOR->run(RELEASE);

    delay(1000); // wait for 1 second

    R_MOTOR->setSpeed(200);
    R_MOTOR->run(BACKWARD);
    L_MOTOR->setSpeed(0);
    L_MOTOR->run(RELEASE);

    delay(1000); // wait for 1 second
  }
  
  // Make sure to stop the motors after the loop
  L_MOTOR->setSpeed(0);
  L_MOTOR->run(RELEASE);
  R_MOTOR->setSpeed(0);
  R_MOTOR->run(RELEASE);

  return 1;
}

int randomMotion(String command) {
  // Run each motor for one second randomly in opposite direction
  for (int i = 0; i < 5; i++) { // adjust the number of iterations as needed
    Serial.println("Random");
    int randomMotor = random(0, 2); // generates either 0 or 1
    
    if (randomMotor == 0) {
      L_MOTOR->setSpeed(200);
      L_MOTOR->run(FORWARD);
      R_MOTOR->setSpeed(0);
      R_MOTOR->run(RELEASE);
    } else {
      R_MOTOR->setSpeed(200);
      R_MOTOR->run(BACKWARD);
      L_MOTOR->setSpeed(0);
      L_MOTOR->run(RELEASE);
    }

    delay(1000); // wait for 1 second
  }

  // Make sure to stop the motors after the loop
  L_MOTOR->setSpeed(0);
  L_MOTOR->run(RELEASE);
  R_MOTOR->setSpeed(0);
  R_MOTOR->run(RELEASE);

  return 1;
}
