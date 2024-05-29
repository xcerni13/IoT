#include <Wire.h>
#include <DallasTemperature.h>

// CAN Receive Example

// https://github.com/fdebrabander/Arduino-LiquidCrystal-I2C-library
#include <LiquidCrystal_I2C.h>

// https://github.com/coryjfowler/MCP_CAN_lib
#include <mcp_can.h>
#include <SPI.h>

//pins for rotary encoder
#define CLK PB4
#define DT PB5
#define SW PA15


LiquidCrystal_I2C lcd(0x3F, 16, 2);
long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];
char msgString1[128];                        // Array to store serial string
char msgString2[128];

float tempF;
int counter = 0;
float old_temp;
float old_pressure;
int country = 1;

int lastStateCLK;
int currentStateCLK;
unsigned long LastButtonPress;

#define CAN0_INT PB15                              // Set INT to pin PB5
MCP_CAN CAN0(PA4);                               // Set CS to pin PA4


void setup()
{ afio_cfg_debug_ports(AFIO_DEBUG_SW_ONLY); 
  Serial.begin(9600);
  
  // Initialize MCP2515 running at 16MHz with a baudrate of 500kb/s and the masks and filters disabled.
  if(CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_16MHZ) == CAN_OK)
    Serial.println("MCP2515 Initialized Successfully!");
  else
    Serial.println("Error Initializing MCP2515...");
  
  CAN0.setMode(MCP_NORMAL);                     // Set operation mode to normal so the MCP2515 sends acks to received data.

  pinMode(CAN0_INT, INPUT);                            // Configuring pin for /INT input
  
  Serial.println("MCP2515 Library Receive Example...");

  // init LCD
  lcd.begin();

  pinMode(CLK,INPUT);
  pinMode(DT,INPUT);
  pinMode(SW, INPUT_PULLUP);
  lastStateCLK = digitalRead(CLK);
}

void loop()
{
  float temp;
  float pressure;
  
  currentStateCLK = digitalRead(CLK);
  // Remember last CLK state
  lastStateCLK = currentStateCLK;

  int btnState = digitalRead(SW);
  if (btnState == LOW) {
  if (millis() - LastButtonPress > 50)
  {
    country = country * (-1);
    Serial.print("BUTTON PRESSED");
    Serial.print(country);
  }
  LastButtonPress = millis();
  }
  
  if(!digitalRead(CAN0_INT))                         // If CAN0_INT pin is low, read receive buffer
  {
    CAN0.readMsgBuf(&rxId, &len, rxBuf);      // Read data: len = data length, buf = data byte(s)
    
//    Serial.print("First reading: ");
//    Serial.print(*(float*)rxBuf);
//    Serial.println();

    if (!counter)
    {
      temp = *(float*)rxBuf;
      old_temp = temp;
      pressure = old_pressure;
      counter++; 
    }
    else
    {
      pressure = *(float*)rxBuf;
      old_pressure = pressure;
      temp = old_temp;
      counter--;
    }

    if (country == -1)
      temp = DallasTemperature::toFahrenheit(temp);
      
//    Serial.print("Temp in Fahrenheit: ");
//    Serial.print(tempF);

    // Arduino's sprintf does not support floats
    sprintf(msgString1, "Temp: %d.%d", (int)temp, (int)(temp * 100) % 100);
    sprintf(msgString2, "Press: %d.%d", (int)pressure, (int)(pressure * 100) % 100);
    
    Serial.println();
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(msgString1);

    //lcd.clear();
    lcd.setCursor(0,1);
    lcd.print(msgString2);
  }
}

/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
