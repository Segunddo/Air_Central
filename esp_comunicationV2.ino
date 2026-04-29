#include "painlessMesh.h"
#include <ArduinoJson.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

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

uint16_t rawDataOn[] = {3358, 9854, 464, 1610, 492, 548, 492, 538,
  492, 538, 492, 1568, 492, 548, 492, 538, 510, 536, 490, 540, 490,
  538, 492, 540, 490, 548, 490, 540, 490, 546, 486, 538, 492, 546,
  492, 1576, 486, 546, 492, 550, 488, 540, 490, 1572, 488, 540,
  488, 542, 488, 1572, 490, 530, 530, 506, 516, 548, 490, 1572, 488};

uint16_t rawDataOff[] = {3112, 9872, 510, 1592, 480, 526, 508, 568,
 460, 550, 484, 1594, 482, 546, 492, 554, 484, 546, 514, 1536, 510,
  1560, 510, 552, 480, 550, 480, 548, 482, 548, 480, 552, 480, 566,
   462, 544, 490, 546, 484, 548, 482, 552, 478, 566, 462, 1616, 462,
    554, 478, 1580, 510, 506, 540, 528, 486, 550, 482, 1596, 480};  

const uint16_t IR_SEND_PIN = 4;
IRsend irsend(IR_SEND_PIN);

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
          ligar();
        } else if (status == "Desligado") {
          powerStatus = false;
          desligar();
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
  
  docStatus["command"] = "Status_Update"; // Identificador para o Qt saber o que é
  docStatus["id"] = myID;
  docStatus["status"] = powerStatus ? "Ligado" : "Desligado";
  docStatus["temp"] = temperaturaAlvo;

  String statusJSON;
  serializeJson(docStatus, statusJSON);

  mesh.sendBroadcast(statusJSON);
  
  Serial.println("Status atualizado enviado para a rede:");
  Serial.println(statusJSON);
}

void novaConexao(uint32_t nodeId) {
    Serial.printf("--> NOVA CONEXÃO: Nó %u entrou na rede!\n", nodeId);
}

void mudancaNaRede() {
    Serial.printf("--> A malha mudou! Lista de nós agora: %s\n", mesh.subConnectionJson().c_str());
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION);
  mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT); // sem canal fixo
  mesh.setContainsRoot(true);

  irsend.begin();
  mesh.onReceive(&mensagensRecebidas);
  mesh.onNewConnection(&novaConexao);
  mesh.onChangedConnections(&mudancaNaRede);

  Serial.println("\nESP iniciada e procurando a rede Mesh...");
}

void loop() {
  mesh.update();

  // Printa a cada 3 segundos para confirmar que está vivo
  static unsigned long ultimo = 0;
  if (millis() - ultimo > 5000) {
    Serial.printf("Vivo. Nós conectados: %d | NodeId: %u\n", 
                  mesh.getNodeList().size(), 
                  mesh.getNodeId());
    ultimo = millis();
  }
}

void ligar(){
  irsend.sendRaw(rawDataOn, sizeof(rawDataOn) / sizeof(rawDataOn[0]), 38);
  Serial.println("Sinal de ligar enviado!\n");
}

void desligar(){
  irsend.sendRaw(rawDataOff, sizeof(rawDataOff) / sizeof(rawDataOff[0]), 38);
  Serial.println("Sinal de desligar enviado!\n");  
}