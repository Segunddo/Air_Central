#include "painlessMesh.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#define MESH_PREFIX   "CI_Rede"
#define MESH_PASSWORD "corredor"
#define MESH_PORT     5555

#define STATION_SSID     "Apto 402 ( TriLuz.Net )"
#define STATION_PASSWORD "est122326"

painlessMesh mesh;
WiFiUDP udp;

const int portaRecebimentoUDP = 8081;
const int portaEnvioQt = 8080;
IPAddress broadcastIP(192, 168, 7, 255);

bool udpIniciado = false;

void mensagemRecebidaDaMesh(uint32_t from, String &msg) {
  if (udpIniciado) {
    udp.beginPacket(broadcastIP, portaEnvioQt);
    udp.print(msg);
    udp.endPacket();
    Serial.printf("Mesh->Qt: %s\n", msg.c_str());
  }
}

void mensagemRecebidaDoQt() {
  if (!udpIniciado) return;
  int packetSize = udp.parsePacket();
  if (packetSize) {
    char packetBuffer[512];
    int len = udp.read(packetBuffer, 511);
    if (len > 0) packetBuffer[len] = 0;
    String mensagemQt = String(packetBuffer);
    Serial.println("Qt->Mesh: " + mensagemQt);
    mesh.sendBroadcast(mensagemQt);
  }
}

void novaConexao(uint32_t nodeId) {
    Serial.printf("--> NOVA CONEXÃO: Nó %u entrou na rede!\n", nodeId);
}

void mudancaNaRede() {
    Serial.printf("--> A malha mudou! Lista de nós agora: %s\n", mesh.subConnectionJson().c_str());
}

void setup() {
  Serial.begin(115200);
  delay(500);

  mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION);
  mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA, 11);
  mesh.stationManual(STATION_SSID, STATION_PASSWORD);
  mesh.setRoot(true);
  mesh.setContainsRoot(true);

  mesh.onReceive(&mensagemRecebidaDaMesh);
  mesh.onNewConnection(&novaConexao);
  mesh.onChangedConnections(&mudancaNaRede);

  Serial.println("Bridge iniciada, aguardando IP...");
}

void loop() {
  mesh.update();
  mensagemRecebidaDoQt();
}