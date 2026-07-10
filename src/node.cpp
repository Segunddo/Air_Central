#include "node.h"

// Inicialização dos objetos globais
painlessMesh mesh;
Scheduler userScheduler;
IRsend irsend(IR_SEND_PIN);

std::vector<Agendamento> listaAgendamentos;

String myID;
bool powerStatus = false;
int temperaturaAtual = 25;
int temperaturaAlvo = 25;
Task tarefaChecagem(5000, TASK_FOREVER, &checarAgendamentos);

// =======================================================================
// Envio de Status para a Central
// =======================================================================
void enviarStatusCentral() {
  JsonDocument docStatus;
  
  docStatus["command"] = "status_update"; 
  docStatus["id"] = myID;
  docStatus["status"] = powerStatus ? "Ligado" : "Desligado";
  docStatus["temp"] = temperaturaAlvo;

  String statusJSON;
  serializeJson(docStatus, statusJSON);

  mesh.sendBroadcast(statusJSON);
}

// =======================================================================
// Recepção e Processamento de Mensagens
// =======================================================================
void mensagensRecebidas(uint32_t nodeId_de_quem_enviou, String &msg) {
  JsonDocument filter;
  filter["command"] = true;
  filter["id"] = true;
  filter["new_id"]  = true;

  JsonDocument docBasico;
  deserializeJson(docBasico, msg, DeserializationOption::Filter(filter));

  String command = docBasico["command"];

  // --- TRATAMENTO: REQUISIÇÃO DE ID ---
  if (command == "Require_IDs") {
    Serial.println("A Central pediu meu ID. Respondendo...");
    JsonDocument docResposta;
    docResposta["command"] = "Resposta_ID";
    docResposta["id"] = myID;
    String respostaJSON;
    serializeJson(docResposta, respostaJSON);
    mesh.sendSingle(nodeId_de_quem_enviou, respostaJSON);
  }
  
  /// --- TRATAMENTO: MUDANÇA DE ID ---
  if (command == "Change_ID" && docBasico["id"] == myID) {
    String novoID = docBasico["new_id"].as<String>();

    if (novoID.length() > 0) {
        Serial.printf("Alterando ID de '%s' para '%s'\n", myID.c_str(), novoID.c_str());
        myID = novoID;

        // LÓGICA DE SALVAR O ID
        File f = LittleFS.open("/nodeID.txt", "w");
        if (f) {
            f.print(myID);
            f.close();
        }

        // Confirma a mudança pra central
        JsonDocument docResposta;
        docResposta["command"] = "Resposta_ID";
        docResposta["id"]      = myID;
        String respostaJSON;
        serializeJson(docResposta, respostaJSON);
        mesh.sendSingle(nodeId_de_quem_enviou, respostaJSON);
    }
  }

  // --- TRATAMENTO: SINCRONIZAÇÃO DE TEMPO ---
  if (command == "Sync_Time") {
    // Pega o Timestamp Unix (em segundos) que veio do Qt
    time_t epochTime = docBasico["timestamp"].as<long>(); 

    // Configura o relógio interno do sistema operacional (RTC)
    struct timeval tv;
    tv.tv_sec = epochTime;
    tv.tv_usec = 0;
    settimeofday(&tv, NULL); 

    Serial.println("Relógio interno atualizado com sucesso via Mesh!");
  }

  // --- TRATAMENTO: RECEBER AGENDAMENTO ---
  if (command == "Add_Schedule" && docBasico["id"] == myID) {
      String diaStr = docBasico["dia"].as<String>();
      String horaStr = docBasico["hora"].as<String>(); // Ex: "14:30"
      String codigo = docBasico["code"].as<String>();

      // Quebra a string "14:30" no caractere ":"
      int separador = horaStr.indexOf(':');
      if (separador != -1) {
          int h = horaStr.substring(0, separador).toInt();
          int m = horaStr.substring(separador + 1).toInt();

          // Cria a estrutura e joga na lista
          Agendamento novoAgendamento = {diaStr, h, m, codigo, false};
          listaAgendamentos.push_back(novoAgendamento);

          Serial.printf("Agendamento salvo: %s às %02d:%02d para código %s\n", 
                        diaStr.c_str(), h, m, codigo.c_str());
      }
  }

  // --- TRATAMENTO: LIMPAR MEMÓRIA ---
  if (command == "Clear_Memory" && docBasico["id"] == myID) {
      listaAgendamentos.clear(); 
      
      // Varre a Flash e apaga todos os arquivos de códigos IR salvos
      Dir dir = LittleFS.openDir("/");
      while (dir.next()) {
          LittleFS.remove(dir.fileName());
      }
      Serial.println("Memória RAM e arquivos da Flash foram limpos.");
  }

  // --- TRATAMENTO: SALVAR CÓDIGO NOVO NA FLASH ---
  if (command == "Add_Code" && docBasico["id"] == myID) {
      JsonDocument docRaw; 
      DeserializationError erro = deserializeJson(docRaw, msg);
      
      if (!erro && docRaw.containsKey("name") && docRaw.containsKey("raw")) {
          String nome = docRaw["name"].as<String>();
          String raw = docRaw["raw"].as<String>();
          
          // Cria um arquivo de texto com o nome do comando (ex: "/Ligar.txt")
          File f = LittleFS.open("/" + nome + ".txt", "w");
          if (f) {
              f.print(raw); // Grava a string gigante na Flash
              f.close();
              Serial.printf("Código IR '%s' salvo no arquivo /%s.txt\n", nome.c_str(), nome.c_str());
          } else {
              Serial.println("Erro ao criar arquivo na Flash!");
          }
      }
  }

  // --- TRATAMENTO: DISPARO MANUAL (ATUALIZADO) ---
  if (command == "Dispatch" && docBasico["id"] == myID) {
      JsonDocument docRaw;
      deserializeJson(docRaw, msg);

      // Agora a ESP só espera receber o NOME do código (ex: "Ligar")
      if (docRaw.containsKey("name")) {
          String nomeComando = docRaw["name"].as<String>();
          dispararCodigoSalvo(nomeComando);
      }

      if (docRaw.containsKey("status")) powerStatus = (docRaw["status"] == "Ligado");
      if (docRaw.containsKey("temp")) temperaturaAlvo = docRaw["temp"].as<int>();
      
      enviarStatusCentral(); 
  }
}

