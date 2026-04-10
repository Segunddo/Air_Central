#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

const uint16_t kRecvPin = D5;  // pino do receptor

// Aumenta o tamanho do buffer
const uint16_t kCaptureBufferSize = 1024; 
const uint8_t kTimeout = 50;

IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
decode_results results;

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("Iniciando receptor IR...");
  irrecv.enableIRIn();  // inicia o receptor
}

void loop() {
  if (irrecv.decode(&results)) {

    Serial.println("=== SINAL RECEBIDO ===");

    // Mostra o tipo de protocolo (NEC, SONY, etc.)
    Serial.print("Protocolo: ");
    Serial.println(typeToString(results.decode_type));

    // Mostra o valor HEX (às vezes não funciona bem pra ar-condicionado)
    Serial.print("HEX: ");
    Serial.println(resultToHexidecimal(&results));

    // MOSTRA O RAW (ESSENCIAL para ar-condicionado)
    Serial.println("RAW:");
    Serial.println(resultToSourceCode(&results));

    Serial.println("======================\n");

    irrecv.resume();  // pronto pra próxima leitura
  }
}