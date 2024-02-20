// Import some libraries which make all the sensors, screen and LoRaWAN radio work
#include "Arduino_SensorKit.h"
#include <TheThingsNetwork.h>
#include <CayenneLPP.h>

// Set your AppEUI and AppKey (use the values from your device on TheThingsNetwork console)
const char *appEui = "0000000000000000";
const char *appKey = "00000000000000000000000000000000";

// Set the LoRaWAN frequency to 868MHz
#define freqPlan TTN_FP_EU868

// Setup sensor pins
#define BUZZER 5
#define LED 6
#define Environment Environment_I2C

// Setup serial port names
#define loraSerial Serial1
#define debugSerial Serial

// Setup LoRaWAN connection
TheThingsNetwork ttn(loraSerial, debugSerial, freqPlan);

// Setup LPP data protocol
CayenneLPP lpp(51);



// This function runs once when we start up our device
// It is used to prepare our device and any sensors for their normal jobs
void setup() {

  // Set input and output pins
  pinMode(BUZZER,OUTPUT);
  pinMode(LED,OUTPUT);

  // Setup temperature sensor
  Environment.begin();

  // Setup OLED screen
  Oled.begin();
  Oled.setFlipMode(true);
  Oled.setFont(u8x8_font_chroma48medium8_r); 
  
  // Show startup message on OLED screen
  Oled.setCursor(0, 0);
  Oled.print("Starting Up");

  // Start serial ports for LoRaWAN radio and serial monitor
  loraSerial.begin(57600);
  debugSerial.begin(9600);


  // Wait a maximum of 10s for Serial Monitor to connect
  while (!debugSerial && millis() < 10000){
    delay(1);
  }

  // Wait 2 seconds
  delay(2000);

  // Set callback for incoming messages
  ttn.onMessage(message);

  // Output LoRaWAN status to serial monitor
  debugSerial.println("-- STATUS");
  ttn.showStatus();

  // Join the LoRaWAN network
  debugSerial.println("-- JOIN");
  Oled.setCursor(0, 1);
  Oled.print("Joining Network");

  // Try to join network
  ttn.join(appEui, appKey);

  // Successfully joined network
  Oled.setCursor(0, 2);
  Oled.print("Joined!");

  // Wait so you can read the message on the screen
  delay(3000);  
}



// This function repeats forever once our device has booted up and the setup function has finished
// This is where the main functionality of our device comes from
void loop() {

  // Clear the screen ready for the next loop
  Oled.clear();

  // Clear the LPP data ready for next reading
  lpp.reset();

  // Read the temperature sensor
  float temperature = Environment.readTemperature();

  // Add the sensor value to LPP data
  lpp.addTemperature(1, temperature);
  
  // Show sensor value on the screen
  Oled.setCursor(0, 0);
  Oled.print("Temp: ");
  Oled.print(temperature);
  

  // Send the data over LoRaWAN and show a message on the screen
  Oled.setCursor(0, 1);
  Oled.print("Sending...");
  ttn.sendBytes(lpp.getBuffer(), lpp.getSize());

  Oled.setCursor(0, 2);
  Oled.print("Sent!");

  // Delay before next loop (30 seconds as a minimum)
  delay(30000);

  // Automatically goes back to the start of the loop function
}



// This function will run when our device receives a LoRaWAN downlink message
void message(const uint8_t *payload, size_t size, port_t port)
{
  
  // Check if the message is for us on port 1
  if(port == 1){

    // Output message to serial monitor
    debugSerial.println("Received " + String(size) + " bytes on port " + String(port) + ":");

    // Clear the screen and show the received data
    Oled.clear();
    Oled.print("Received:");

    // Work through each byte which was received
    for (int i = 0; i < size; i++)
    {
      // Output to serial monitor
      debugSerial.print(" " + String(payload[i]));

      // Output to OLED screen
      Oled.setCursor(i, 1);
      Oled.print(payload[i]);
    }

    if(payload[0] == 0){
      digitalWrite(LED,LOW);
    }

    if(payload[0] == 1){
      digitalWrite(LED,HIGH);
    }

    // Buzz the buzzer  
    tone(BUZZER, 512); // Try changing the tone here (0-1023)
    delay(500); // Try changing the duration here (in milliseconds)
    noTone(BUZZER);
          
  }
  
  
}
