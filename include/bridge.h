#ifndef BRIDGE_H
#define BRIDGE_H

#include <Arduino.h>
#include "painlessMesh.h"
#include <ArduinoJson.h>    
#include <IRrecv.h>         
#include <IRutils.h>

// ==========================================
// Configurações da Rede Mesh
// ==========================================
#define MESH_PREFIX   "CI_Rede"
#define MESH_PASSWORD "corredor"
#define MESH_PORT     5555

// ==========================================
// Configurações do Infravermelho
// ==========================================
#define CAPTURE_BUFFER_SIZE 1024
#define TIMEOUT 70
#define IR_RECV_PIN 14

// ==========================================
// Protótipos das Funções
// ==========================================
void mensagemRecebidaDaMesh(uint32_t from, String &msg);
void mensagemRecebidaDoQt();
void checarReceptorIR();

#endif // BRIDGE_H