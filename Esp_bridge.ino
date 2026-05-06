#include "painlessMesh.h"
#include <ArduinoJson.h>    
#include <IRrecv.h>         
#include <IRutils.h>

#define MESH_PREFIX   "CI_Rede"
#define MESH_PASSWORD "corredor"
#define MESH_PORT     5555

painlessMesh mesh;

// === Configurações do IR ===
const uint16_t kRecvPin = 14; 
IRrecv irrecv(kRecvPin);
decode_results results;
bool modoLeituraIR = false;

// =======================================================================
// MESH -> QT (Via Cabo)
// =======================================================================
void mensagemRecebidaDaMesh(uint32_t from, String &msg) {
  // Envia a string JSON recebida da rede direto para o PC
  Serial.println(msg); 
}

// =======================================================================
// QT -> MESH ou QT -> BRIDGE (Interceptação)
// =======================================================================
void mensagemRecebidaDoQt() {
  if (Serial.available()) {
    
    // Lê o JSON até a quebra de linha
    String mensagemQt = Serial.readStringUntil('\n');
    mensagemQt.trim();

    if (mensagemQt.length() > 0) {
      DynamicJsonDocument doc(512);
      DeserializationError error = deserializeJson(doc, mensagemQt);

      if (!error) {
        String command = doc["command"].as<String>();

        // Se o comando for para a PRÓPRIA Bridge ler o controle:
        if (command == "Require_IR") {
          modoLeituraIR = true;
          irrecv.enableIRIn(); 
        } 
        // Repassa para os outros nós da Mesh
        else {
          mesh.sendBroadcast(mensagemQt);
        }
      }
    }
  }
}

// =======================================================================
// Leitura do Infravermelho
// =======================================================================
void checarReceptorIR() {
  if (modoLeituraIR) {
    if (irrecv.decode(&results)) {
      String codigoHex = "0x" + String(results.value, HEX);
      codigoHex.toUpperCase();

      // Monta a resposta JSON para o Qt
      DynamicJsonDocument doc(256);
      doc["command"] = "new_code";
      doc["code"] = codigoHex;
      
      String respostaJSON;
      serializeJson(doc, respostaJSON);

      // Envia o JSON do código lido direto para o cabo USB
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