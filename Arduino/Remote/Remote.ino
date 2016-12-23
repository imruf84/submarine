#define NANO
#define USE_SERIAL

#define JOY_MIN 100
#define JOY_MAX 800

#ifdef NANO
#define JOY_X A0
#define JOY_Y A1
#endif

// Rádió.
#define DATA_PAYLOAD_COUNT 3
#define RF24_PAYLOAD_SIZE 16
#define MAX_DATA_STRING_LENGTH (RF24_PAYLOAD_SIZE * DATA_PAYLOAD_COUNT)
char data_string[MAX_DATA_STRING_LENGTH];
// Ideiglenes adatok törlése.
void clearDataString()
{
  strcpy(data_string, "");
}


#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
RF24 radio(9, 10);
byte addresses[][15] = {"remoteToSm", "smToRemote"};

// Rádió beállítása.
void setupRadio()
{
  radio.begin();
  radio.setRetries(15, 15);
  radio.setChannel(30);
  radio.setPayloadSize(RF24_PAYLOAD_SIZE);
  radio.setDataRate(RF24_250KBPS);
  radio.setCRCLength(RF24_CRC_16);
  radio.setPALevel(RF24_PA_MAX);
  radio.openWritingPipe(addresses[0]);
  radio.openReadingPipe(1, addresses[1]);
  radio.startListening();
}

// Beérkező rádió üzenet kezelése.
void handleRadioData(char *data)
{
  // Küldjük a soros portra.
#if defined(NANO) && defined(USE_SERIAL)
  Serial.println(data);
#endif
}

// Üzenet küldése rádión keresztül.
void sendDataOnRadio(char *data)
{
  radio.stopListening();
  while (!radio.write(data, MAX_DATA_STRING_LENGTH));
  // HACK: valamiért szabálytalan időközönként minden második
  //       üzenet nem érkezik meg, így minden üzenet küldése
  //       után küldök valami feleslegeset is
  while (!radio.write(".", 1));
  radio.powerUp();
  radio.startListening();
}
void sendDataOnRadio(String s)
{
  s.toCharArray(data_string, s.length() + 1);
  sendDataOnRadio(data_string);
  clearDataString();
}

void setup()
{
#if defined(NANO) && defined(USE_SERIAL)
  Serial.begin(9600);
  Serial.print("Set up Serial...");
  while (!Serial);
  Serial.println("OK");
#endif

  // RF Rádió.
#if defined(NANO) && defined(USE_SERIAL)
  Serial.print("Init Radio...");
#endif
  setupRadio();
#if defined(NANO) && defined(USE_SERIAL)
  Serial.println("OK");
#endif

  // Joystick.
  pinMode(JOY_X, INPUT);
  pinMode(JOY_Y, INPUT);

  delay(500);
}

int oldPosX = 0;
int oldPosY = 0;

void loop()
{

  // Joystick figyelése.
  int posX = analogRead(JOY_X);
  posX = (posX < JOY_MIN ? 2 : (posX > JOY_MAX ? 1 : 0));
  int posY = analogRead(JOY_Y);
  posY = (posY < JOY_MIN ? 1 : (posY > JOY_MAX ? 2 : 0));

  // Ha van változás az irányban akkor elküldjük.
  if (posX != oldPosX || posY != oldPosY) {
    oldPosX = posX;
    oldPosY = posY;
    sendDataOnRadio(String("m ") + String(posX) + String(" ") + String(posY));

#if defined(NANO) && defined(USE_SERIAL)
    Serial.print(posX);
    Serial.print(" ");
    Serial.print(posY);
    Serial.print(" ");
    Serial.print(analogRead(JOY_X));
    Serial.print(" ");
    Serial.print(analogRead(JOY_Y));
    Serial.println();
#endif
  }
}
