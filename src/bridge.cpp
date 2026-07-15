#include "bridge.h"

// Inicialização dos objetos globais
painlessMesh mesh;
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
      // CORREÇÃO: Filtro estático para decodificar apenas o "command"
      // Evita carregar o array "raw" gigante na memória RAM da Bridge!
      StaticJsonDocument<256> filter;
      filter["command"] = true;

      StaticJsonDocument<256> doc;
      DeserializationError error = deserializeJson(doc, mensagemQt, DeserializationOption::Filter(filter));
      
      if (!error) {
        String command = doc["command"].as<String>();
        if (command == "Require_IR") {
          modoLeituraIR = true;
          irrecv.enableIRIn();
          Serial.println("{\"log\":\"Aguardando sinal IR...\"}");
        }
        else {
          // Repassa o JSON completo intacto para a rede Mesh
          mesh.sendBroadcast(mensagemQt);
        }
      } else {
        Serial.printf("{\"log\":\"Erro decodificacao Bridge: %s\"}\n", error.c_str());
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
      
      String rawString = "";
      uint16_t count = results.rawlen - 1;

      for (uint16_t i = 1; i <= count; i++) {
        rawString += String(results.rawbuf[i] * RAWTICK);
        if (i < count) rawString += ",";
        yield(); // Evita crash no ESP8266 durante loops longos
      }

      DynamicJsonDocument docResposta(2048); 
      docResposta["command"] = "new_code";
      docResposta["type"] = "RAW";
      docResposta["code"] = rawString;
      docResposta["size"] = count;

      String respostaJSON;
      serializeJson(docResposta, respostaJSON);
      
      Serial.println(respostaJSON);

      modoLeituraIR = false;
      irrecv.disableIRIn();
      irrecv.resume();
    }
  }
}

// =======================================================================
// Ciclo de Vida do Microcontrolador
// =======================================================================
void setup() {
  // CORREÇÃO: Buffer serial aumentado para receber strings longas do Qt
  Serial.setRxBufferSize(4096);
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