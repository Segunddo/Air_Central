#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

const char* ssid = "Apto 402 ( TriLuz.Net )";
const char* password = "est122326";

WiFiUDP udp;

const int portaRecebimento = 8081;
const int portaResposta = 8080;

IPAddress broadcastIP(192,168,7,255);

String myID = "CI101";

// variaveis a serem alteradas
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

void setup() {

  Serial.begin(115200);
  irsend.begin();
  WiFi.begin(ssid, password);

  Serial.print("Conectando");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi conectado");
  Serial.println(WiFi.localIP());

  udp.begin(portaRecebimento);
}

void loop() {

  int packetSize = udp.parsePacket();

  if(packetSize) {

    char packetBuffer[255];

    int len = udp.read(packetBuffer, 255);

    if(len > 0)
      packetBuffer[len] = 0;

    String mensagem = String(packetBuffer);

    Serial.println("Mensagem recebida:");
    Serial.println(mensagem);

    tratarMensagem(mensagem);
  }
}

void tratarMensagem(String msg){

  int p1 = msg.indexOf(',');
  int p2 = msg.indexOf(',', p1+1);
  int p3 = msg.indexOf(',', p2+1);

  String idDestino = msg.substring(0, p1);
  String status = msg.substring(p1+1, p2);
  String temp = msg.substring(p2+1, p3);
  int ttl = msg.substring(p3+1).toInt();

  Serial.println("Destino: " + idDestino);
  Serial.println("TTL: " + String(ttl));

  if(idDestino == myID){

    Serial.println("Comando é para mim!");
    executarComando(status, temp);

  } 
  else {

    ttl--;

    if(ttl > 0){

      String novaMensagem =
        idDestino + "," +
        status + "," +
        temp + "," +
        String(ttl);

      Serial.println("Reenviando pacote:");
      Serial.println(novaMensagem);

      udp.beginPacket(broadcastIP, portaRecebimento);
      udp.print(novaMensagem);
      udp.endPacket();

    } else {

      Serial.println("TTL chegou a zero. Pacote descartado.");
    }
  }
}

void executarComando(String status, String temp){

  if(status != "-1"){

    if(status == "Ligado"){
      powerStatus = true;
      ligar();
    }else{
      powerStatus = false;
      desligar(); 
    }
  }

  if(temp != "-1")
    temperaturaAlvo = temp.toInt();

  Serial.println("Estado atualizado:");
  Serial.println(powerStatus);
  Serial.println(temperaturaAlvo);

  enviarStatusCentral();
}

void enviarStatusCentral(){

  String resposta = myID + "," +
                    (powerStatus ? "Ligado" : "Desligado") + "," +
                    String(temperaturaAtual);

  udp.beginPacket(broadcastIP, portaResposta);
  udp.print(resposta);
  udp.endPacket();

  Serial.println("Status enviado:");
  Serial.println(resposta);
}

void ligar(){
  irsend.sendRaw(rawDataOn, sizeof(rawDataOn) / sizeof(rawDataOn[0]), 38);
  Serial.println("Sinal de ligar enviado!\n");
  }

  void desligar(){
    irsend.sendRaw(rawDataOff, sizeof(rawDataOff) / sizeof(rawDataOff[0]), 38);
    Serial.println("Sinal de desligar enviado!\n");  
  }