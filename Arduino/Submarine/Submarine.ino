#define NANO
#define USE_SERIAL

#define MOTOR_SPEED 255

#ifdef NANO
#define MOTOR_LEFT_FORWARD 2
#define MOTOR_LEFT_BACKWARD 3
#define MOTOR_RIGHT_FORWARD 4
#define MOTOR_RIGHT_BACKWARD 5
#endif

// Motorok művelettáblája.
/*
 *   x|  y| LF| LB| RF| RB
 *   0|  0|  0|  0|  0|  0
 *   0|  1|  0|  1|  0|  1
 *   0|  2|  1|  0|  1|  0
 *   1|  0|  0|  1|  1|  0
 *   1|  1|  0|  0|  0|  1
 *   1|  2|  0|  0|  1|  0
 *   2|  0|  1|  0|  0|  1
 *   2|  1|  0|  1|  0|  0
 *   2|  2|  1|  0|  0|  0
 */
const byte MOTION_DATA[][3][4] = {{{0, 0, 0, 0}, {0, 1, 0, 1}, {1, 0, 1, 0}}, {{0, 1, 1, 0}, {0, 0, 0, 1}, {0, 0, 1, 0}}, {{1, 0, 0, 1}, {0, 1, 0, 0}, {1, 0, 0, 0}}};
const byte MOTORS[] = {MOTOR_LEFT_FORWARD, MOTOR_LEFT_BACKWARD, MOTOR_RIGHT_FORWARD, MOTOR_RIGHT_BACKWARD};

// Rádió.
#define RF24_PAYLOAD_SIZE 16
#define MAX_DATA_STRING_LENGTH (RF24_PAYLOAD_SIZE * 2)
char data_string[MAX_DATA_STRING_LENGTH];
String prevRadioData = "";
// Ideiglenes adatok törlése.
void clearDataString()
{
  strcpy(data_string, "");
}


#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
RF24 radio(9, 10);
byte addresses[][15] = {"smToRemote", "remoteToSm"};

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

  // A pontokkal valamint az ismételt üzenetekkel nem kezdünk semmit (HACK miatt vannak küldve).
  String lData = String(data);
  if (lData.startsWith(".") || lData.equals(prevRadioData))
  {
    return;
  }
  prevRadioData = lData;

  Serial.println(lData.c_str());

  // Motorvezérlő parancs érkezett.
  if (lData.startsWith("m "))
  {
    strtok(lData.c_str(), " ");
    int posX = atoi(strtok(NULL, " "));
    int posY = atoi(strtok(NULL, " "));

    for (int i = 0; i < 4; i++) {
      analogWrite(MOTORS[i], MOTION_DATA[posX][posY][i] * MOTOR_SPEED);
    }

#if defined(NANO) && defined(USE_SERIAL)
    Serial.print("motor contol: ");
    Serial.print(posX);
    Serial.print(" ");
    Serial.print(posY);
    Serial.println();
    Serial.print("MOTION_DATA: [");
    Serial.print(MOTION_DATA[posX][posY][0]); Serial.print(",");
    Serial.print(MOTION_DATA[posX][posY][1]); Serial.print(",");
    Serial.print(MOTION_DATA[posX][posY][2]); Serial.print(",");
    Serial.print(MOTION_DATA[posX][posY][3]);
    Serial.print("]");
    Serial.println();
#endif

    return;
  }

  // Ismeretlen parancs.
#if defined(NANO) && defined(USE_SERIAL)
  Serial.print("Unknown command:");
  Serial.println(data);
#endif
}

// Üzenet küldése rádión keresztül.
void sendDataOnRadio(char *data)
{
  radio.stopListening();
  while (!radio.write(data, MAX_DATA_STRING_LENGTH));
  radio.powerUp();
  radio.startListening();
}
void sendDataOnRadio(String s)
{
  s.toCharArray(data_string, s.length() + 1);
  sendDataOnRadio(data_string);
  clearDataString();
}


void setup() {
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

  // Motorok.
  pinMode(MOTOR_LEFT_FORWARD, OUTPUT);
  pinMode(MOTOR_LEFT_BACKWARD, OUTPUT);
  pinMode(MOTOR_RIGHT_FORWARD, OUTPUT);
  pinMode(MOTOR_RIGHT_BACKWARD, OUTPUT);

  delay(500);
}

void loop() {
  // Rádió figyelése.
  if (radio.available())
  {
    radio.read(data_string, MAX_DATA_STRING_LENGTH);
    handleRadioData(data_string);
    clearDataString();
  }
}
