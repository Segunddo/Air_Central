#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

const uint16_t IR_RECV_PIN = 14; // D5 no NodeMCU

// 1. AUMENTANDO O BUFFER
// Aumentamos para 1024 para capturar com folga controles de Ar-Condicionado ou som
const uint16_t CAPTURE_BUFFER_SIZE = 1024; 
const uint8_t TIMEOUT = 70; // Tempo de espera para finalizar a captura (em milissegundos)

// Inicia o receptor com as novas configurações de buffer e timeout
IRrecv irrecv(IR_RECV_PIN, CAPTURE_BUFFER_SIZE, TIMEOUT, true);
decode_results results;

void setup() {
  Serial.begin(115200);
  irrecv.enableIRIn();

  Serial.println("=== Receptor IR Iniciado ===");
  Serial.println("Aponte o controle e pressione qualquer botao...");
}

void loop() {
  if (irrecv.decode(&results)) {
    // Informações Básicas e Hexadecimal
    Serial.println("=== Sinal Recebido! ===");
    Serial.print("Protocolo : ");
    Serial.println(typeToString(results.decode_type));
    
    // Exibe o Valor Hexadecimal
    Serial.print("Valor Hex  : 0x");
    Serial.println(uint64ToString(results.value, 16)); 
    
    // Exibe a quantidade de bits do protocolo
    Serial.print("Bits       : ");
    Serial.println(results.bits);
    
    // Formatação para o Array Raw
    uint16_t count = results.rawlen - 1;
    Serial.print("Total pulsos: ");
    Serial.println(count);
    
    Serial.print("uint16_t rawData[");
    Serial.print(count);
    Serial.println("] = {");

    for (uint16_t i = 1; i <= count; i++) {
      Serial.print(results.rawbuf[i] * RAWTICK);
      if (i < count) Serial.print(", ");
      
      // Quebra de linha a cada 15 valores para deixar o monitor serial mais legível
      if (i % 15 == 0) Serial.println(); 
      
      // 2. YIELD
      // Dá um respiro ao processador do ESP8266 evitando o travamento (WDT Reset)
      yield(); 
    }

    Serial.println();
    Serial.println("};");
    Serial.println("------------------------\n");

    irrecv.resume(); // Prepara para o próximo sinal
  }
}