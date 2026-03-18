#include <IRremoteESP8266.h>
#include <IRsend.h>

const uint16_t kIrLed = D2;

IRsend irsend(kIrLed);

// seu RAW capturado
uint16_t rawData[73] = {
  8998, 4512, 628, 1678, 630, 576, 628, 576,
  628, 1680, 628, 1678, 630, 1676, 630, 578,
  628, 576, 628, 576, 628, 1678, 630, 576,
  628, 1678, 628, 576, 628, 578, 628, 576,
  630, 576, 628, 578, 628, 578, 626, 578,
  628, 576, 630, 576, 628, 1678, 628, 1678,
  628, 576, 628, 576, 630, 576, 628, 578,
  628, 578, 628, 1680, 628, 576, 628, 1678,
  628, 576, 626, 580, 628, 1680, 626, 580,
  630
};

void setup() {
  Serial.begin(115200);
  irsend.begin();
}

void loop() {

  Serial.println("Enviando RAW...");

  // envia várias vezes pra compensar sinal fraco
  for (int i = 0; i < 3; i++) {
    irsend.sendRaw(rawData, 73, 38); // 38 kHz
    delay(50);
  }

  Serial.println("Enviado!");

  delay(10000);
}