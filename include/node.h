#ifndef NODE_H
#define NODE_H

#include <Arduino.h>
#include "painlessMesh.h"
#include <ArduinoJson.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <Preferences.h>
#include <sys/time.h>
#include <vector>
#include <map>

// ==========================================
// Configurações da Rede Mesh
// ==========================================
#define MESH_PREFIX   "CI_Rede"  // Nome invisível da malha
#define MESH_PASSWORD "corredor" // Senha da malha
#define MESH_PORT     5555       // Porta de comunicação

// ==========================================
// Configurações do Infravermelho
// ==========================================
#define IR_SEND_PIN 4

// ==========================================
// Protótipos das Funções
// ==========================================
void mensagensRecebidas(uint32_t nodeId_de_quem_enviou, String &msg);
void enviarStatusCentral();
void checarAgendamentos();
void taskChecadorDeTempo(void *pvParameters);
void dispararCodigoSalvo(String nomeCodigo)

// ==========================================
// Variáveis
// ==========================================
struct Agendamento {
    String diaDaSemana;
    int hora;
    int minuto;
    String nomeCodigo;  // Ex: "Ligar", "22C"
    bool jaDisparou;    // A trava de segurança!
};

std::vector<Agendamento> listaAgendamentos;
std::map<String, String> mapaCodigosIR;

#endif // NODE_H