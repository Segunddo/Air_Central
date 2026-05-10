#include "painlessMesh.h"
#include <ArduinoJson.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

#define MESH_PREFIX   "CI_Rede" // Nome invisível da malha
#define MESH_PASSWORD "corredor"     // Senha da malha
#define MESH_PORT     5555                 // Porta de comunicação

painlessMesh mesh;

String myID = "CI101"; 

bool powerStatus = false;
int temperaturaAtual = 25;
int temperaturaAlvo = 25;

const uint16_t IR_SEND_PIN = 4;
IRsend irsend(IR_SEND_PIN);

void mensagensRecebidas(uint32_t nodeId_de_quem_enviou, String &msg) {
  StaticJsonDocument<256> filter;
  filter["command"] = true;
  filter["id"] = true;
  filter["new_id"]  = true;

  StaticJsonDocument<256> docBasico;
  deserializeJson(docBasico, msg, DeserializationOption::Filter(filter));

  String command = docBasico["command"];

   if (command == "Require_IDs") {
    Serial.println("A Central pediu meu ID. Respondendo...");
    JsonDocument docResposta;
    docResposta["command"] = "Resposta_ID";
    docResposta["id"] = myID;
    String respostaJSON;
    serializeJson(docResposta, respostaJSON);
    mesh.sendSingle(nodeId_de_quem_enviou, respostaJSON);
  }
  
  if (command == "Change_ID" && docBasico["id"] == myID) {
    String novoID = docBasico["new_id"].as<String>();

    if (novoID.length() > 0) {
        Serial.printf("Alterando ID de '%s' para '%s'\n", myID.c_str(), novoID.c_str());
        myID = novoID;

        // Confirma a mudança pra central
        JsonDocument docResposta;
        docResposta["command"] = "Resposta_ID";
        docResposta["id"]      = myID;
        String respostaJSON;
        serializeJson(docResposta, respostaJSON);
        mesh.sendSingle(nodeId_de_quem_enviou, respostaJSON);
    }
}

  if (command == "Dispatch" && docBasico["id"] == myID) {
    // Agora sim, usamos o buffer grande apenas se a mensagem for para MIM
    DynamicJsonDocument docRaw(2048); 
    DeserializationError erro = deserializeJson(docRaw, msg);
    
    if (erro) {
      Serial.println("Erro ao decodificar JSON RAW longo");
      return;
    }

    if (docRaw.containsKey("code")) {
      String rawStr = docRaw["code"].as<String>();
      
      int count = 1;
      for (int i = 0; i < rawStr.length(); i++) if (rawStr[i] == ',') count++;

      uint16_t* pRaw = new (std::nothrow) uint16_t[count]; 
      if (pRaw != nullptr) {
        int index = 0;
        int pos = 0;
        while (rawStr.indexOf(",", pos) != -1) {
          int nextPos = rawStr.indexOf(",", pos);
          pRaw[index++] = rawStr.substring(pos, nextPos).toInt();
          pos = nextPos + 1;
        }
        pRaw[index] = rawStr.substring(pos).toInt();

        irsend.sendRaw(pRaw, count, 38);
        yield();
        delete[] pRaw;
        Serial.printf("RAW enviado: %d pulsos\n", count);
      }
    }

    if (docRaw.containsKey("status")) powerStatus = (docRaw["status"] == "Ligado");
    if (docRaw.containsKey("temp")) temperaturaAlvo = docRaw["temp"].as<int>();
    
    enviarStatusCentral(); 
  }
}

void enviarStatusCentral() {
  JsonDocument docStatus;
  
  docStatus["command"] = "status_update"; 
  docStatus["id"] = myID;
  docStatus["status"] = powerStatus ? "Ligado" : "Desligado";
  docStatus["temp"] = temperaturaAlvo;

  String statusJSON;
  serializeJson(docStatus, statusJSON);

  mesh.sendBroadcast(statusJSON);
}

void setup() {
  Serial.begin(115200);
  mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT);
  mesh.setDebugMsgTypes(0);
  irsend.begin();
  mesh.onReceive(&mensagensRecebidas);
}

void loop() {
  mesh.update();
}