#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>

#define CE  4
#define CSN 5
#define led 3
#define on 2
#define NUM_SERVOS 8
#define SMOOTH 0
#define DELAY 0

RF24 radio(CE,CSN);

const byte address[6] = "00001";

Servo servos[NUM_SERVOS];
int vals[NUM_SERVOS+2];
int seq[100][NUM_SERVOS];
int cur[NUM_SERVOS];
int nex[NUM_SERVOS];
int counter = 0;
bool b1_cooldown, b2_cooldown, b3_cooldown;

void setup() {
  // put your setup code here, to run once:
  pinMode(led,OUTPUT);
  pinMode(on,OUTPUT);
  digitalWrite(on,HIGH);
  for (int i = 0; i < NUM_SERVOS; i++) servos[i].attach(i+6);
  for (int i = 0; i < NUM_SERVOS; i++) cur[i] = 0;
  Serial.begin(9600);
  radio.begin();
  radio.openReadingPipe(0,address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
  b1_cooldown = 0;
  b2_cooldown = 0;
  b3_cooldown = 0;
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(5);
  if (radio.available()) {
    digitalWrite(led,HIGH);
    radio.read(vals,sizeof(vals));
    for (int i = 0; i < NUM_SERVOS-1; i++) {
      Serial.print(vals[i]);
      Serial.print(" ");
      int deg = map(vals[i],0,1024,0,180);

      //Smoothing condition
      if (i == 1) {
        if (abs(deg-cur[i]) <= SMOOTH) continue;
      }
      else if (i == 0) {
        if (abs(deg-cur[i]) <= SMOOTH) continue;
      }
      else {
        if (abs(deg-cur[i+1]) <= SMOOTH) continue;
      }

      //Store next values
      if (i == 1) {
       //servos[i].write(deg);
       nex[i] = deg;
       //servos[i+1].write(180-deg);
       nex[i+1] = 180-deg;
      }
      else if (i == 0) {
       //servos[i].write(deg);
       nex[i] = deg;
      }
      else {
       //servos[i+1].write(deg);
       nex[i+1] = deg;
      }
    }
    Serial.println();

    //Write values to the servos
    while (true) {
      bool done = 1;
      for (int j = 0; j < NUM_SERVOS; j++) {
       if (cur[j] != nex[j]) {
          done = 0;
          int dir = (nex[j] - cur[j])/abs(nex[j] - cur[j]);
          servos[j].write(cur[j] + dir);
          cur[j] += dir;        
        }
      }
      if (done) break;
      delay(DELAY);
    }

    if (vals[NUM_SERVOS-1] == LOW) b1_cooldown = 0;
    if (vals[NUM_SERVOS] == LOW) b2_cooldown = 0;
    if (vals[NUM_SERVOS+1] == LOW) b3_cooldown = 0;
    
    if (vals[NUM_SERVOS-1] == HIGH && !b1_cooldown) {
      b1_cooldown = 1;
      store();
    }
    else if (vals[NUM_SERVOS] == HIGH && !b2_cooldown) {
      b2_cooldown = 1;
      play();
    }
    else if (vals[NUM_SERVOS+1] == HIGH && !b3_cooldown) {
      b3_cooldown = 1;
      clear();
    }
  }
  else {
    digitalWrite(led,LOW);
    Serial.println("No Radio Connection");
  }
}

void store() {
  Serial.println("Stored");
  for (int i = 0; i < NUM_SERVOS-1; i++) {
    int deg = map(vals[i],0,1024,0,180);
    if (i == 1) {
      seq[counter][i] = deg;
      seq[counter][i+1] = 180-deg;
    }
    else if (i == 0) {
      seq[counter][i] = deg;
    }
    else {
      seq[counter][i+1] = deg;
    }
  }
  counter++;
}

void play() {
  Serial.println("Play");
  for (int i = 0; i < NUM_SERVOS; i++) {
    seq[counter][i] = seq[0][i];
  }
  counter++;
  while (true) {
    for (int i = 0; i < counter-1; i++) {
      int temp[NUM_SERVOS];
      for (int j = 0; j < NUM_SERVOS; j++) {
        temp[j] = seq[i][j];
      }
      while (true) {
        if (radio.available()) radio.read(vals,sizeof(vals));
        if (vals[NUM_SERVOS+1] == HIGH && !b3_cooldown) {
          b3_cooldown = 1;
          clear();
          return;
        }
        bool done = 1;
        for (int j = 0; j < NUM_SERVOS; j++) {
          if (temp[j] != seq[i+1][j]) {
            done = 0;
            int dir = (seq[i+1][j] - temp[j])/abs(seq[i+1][j] - temp[j]);
            servos[j].write(temp[j] + dir);
            temp[j] += dir;
          }
        }
        if (done) break;
        delay(20);
      }
    }
  }
}

void clear() {
  Serial.println("Cleared");
  counter = 0;
}