void dispararCodigoSalvo(String nomeCodigo) {
  // Tenta abrir o arquivo na Flash
  
  String caminhoArquivo = "/" + nomeCodigo + ".txt";
  File f = LittleFS.open(caminhoArquivo, "r");

  if (!f) {
    Serial.printf("Erro: Arquivo do código '%s' não encontrado na Flash!\n", nomeCodigo.c_str());
    return;
  }

  // Lê o arquivo inteiro para a RAM apenas no momento do disparo
  String rawData = f.readString();
  f.close();

  // Pega o ponteiro da string
  const char* rawStr = rawData.c_str(); 
  
  int count = 1;
  for (int i = 0; rawStr[i] != '\0'; i++) {
    if (rawStr[i] == ',') count++;
  }

  uint16_t* pRaw = new uint16_t[count];
  if (pRaw != nullptr) {
    int index = 0;
    int currentVal = 0;
    
    for (int i = 0; rawStr[i] != '\0'; i++) {
      if (rawStr[i] == ',') {
        pRaw[index++] = currentVal;
        currentVal = 0; 
      } else if (isDigit(rawStr[i])) {
        currentVal = (currentVal * 10) + (rawStr[i] - '0');
      }
    }
    pRaw[index] = currentVal; 

    irsend.sendRaw(pRaw, count, 38);
    yield();
    delete[] pRaw;
    Serial.printf("Disparo IR '%s' executado a partir da Flash com sucesso!\n", nomeCodigo.c_str());
  } else {
    Serial.println("ERRO CRÍTICO: Sem heap para o array IR no momento do disparo!");
  }
}

void checarAgendamentos() {
    const char* diasDaSemana[] = {"Dom", "Seg", "Ter", "Qua", "Qui", "Sex", "Sab"};
    
    time_t agora;
    struct tm infoTempo;
    time(&agora);
    localtime_r(&agora, &infoTempo);

    int horaAtual = infoTempo.tm_hour;
    int minutoAtual = infoTempo.tm_min;
    String diaAtual = diasDaSemana[infoTempo.tm_wday];

    for (auto &agenda : listaAgendamentos) {
        if (agenda.diaDaSemana == diaAtual && agenda.hora == horaAtual && agenda.minuto == minutoAtual) {
            if (!agenda.jaDisparou) {
                Serial.printf("Hora de disparar! Código: %s\n", agenda.nomeCodigo.c_str());
                dispararCodigoSalvo(agenda.nomeCodigo); 
                agenda.jaDisparou = true; 
            }
        } else {
            agenda.jaDisparou = false; 
        }
    }
}

// =======================================================================
// Ciclo de Vida do Microcontrolador
// =======================================================================
void setup() {
  Serial.begin(115200);

  // Inicializa o LittleFS
  if (!LittleFS.begin()) {
      Serial.println("Formatando a memória Flash pela primeira vez...");
      LittleFS.format();
      LittleFS.begin();
  }

  // LÓGICA DE LER O ID
  if (LittleFS.exists("/nodeID.txt")) {
      File f = LittleFS.open("/nodeID.txt", "r");
      myID = f.readString();
      f.close();
  } else {
      myID = "CI000"; // Se o arquivo não existir, assume o ID padrão
  }

  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.setDebugMsgTypes(0);
  irsend.begin();
  mesh.onReceive(&mensagensRecebidas);

  userScheduler.addTask(tarefaChecagem);
  tarefaChecagem.enable();
}

void loop() {
  mesh.update();
}