#include "painlessMesh.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#define MESH_PREFIX   "CI_Rede"
#define MESH_PASSWORD "corredor"
#define MESH_PORT     5555

painlessMesh mesh;

// Configurações do Roteador Wi-Fi
#define STATION_SSID     "NOME_DO_SEU_WIFI"
#define STATION_PASSWORD "SENHA_DO_SEU_WIFI"

WiFiUDP udp;
const int portaRecebimentoUDP = 8081; // Porta onde a Bridge escuta os comandos do Qt
const int portaEnvioQt = 8080;        // Porta que o seu Qt está escutando (Ajuste se necessário)

IPAddress broadcastIP(255, 255, 255, 255); 

// =======================================================================
// Função: O que fazer quando uma mensagem vem da MESH (ESPs -> Bridge -> Qt)
// =======================================================================
void mensagemRecebidaDaMesh(uint32_t from, String &msg) {
  Serial.printf("MESH -> UDP | Recebido do nó %u: %s\n", from, msg.c_str());

  udp.beginPacket(broadcastIP, portaEnvioQt);
  udp.print(msg);
  udp.endPacket();
}

// =======================================================================
// Verifica se chegou alguma coisa do QT (Qt -> Bridge -> ESPs)
// =======================================================================
void mensagemRecebidaDoQt() {
  int packetSize = udp.parsePacket();
  
  if (packetSize) {
    char packetBuffer[512];
    int len = udp.read(packetBuffer, 511);
    
    if (len > 0) {
      packetBuffer[len] = 0;
    }

    String mensagemQt = String(packetBuffer);
    Serial.println("UDP -> MESH | Recebido do Qt: " + mensagemQt);

    // Repassa a string JSON recebida para TODA a rede Mesh
    mesh.sendBroadcast(mensagemQt);
  }
}

void setup() {
  Serial.begin(115200);

  mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT);
  
  mesh.onReceive(&mensagemRecebidaDaMesh);

  mesh.stationManual(STATION_SSID, STATION_PASSWORD);

  udp.begin(portaRecebimentoUDP);

  Serial.println("\n=================================");
  Serial.println("ESP BRIDGE INICIADA");
  Serial.println("Aguardando conexões...");
  Serial.println("=================================");
}

void loop() {
  mesh.update();
  
  // Fica veifiando se o Qt enviou algo
  mensagemRecebidaDoQt();
}