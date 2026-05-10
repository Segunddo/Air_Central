#include "painlessMesh.h"
#include <ArduinoJson.h>    
#include <IRrecv.h>         
#include <IRutils.h>

#define MESH_PREFIX   "CI_Rede"
#define MESH_PASSWORD "corredor"
#define MESH_PORT     5555

painlessMesh mesh;

// Configurações do buffer para sinais longos de Ar-Condicionado
const uint16_t CAPTURE_BUFFER_SIZE = 1024; 
const uint8_t TIMEOUT = 70;

// === Configurações do IR ===
const uint16_t IR_RECV_PIN = 14;
IRrecv irrecv(IR_RECV_PIN, CAPTURE_BUFFER_SIZE, TIMEOUT, true);
decode_results results;
bool modoLeituraIR = false;

// =======================================================================
// MESH -> QT (Via Cabo)
// =======================================================================
void mensagemRecebidaDaMesh(uint32_t from, String &msg) {
  Serial.println(msg);
}

// =======================================================================
// QT -> MESH ou BRIDGE
// =======================================================================
void mensagemRecebidaDoQt() {
  if (Serial.available()) {
    String mensagemQt = Serial.readStringUntil('\n');
    mensagemQt.trim();
    if (mensagemQt.length() > 0) {
      DynamicJsonDocument doc(512);
      DeserializationError error = deserializeJson(doc, mensagemQt);
      if (!error) {
        String command = doc["command"].as<String>();
        if (command == "Require_IR") {
          modoLeituraIR = true;
          irrecv.enableIRIn();
          Serial.println("{\"log\":\"Aguardando sinal IR...\"}");
        }
        else {
          mesh.sendBroadcast(mensagemQt);
        }
      }
    }
  }
}

// =======================================================================
// Leitura do Infravermelho (APENAS RAW)
// =======================================================================
void checarReceptorIR() {
  if (modoLeituraIR) {
    if (irrecv.decode(&results)) {
      
      // Criamos uma String para armazenar os pulsos separados por vírgula
      String rawString = "";
      uint16_t count = results.rawlen - 1;

      for (uint16_t i = 1; i <= count; i++) {
        rawString += String(results.rawbuf[i] * RAWTICK);
        if (i < count) rawString += ",";
        yield(); // Evita crash no ESP8266 durante loops longos
      }

      // Monta o JSON de resposta para o Qt
      // Nota: Aumentamos o tamanho do doc para comportar a string longa
      DynamicJsonDocument docResposta(2048); 
      docResposta["command"] = "new_code";
      docResposta["type"] = "RAW";
      docResposta["code"] = rawString;
      docResposta["size"] = count;

      String respostaJSON;
      serializeJson(docResposta, respostaJSON);
      
      // Envia o JSON final via USB
      Serial.println(respostaJSON);

      modoLeituraIR = false;
      irrecv.disableIRIn();
      irrecv.resume();
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  mesh.setDebugMsgTypes(0);
  mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT);
  mesh.onReceive(&mensagemRecebidaDaMesh);
}

void loop() {
  mesh.update();            
  mensagemRecebidaDoQt();    
  checarReceptorIR();        
}