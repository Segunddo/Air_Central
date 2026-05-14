#include "painlessMesh.h"
#include <ArduinoJson.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <Preferences.h>

#define MESH_PREFIX   "CI_Rede" // Nome invisível da malha
#define MESH_PASSWORD "corredor"     // Senha da malha
#define MESH_PORT     5555                 // Porta de comunicação

painlessMesh mesh;

Preferences preferences;
String myID; 

bool powerStatus = false;
int temperaturaAtual = 25;
int temperaturaAlvo = 25;

const uint16_t IR_SEND_PIN = 4;
IRsend irsend(IR_SEND_PIN);

void mensagensRecebidas(uint32_t nodeId_de_quem_enviou, String &msg) {
  JsonDocument filter;
  filter["command"] = true;
  filter["id"] = true;
  filter["new_id"]  = true;

  JsonDocument docBasico;
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

        // Muda a nível de flash
        preferences.putString("nodeID", myID);

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
    // Aumentamos o buffer para evitar o estouro com a cópia da string longa
    JsonDocument docRaw; 
    DeserializationError erro = deserializeJson(docRaw, msg);
    
    if (erro) {
      Serial.print("Erro ao decodificar JSON RAW longo: ");
      Serial.println(erro.c_str()); // Agora vai te dizer o motivo exato do erro!
      return;
    }

    if (docRaw.containsKey("code")) {
      // Pegamos como const char* para não criar cópias pesadas da classe String
      const char* rawStr = docRaw["code"]; 
      
      int count = 1;
      for (int i = 0; rawStr[i] != '\0'; i++) {
        if (rawStr[i] == ',') count++;
      }

      uint16_t* pRaw = new (std::nothrow) uint16_t[count]; 
      if (pRaw != nullptr) {
        int index = 0;
        int currentVal = 0;
        
        // Algoritmo de extração ultrarrápido (não fragmenta a heap)
        for (int i = 0; rawStr[i] != '\0'; i++) {
          if (rawStr[i] == ',') {
            pRaw[index++] = currentVal;
            currentVal = 0; // Reseta para o próximo número
          } else if (isDigit(rawStr[i])) {
            // Constrói o número inteiro dígito por dígito
            currentVal = (currentVal * 10) + (rawStr[i] - '0');
          }
        }
        pRaw[index] = currentVal; // Adiciona o último valor da lista

        irsend.sendRaw(pRaw, count, 38);
        yield();
        delete[] pRaw;
      } else {
        Serial.println("ERRO CRÍTICO: Sem memória (Heap) para criar o array de pulsos!");
      }
    }

    if (docRaw.containsKey("status")) powerStatus = (docRaw["status"] == "Ligado");
    if (docRaw.containsKey("temp")) temperaturaAlvo = docRaw["temp"].as<int>();
    
    enviarStatusCentral(); 
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
  preferences.begin("config", false);
  myID = preferences.getString("nodeID", "CI000");

  Serial.begin(115200);
  mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT);
  mesh.setDebugMsgTypes(0);
  irsend.begin();
  mesh.onReceive(&mensagensRecebidas);
}

void loop() {
  mesh.update();
}