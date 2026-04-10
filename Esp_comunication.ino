#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

const char* ssid = ""; //nome da rede do ci
const char* password = ""; //senha da rede

WiFiUDP udp;

const int portaRecebimento = 8081;
const int portaResposta = 8080;

IPAddress broadcastIP(255,255,255,255);

String myID = "CI101"; //id para cada esp

// variaveis a serem alteradas

bool powerStatus;
int temperaturaAtual;
int temperaturaAlvo;

void setup() {

  Serial.begin(115200);

  WiFi.begin(ssid, password); //conectando na rede do ci

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
	
  // Verifica se é a mensagem de requisição de IDs
  if (msg.startsWith("Require_IDs")) {
    
    // Verifica se o ID já está dentro dessa string
    // Se retornar -1, significa que meu ID ainda NÃO está na lista
    if (msg.indexOf(myID) == -1) {
      
      Serial.println("Recebi pedido de IDs. Adicionando o meu...");
      
      // Adiciona o meu ID no final da string separando por vírgula
      String novaMensagem = msg + "," + myID;
      
      Serial.println("Reenviando string atualizada: " + novaMensagem);
      
      // Reenvia para a rede (a Central e as outras ESPs vão ouvir)
      reenviarIds(novaMensagem);

    } else {
      Serial.println("Meu ID ja esta na lista. Ignorando pacote para evitar loop.");
    }
    
    return; // Encerra a função para não executar o código normal abaixo
  }

  // Se não for "Require_IDs", o código segue para o fluxo normal (parser com vírgulas)
  int p1 = msg.indexOf(',');
  int p2 = msg.indexOf(',', p1+1);
  int p3 = msg.indexOf(',', p2+1);

  if (p1 == -1 || p2 == -1 || p3 == -1) {
    Serial.println("Formato de mensagem invalido ou desconhecido.");
    return;
  }

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

    if(status == "Ligado")
      powerStatus = true;
    else
      powerStatus = false;
  }

  if(temp != "-1")
    temperaturaAlvo = temp.toInt();

  Serial.println("Estado atualizado:");
  Serial.println(powerStatus);
  Serial.println(temperaturaAlvo);
}

void enviarIdCentral(){

  String resposta = myID;

  udp.beginPacket(broadcastIP, portaResposta);
  udp.print(resposta);
  udp.endPacket();

  Serial.println("Id enviado:");
  Serial.println(resposta);
}

void reenviarIds(String msg){

  udp.beginPacket(broadcastIP, portaRecebimento);
  udp.print(msg);
  udp.endPacket();

  Serial.println("Requisição enviada");
  Serial.println(msg);
}
