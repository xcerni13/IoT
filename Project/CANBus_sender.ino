/********************************************************************/
// First we include the libraries
#include <OneWireSTM.h> 
#include <DallasTemperature.h>
/********************************************************************/
// Data wire is plugged into pin PA15 on the BluePill 
#define ONE_WIRE_BUS PA15 

// https://github.com/coryjfowler/MCP_CAN_lib
#include <mcp_can.h>
#include <SPI.h>

MCP_CAN CAN0(PA4);     // Set CS to pin PA4
/********************************************************************/
// Setup a oneWire instance to communicate with any OneWire devices  
// (not just Maxim/Dallas temperature ICs) 
OneWire oneWire(ONE_WIRE_BUS); 
/********************************************************************/
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);
/********************************************************************/ 
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
Adafruit_BMP280 bme;
#define SEALEVELPRESSURE_HPA (1013.25)
bool status;

float tempF;
float tempBME;
float pressBME;
float altBME;

void setup(void) 
{ 
  afio_cfg_debug_ports(AFIO_DEBUG_SW_ONLY); 
  Serial.begin(9600);

  // Initialize MCP2515 running at 16MHz with a baudrate of 500kb/s and the masks and filters disabled.
  if(CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_16MHZ) == CAN_OK) Serial.println("MCP2515 Initialized Successfully!");
  else Serial.println("Error Initializing MCP2515...");

  CAN0.setMode(MCP_NORMAL);   // Change to normal mode to allow messages to be transmitted

  status = bme.begin(0x76); //adafruit
} 

byte data[8] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
float temp;

void loop(void) 
{ 
  // call sensors.requestTemperatures() to issue a global temperature 
  // request to all devices on the bus 
  /********************************************************************/
  Serial.print(" Requesting temperatures..."); 
  sensors.requestTemperatures(); // Send the command to get temperature readings 
  Serial.println("DONE"); 
  /********************************************************************/
  Serial.print("Temperature is: "); 
  temp = sensors.getTempCByIndex(0);
  Serial.print(temp);
  
  delay(1000); 
  byte bytesTemp[4];
  byte bytesPress[4];
  byte bytesAlt[4];
  
  //original Dallas Temperature temp collection
  *((float *)bytesTemp) = temp;
  // send data:  ID = 0x100, Standard CAN Frame, Data length = 8 bytes, 'data' = array of data bytes to send
  byte sndStat = CAN0.sendMsgBuf(0x100, 0, 4, bytesTemp);
  if(sndStat == CAN_OK){
    Serial.println("Message Sent Successfully!");
  } else {
    Serial.println("Error Sending Message...");
  }
  delay(1000); 

//  tempBME = bme.readTemperature();
  //AdaFruit Pressure reading/collection
  *((float *)bytesPress) = bme.readPressure() / 100.0F;
  

  // send data:  ID = 0x100, Standard CAN Frame, Data length = 8 bytes, 'data' = array of data bytes to send
  sndStat = CAN0.sendMsgBuf(0x100, 0, 4, bytesPress);
  if(sndStat == CAN_OK){
    Serial.println("Message Sent Successfully!");
  } else {
    Serial.println("Error Sending Message...");
  }

  *((float *)bytesAlt) = bme.readAltitude(SEALEVELPRESSURE_HPA);
}
