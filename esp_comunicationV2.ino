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

uint16_t rawDataOn[] = {9002,4478,662,1644,664,540,664,542,664,544,664,542,662,544,662,544,662,542,664,542,664,542,664,1644,664,542,662,544,664,542,662,542,660,546,662,542,664,544,662,544,662,542,664,542,664,1644,664,542,664,542,664,542,664,544,662,542,664,542,664,1644,662,544,662,1644,664,542,662,544,662,1642,664,542,688,19958,664,542,664,540,662,544,662,542,664,540,664,542,664,542,664,542,664,1644,664,1640,666,542,662,542,662,544,662,544,660,542,664,542,664,542,664,540,662,544,662,542,664,542,662,542,662,544,664,540,664,542,662,542,662,542,664,542,662,1644,664,1644,664,1644,662,1644,664};

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
	  
	  if (docRecebido.containsKey("status")) {
		  String status = docRecebido["status"];
		  
		  // Atualiza o Power Status
		  if (status == "Ligado") {
			  powerStatus = true;
			  ligar();
		  } else if (status == "Desligado") {
			  powerStatus = false;
			  desligar();
          }
	  }
	  
	  if (docRecebido.containsKey("temp")) {
		  String temp = docRecebido["temp"];
		  
		  // Atualiza a Temperatura Alvo
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
  
  docStatus["command"] = "status_update"; // Identificador para o Qt saber o que é
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
  irsend.begin();
  mesh.onReceive(&mensagensRecebidas);

  Serial.println("\nESP iniciada e procurando a rede Mesh...");
}

void loop() {
  mesh.update();
}

void ligar(){
  irsend.sendRaw(rawDataOn, sizeof(rawDataOn) / sizeof(rawDataOn[0]), 38);
  Serial.println("Sinal de ligar enviado!\n");
  }

  void desligar(){
    irsend.sendRaw(rawDataOff, sizeof(rawDataOff) / sizeof(rawDataOff[0]), 38);
    Serial.println("Sinal de desligar enviado!\n");  
  }