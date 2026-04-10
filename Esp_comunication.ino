#include "painlessMesh.h"
#include <ArduinoJson.h>

// Configurações da Rede Mesh (Todas as ESPs devem ter as mesmas credenciais)
#define MESH_PREFIX   "CI_Rede" // Nome invisível da malha
#define MESH_PASSWORD "corredor"     // Senha da malha
#define MESH_PORT     5555                 // Porta de comunicação

painlessMesh mesh;

// Identificação deste módulo específico
String myID = "CI101"; 

// Variáveis de controle
bool powerStatus = false;
int temperaturaAtual = 25;
int temperaturaAlvo = 25;

// =========================================================================
// Função que é chamada AUTOMATICAMENTE sempre que um pacote chega na Mesh
// =========================================================================
void mensagensRecebidas(uint32_t nodeId_de_quem_enviou, String &msg) {
  
  Serial.printf("Mensagem recebida do nó %u: %s\n", nodeId_de_quem_enviou, msg.c_str());

  JsonDocument docRecebido;
  DeserializationError erro = deserializeJson(docRecebido, msg);

  if (erro) {
    Serial.println("Erro: Mensagem não é um JSON válido.");
    return;
  }

  // Verifica o "comando" da mensagem
  String command = docRecebido["command"];

  // --- TRATANDO A REQUISIÇÃO DE IDs ---
  if (command == "Require_IDs") {
    Serial.println("A Central pediu meu ID. Respondendo...");

    JsonDocument docResposta;
    docResposta["command"] = "Resposta_ID";
    docResposta["id"] = myID;

    String respostaJSON;
    serializeJson(docResposta, respostaJSON);

    mesh.sendSingle(nodeId_de_quem_enviou, respostaJSON);
  }

  // --- TRATANDO COMANDOS (Ligar, Mudar Temperatura, etc) ---
  else if (command == "Dispatch") {
    String destino = docRecebido["id"];

    // Só executa se o comando for pra essa ESP
    if (destino == myID) {
      Serial.println("Comando recebido para MIM!");

      String status = docRecebido["status"];
      String temp = docRecebido["temp"];

      // Atualiza o Power Status
      if (status != "-1") {
        if (status == "Ligado") {
          powerStatus = true;
        } else if (status == "Desligado") {
          powerStatus = false;
        }
      }

      // Atualiza a Temperatura Alvo
      if (temp != "-1") {
        temperaturaAlvo = temp.toInt();
      }

      Serial.println("--- Novo Estado ---");
      Serial.print("Status: "); Serial.println(powerStatus ? "Ligado" : "Desligado");
      Serial.print("Temp Alvo: "); Serial.println(temperaturaAlvo);
      
      enviarStatusCentral(); 
    } else {
      Serial.println("Comando não é pra mim. Ignorando.");
    }
  }
}

void enviarStatusCentral() {
  
  JsonDocument docStatus;
  
  docStatus["tipo"] = "Status_Update"; // Identificador para o Qt saber o que é
  docStatus["id"] = myID;
  docStatus["status"] = powerStatus ? "Ligado" : "Desligado";
  docStatus["temp"] = temperaturaAlvo;

  String statusJSON;
  serializeJson(docStatus, statusJSON);

  mesh.sendBroadcast(statusJSON);
  
  Serial.println("Status atualizado enviado para a rede:");
  Serial.println(statusJSON);
}

void setup() {
  Serial.begin(115200);

  // Inicia tudo
  mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT);
  
  mesh.onReceive(&mensagensRecebidas);

  Serial.println("\nESP iniciada e procurando a rede Mesh...");
}

void loop() {
  mesh.update();
}