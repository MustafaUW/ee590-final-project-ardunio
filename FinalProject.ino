/*********************************************************************
  * Laura Arjona. PMPEE590'
  * Modified By Mustapha Ennaimy 
  * to use BLE functionality for final project
*********************************************************************/
#include <Arduino.h>
#include <ctype.h>
#include <SPI.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"
#include <Adafruit_CircuitPlayground.h>

#include "FinalProject.h"

#if SOFTWARE_SERIAL_AVAILABLE
  #include <SoftwareSerial.h>
#endif


// Strings to compare incoming BLE messages
String start = "start";
String readtemp = "readtemp";
String stp = "stp";
boolean startMotors = false; 
int redcolor = 1;
int greencolor = 1;
int bluecolor = 1;
int timer = 1000;

int  sensorTemp = 0;

// pins for the DC motor 
int enA = 6;
int enB = 9;
const int IA = 12;  //  pin 9 connected to pin IA
const int IB = 10;  //  pin 9 connected to pin IA


//contorl the motor
byte speed = 250;  // change this (0-255) to control the speed of the motor


/*=========================================================================
    APPLICATION SETTINGS
    -----------------------------------------------------------------------*/
    #define FACTORYRESET_ENABLE         0
    #define MINIMUM_FIRMWARE_VERSION    "0.6.6"
    #define MODE_LED_BEHAVIOUR          "MODE"
/*=========================================================================*/

// Create the bluefruit object, either software serial...uncomment these lines

Adafruit_BluefruitLE_UART ble(BLUEFRUIT_HWSERIAL_NAME, BLUEFRUIT_UART_MODE_PIN);

/* ...hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */
// Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

/* ...software SPI, using SCK/MOSI/MISO user-defined SPI pins and then user selected CS/IRQ/RST */
//Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_SCK, BLUEFRUIT_SPI_MISO,
//                             BLUEFRUIT_SPI_MOSI, BLUEFRUIT_SPI_CS,
//                             BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);


// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

/**************************************************************************/
/*!
    @brief  Sets up the HW an the BLE module (this function is called
            automatically on startup)
*/
/**************************************************************************/
void setup(void)
{
  CircuitPlayground.begin();
  
 // set up the pins for the motor to output 
  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(IA, OUTPUT); // set pins to output
  pinMode(IB, OUTPUT); // set pins to output
    
  Serial.begin(115200);
  Serial.println(F("Adafruit Bluefruit Command <-> Data Mode Example"));
  Serial.println(F("------------------------------------------------"));

  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ){
      error(F("Couldn't factory reset"));
    }
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  Serial.println(F("Please use Adafruit Bluefruit LE app to connect in UART mode"));
  Serial.println(F("Then Enter characters to send to Bluefruit"));
  Serial.println();

  ble.verbose(false);  // debug info is a little annoying after this point!

  /* Wait for connection */
  while (! ble.isConnected()) {
      delay(500);
  }

  Serial.println(F("******************************"));

  // LED Activity command is only supported from 0.6.6
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
  {
    // Change Mode LED Activity
    Serial.println(F("Change LED activity to " MODE_LED_BEHAVIOUR));
    ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
  }

  // Set module to DATA mode
  Serial.println( F("Switching to DATA mode!") );
  ble.setMode(BLUEFRUIT_MODE_DATA);

  Serial.println(F("******************************"));

  CircuitPlayground.setPixelColor(0,255,0,0);
 
  delay(1000);
}
/**************************************************************************/
/*!
    @brief  Constantly poll for new command or response data
*/
/**************************************************************************/
void loop(void)
{
  // Save received data to string
  String received = "";
  int c;
  while ( ble.available() )
  {
     c = ble.read();
    Serial.print((char)c);
    received += (char)c;
        delay(50);

  }
  // start using the phone 
  if(start== received){
     startMotors = true;
  }

  // check if incoming data is time change
  if(isDigit(received.charAt(0))){
  speed = (received.toInt()); 
  } 
  
if (startMotors) {
    //LED to blue when starting the motor
     CircuitPlayground.setPixelColor(0,0,255,0);
    //start the motors 
    forward();
   }
  if (stp == received){
   startMotors = false;
   CircuitPlayground.clearPixels();
   stop();

    }
 
 if(readtemp == received){
        
    sensorTemp = CircuitPlayground.temperature(); // returns a floating point number in Centigrade
    Serial.println("Read temperature sensor"); 
    delay(10);

   //Send data to Android Device
    char output[8];
    String data = "";
    data += sensorTemp;
    Serial.println(data);
    data.toCharArray(output,8);
    // turn LED BLUE when temp request
    CircuitPlayground.setPixelColor(9,0,0,255);
    ble.print(data);
    delay(1000);
    CircuitPlayground.clearPixels(); 
  }
}

void forward()
{
     digitalWrite(enA, speed);
     digitalWrite(IA,HIGH);
     digitalWrite(enB, speed);
     digitalWrite(IB,HIGH);
}

void stop()
{   
     digitalWrite(IA, 0);
     digitalWrite(enA,0);
     digitalWrite(IB, 0);
     digitalWrite(enB,0);
     speed = 250;
}

 
