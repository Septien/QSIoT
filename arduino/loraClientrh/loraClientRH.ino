// rf69_client.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messageing client
// with the RH_RF69 class. RH_RF69 class does not provide for addressing or
// reliability, so you should only use RH_RF69  if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example rf69_server.
// Demonstrates the use of AES encryption, setting the frequency and modem 
// configuration
// Tested on Moteino with RFM69 http://lowpowerlab.com/moteino/
// Tested on miniWireless with RFM69 www.anarduino.com/miniwireless
// Tested on Teensy 3.1 with RF69 on PJRC breakout board

#include <SPI.h>
#include <RHReliableDatagram.h>
#include <RH_RF69.h>
#include <DHT.h>

#define RFM69_FREQ 915.0

#define RFM69_INT 2
#define RFM69_CS 10
#define RFM69_RST 3 // 

#define NODEID 2
#define NETWORKID 69
#define GATEWAY 1

// Packet sent/received indicator LED (optional):

#define LED           9 // LED positive pin
#define GND           8 // LED ground pin

//  #define LED1          6

// Pins at which the DHT sensor will be connected

#define DHTPIN 7
#define DHTTYPE DHT22


// Singleton instance of the radio driver
RH_RF69 rf69(RFM69_CS, RFM69_INT);
//RHReliableDatagram radio(rf69, NODEID);

DHT dht(DHTPIN, DHTTYPE);

void Blink(byte PIN, int DELAY_MS);
String getReadings();

void setup() 
{
  Serial.begin(9600);
  while (!Serial);
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, LOW);

  pinMode(LED,OUTPUT);
  digitalWrite(LED,LOW);
  pinMode(GND,OUTPUT);
  digitalWrite(GND,LOW);
  /*pinMode(LED1, OUTPUT);
  digitalWrite(LED1, LOW);*/

  // Manual reset of rfm
  digitalWrite(RFM69_RST, HIGH);
  delay(10);
  digitalWrite(RFM69_RST, LOW);
  delay(10);
  
  if (!rf69.init())
    ;
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
  // No encryption
  if (!rf69.setFrequency(RFM69_FREQ))
    ;

  // If you are using a high power RF69 eg RFM69HW, you *must* set a Tx power with the
  // ishighpowermodule flag set like this:
  rf69.setTxPower(20, true);

  // The encryption key has to be the same as the one in the server
  uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
  rf69.setEncryptionKey(key);

  /*uint8_t syncwords[2];
  syncwords[0] = 0x2d;
  syncwords[1] = NETWORKID;
  rf69.setSyncWords(syncwords, sizeof(syncwords));*/

  // Init the dht sensor
  dht.begin();
  uint8_t t = rf69.temperatureRead();
  Serial.println(t);
}

uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
uint8_t from;
static char sendBuffer[62];
static int sendLength = 0;
void loop()
{
  // Set up a "buffer" for characters that we'll send:
  bool sendF = false;

  // Get the data from the sensors and send it to the server}
  String data = getReadings();
  uint8_t t = rf69.temperatureRead();
  //Serial.println(t);
  if (data != '\0')
  {
    sendLength = 0;
    data += String(NODEID);
    for (int i = 0; i < data.length(); i++)
    {
      sendBuffer[i] = data[i];
      sendLength++;
    }
    sendF = true;
    Serial.println(sendBuffer);
  }

  if (sendF)
  {
    Serial.println(sendLength);
    rf69.send(sendBuffer, sendLength);
  }

  Blink(LED, 100);
  
  delay(10000);
}

String getReadings()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  /*if (isnan(h) || isnan(t)) {
    return String('\0');
  }*/
  //Blink(LED1, 100);
  return String(h) + "," + String(t) + ",";
}

// Blink an LED for a given number of ms
void Blink(byte PIN, int DELAY_MS)
{
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
}
