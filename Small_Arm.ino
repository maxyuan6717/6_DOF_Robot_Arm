#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define CE  5
#define CSN 6
#define b1 11
#define b2 12
#define b3 7
#define led1 8
#define led2 9
#define led3 10
#define NUM_SERVOS 8

RF24 radio(CE,CSN);

const byte address[6] = "00001";

int pins[NUM_SERVOS-1] = {A7,A6,A5,A4,A3,A2,A1};
int vals[NUM_SERVOS+2];

void setup() {
  // put your setup code here, to run once:
  for (int i = 0; i < NUM_SERVOS-1; i++) pinMode(pins[i],INPUT);
  pinMode(b1,INPUT);
  pinMode(b2,INPUT);
  pinMode(b3,INPUT);
  pinMode(led1,OUTPUT);
  pinMode(led2,OUTPUT);
  pinMode(led3,OUTPUT);
  Serial.begin(9600);
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(5);
  for (int i = 0; i < NUM_SERVOS-1; i++) {
    if (i == 4) vals[i] = analogRead(pins[i]);
    else vals[i] = 1023 - analogRead(pins[i]);
    Serial.print(vals[i]);
    Serial.print(" ");
  }
  Serial.println();
  vals[NUM_SERVOS-1] = digitalRead(b1);
  vals[NUM_SERVOS] = digitalRead(b2);
  //if (vals[8] == HIGH) Serial.print("BUTTON");
  vals[NUM_SERVOS+1] = digitalRead(b3);
  digitalWrite(led1,vals[NUM_SERVOS-1]);
  digitalWrite(led2,vals[NUM_SERVOS]);
  digitalWrite(led3,vals[NUM_SERVOS+1]);
  radio.write(vals,sizeof(vals));
}
