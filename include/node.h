#ifndef NODE_H
#define NODE_H

#include <Arduino.h>
#include "painlessMesh.h"
#include <ArduinoJson.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <sys/time.h>
#include <vector>
#include <LittleFS.h>
#include <New>

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
// Relógio e Agendamentos
// ==========================================
#define FUSO_HORARIO "<-03>3"          // POSIX TZ de Brasília (UTC-3)
#define ARQUIVO_AGENDAMENTOS "/_agenda.cfg"

// ==========================================
// Protótipos das Funções
// ==========================================
void mensagensRecebidas(uint32_t nodeId_de_quem_enviou, String &msg);
void enviarStatusCentral();
void checarAgendamentos();
void dispararCodigoSalvo(String nomeCodigo);
void dispararStringRaw(const char* rawStr);
void pedirHoraSeNecessario();
void carregarAgendamentos();

// ==========================================
// Variáveis
// ==========================================
struct Agendamento {
    int8_t diaDaSemana; // 0 = Dom, 1 = Seg, 2 = Ter... até 6 = Sab
    int8_t hora;        // 0 a 23
    int8_t minuto;      // 0 a 59
    String nomeCodigo;  // O nome do arquivo na Flash (Ex: "Ligar")
    bool jaDisparou;    // Trava para não atirar 60 vezes no mesmo minuto
};

void salvarAgendamento(const Agendamento &agenda);

#endif // NODE_H