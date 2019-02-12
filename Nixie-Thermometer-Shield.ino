/* NIXIE THERMOMETER SHIELD FOR ARDUINO V1.2

  Author: My Electronics http://my-electronics.net
  Copyright (C) 2018 My Electronics

  For more information and to find out where to buy the kit visit http://my-electronics.net

  SAFTEY NOTES:
  This circuit design includes a switch‐mode voltage converter which generates 170 VDC.
  DO NOT USE IF YOU DON’T KNOW HOW TO HANDLE HIGH VOLTAGES.
  You are responsible for the safety during the assembly and operation of this device.

  The source code can be modified for personal and educational purposes. The commercial usage is restricted.

*/

#include <OneWire.h>

#define SLOT_MACHINE_EFFECT true  // Enable slot-machine effect for anti-cathode poisoning  

// Variables for timing 
unsigned long t = 0; 
unsigned long t_prev = 0; 

const int EN = A0;
OneWire ds(A1); // Signal pin of the temperature sensor on A1

// Pin for the minus sign Neon lamp
const int SGN = 9;

// Pins of the 1st Nixie driver
const int N1_A = 13;
const int N1_B = 12;
const int N1_C = 11;
const int N1_D = 10;

// Pins of the 2nd Nixie driver
const int N2_A = 5;
const int N2_B = 4;
const int N2_C = 3;
const int N2_D = 2;

bool nixieTable[10][4] = {
  {0, 0, 0, 0}, // 0
  {0, 0, 0, 1}, // 1
  {0, 0, 1, 0}, // 2
  {0, 0, 1, 1}, // 3
  {0, 1, 0, 0}, // 4
  {0, 1, 0, 1}, // 5
  {0, 1, 1, 0}, // 6
  {0, 1, 1, 1}, // 7
  {1, 0, 0, 0}, // 8
  {1, 0, 0, 1}, // 9
};


void setup() {
  pinMode(EN, OUTPUT);
  pinMode(SGN, OUTPUT);
  pinMode(N1_A, OUTPUT);
  pinMode(N1_B, OUTPUT);
  pinMode(N1_C, OUTPUT);
  pinMode(N1_D, OUTPUT);
  pinMode(N2_A, OUTPUT);
  pinMode(N2_B, OUTPUT);
  pinMode(N2_C, OUTPUT);
  pinMode(N2_D, OUTPUT);
  digitalWrite(EN, LOW); // Turn the tubes On

  // Start the serial port
  Serial.begin(9600);
}


void loop() 
{
  float temperature = getTemperature(ds); // Get the temperature from the sensor in degree Celsius (750 ms delay)
  Serial.println(temperature);

  int T = (int)(temperature >= 0.0 ? (temperature + 0.5) : (temperature - 0.5)); // Convert the temperature to integer, round half away from zero
 
  if (T >= 0) // Check for the sign 
    digitalWrite(SGN, 0); // Turn the Neon lamp Off
  else 
    digitalWrite(SGN, 1);
  // Display the 1st and 2nd digit of the temperature on the Nixie tube N1 and N2  
  T = abs(T);
  nixieWrite(N1_A, N1_B, N1_C, N1_D, (T / 10) % 10);
  nixieWrite(N2_A, N2_B, N2_C, N2_D, T % 10);

  #if (SLOT_MACHINE_EFFECT)
    t = millis();
    if (t - t_prev > 60000) { // Do slot-machine effect every 60 seconds
      t_prev = t;
      for (int i = 0; i < 50; i++) {
        nixieWrite(N1_A, N1_B, N1_C, N1_D, (i + 1) % 10);
        nixieWrite(N2_A, N2_B, N2_C, N2_D, i % 10);
        delay(100); 
      }
    }
  #endif
  
}


void nixieWrite(int a, int b, int c, int d, int digit) {
  digitalWrite(d, nixieTable[digit][0]);
  digitalWrite(c, nixieTable[digit][1]);
  digitalWrite(b, nixieTable[digit][2]);
  digitalWrite(a, nixieTable[digit][3]);
}

float getTemperature(OneWire &ds) { // Returns the temperature from the DS18B20 in °C
  
  byte addr[8];
  if (!ds.search(addr)) {  // Find sensor, reset search if no more sensors on chain
    ds.reset_search();
    return;
  }
  
  if (OneWire::crc8(addr, 7) != addr[7]) {
    Serial.println("CRC is not valid!");
    return;
  }
  
  if (addr[0] != 0x10 && addr[0] != 0x28) { // 0x10: DS18S20, 0x28: DS18B20
    Serial.print("Device is not recognized: 0x");
    Serial.println(addr[0], HEX);
    return;
  }
  
  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1); // Start the convertion with parasite power on
  delay(750); // Wait for 750 ms. See Tab. 2 in the DS18S20 datasheet
  ds.reset();
  ds.select(addr);
  ds.write(0xBE); // Send the Read Scratchpad command
  byte data[12];
  for (int i = 0; i < 9; i++) { // Recieve 9 bytes
    data[i] = ds.read();
  }
  //ds.reset_search();
  byte LSB = data[0];
  byte MSB = data[1];
  int value = ((MSB << 8) | LSB); // Convert the data to signed integer using two's complement
  float temperature = value*0.0625;  // Convert the temperature value in degree Celsius
  
  return temperature;
}
