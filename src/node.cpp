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
// CORREÇÃO: Função auxiliar para imprimir dados da flash na Serial
// =======================================================================
void imprimirArquivosFlash() {
  Serial.println("\n==================================================");
  Serial.println("[LITTLEFS] LISTANDO ARQUIVOS SALVOS NA FLASH");
  Serial.println("==================================================");

  Dir dir = LittleFS.openDir("/"); //[cite: 2]
  int contadorDeArquivos = 0;

  while (dir.next()) { //[cite: 2]
    String nomeArquivo = dir.fileName(); //[cite: 2]
    File f = LittleFS.open("/" + nomeArquivo, "r"); //[cite: 2]
    if (f) {
      size_t tamanho = f.size();
      Serial.printf("-> Arquivo: '%s' (%d bytes)\n", nomeArquivo.c_str(), tamanho);
      Serial.print("   Conteudo: ");
      while (f.available()) {
        Serial.write(f.read());
      }
      Serial.println("\n--------------------------------------------------");
      f.close();
    }
    contadorDeArquivos++;
  }

  if (contadorDeArquivos == 0) {
    Serial.println("Nenhum arquivo encontrado na memoria Flash!");
  } else {
    Serial.printf("Total de %d arquivo(s) encontrado(s).\n", contadorDeArquivos);
  }
  Serial.println("==================================================\n");
}

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
  filter["command"]   = true;
  filter["id"]        = true;
  filter["new_id"]    = true;
  
  filter["timestamp"] = true; 
  filter["dia"]       = true;
  filter["hora"]      = true;
  filter["code"]      = true;

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
  
  // --- TRATAMENTO: MUDANÇA DE ID ---
  if (command == "Change_ID" && docBasico["id"] == myID) {
    String novoID = docBasico["new_id"].as<String>();

    if (novoID.length() > 0) {
        Serial.printf("Alterando ID de '%s' para '%s'\n", myID.c_str(), novoID.c_str());
        myID = novoID;

        File f = LittleFS.open("/nodeID.txt", "w"); //[cite: 2]
        if (f) {
            f.print(myID);
            f.close();
        }

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
    time_t epochTime = docBasico["timestamp"].as<long>(); 

    struct timeval tv;
    tv.tv_sec = epochTime;
    tv.tv_usec = 0;
    settimeofday(&tv, NULL); 

    Serial.println("Relógio interno atualizado com sucesso via Mesh!");
  }

  // --- TRATAMENTO: RECEBER AGENDAMENTO ---
  if (command == "Add_Schedule" && docBasico["id"] == myID) {
      int8_t diaNum = docBasico["dia"].as<int>(); 
      String horaStr = docBasico["hora"].as<String>();
      String codigo = docBasico["code"].as<String>();

      int separador = horaStr.indexOf(':');
      if (separador != -1) {
          int8_t h = horaStr.substring(0, separador).toInt();
          int8_t m = horaStr.substring(separador + 1).toInt();

          Agendamento novoAgendamento = {diaNum, h, m, codigo, false};
          listaAgendamentos.push_back(novoAgendamento);

          Serial.printf("Agendamento salvo: Dia %d às %02d:%02d para código %s\n", 
                        diaNum, h, m, codigo.c_str());
      }
  }

  // --- TRATAMENTO: LIMPAR MEMÓRIA (Proteger nodeID.txt) ---
  if (command == "Clear_Memory" && docBasico["id"] == myID) {
      listaAgendamentos.clear(); 
      
      // CORREÇÃO: Remove todos os arquivos exceto nodeID.txt
      Dir dir = LittleFS.openDir("/"); //[cite: 2]
      while (dir.next()) { //[cite: 2]
          String nomeArquivo = dir.fileName(); //[cite: 2]
          if (nomeArquivo != "nodeID.txt") {
              LittleFS.remove("/" + nomeArquivo);
          }
      }
      Serial.println("Memória RAM e arquivos de códigos limpos. ID protegido.");
      
      // CORREÇÃO: Mostra o estado da flash na serial
      imprimirArquivosFlash();
  }

  // --- TRATAMENTO: SALVAR CÓDIGO NOVO NA FLASH ---
  if (command == "Add_Code" && docBasico["id"] == myID) {
    DynamicJsonDocument docRaw(4096); // Alocação robusta para códigos longos[cite: 2]
    DeserializationError erro = deserializeJson(docRaw, msg);
      
    if (!erro) {
        if (docRaw.containsKey("name") && docRaw.containsKey("raw")) {
            String nome = docRaw["name"].as<String>();
            String raw = docRaw["raw"].as<String>();
              
            File f = LittleFS.open("/" + nome + ".txt", "w"); //[cite: 2]
            if (f) {
                f.print(raw);
                f.close();
                Serial.printf("Código IR '%s' salvo no arquivo /%s.txt\n", nome.c_str(), nome.c_str());
                
                // CORREÇÃO: Mostra a flash com o novo arquivo em tempo real
                imprimirArquivosFlash();
            } else {
                Serial.println("Erro ao criar arquivo na Flash!");
            }
        } else {
            Serial.println("Erro: JSON recebido não possui 'name' ou 'raw'.");
        }
    } else {
        Serial.print("Falha ao ler JSON gigante do Add_Code: ");
        Serial.println(erro.c_str()); 
    }
}

  // --- TRATAMENTO: DISPARO MANUAL ---
  if (command == "Dispatch" && docBasico["id"] == myID) {
      JsonDocument docRaw;
      deserializeJson(docRaw, msg);

      if (docRaw.containsKey("raw")) {
          const char* rawStr = docRaw["raw"];
          Serial.printf("[DEBUG ESP] Recebido comando manual pela rede. Tamanho da string: %d caracteres.\n", strlen(rawStr));
          dispararStringRaw(rawStr);
      } 
      else if (docRaw.containsKey("name")) {
          String nomeComando = docRaw["name"].as<String>();
          Serial.printf("[DEBUG ESP] Disparo pela Flash solicitado. Arquivo: %s\n", nomeComando.c_str());
          dispararCodigoSalvo(nomeComando);
      } else {
          Serial.println("[DEBUG ESP] Comando 'Dispatch' recebido, mas não havia 'raw' nem 'name'.");
      }

      if (docRaw.containsKey("status")) powerStatus = (docRaw["status"] == "Ligado");
      if (docRaw.containsKey("temp")) temperaturaAlvo = docRaw["temp"].as<int>();
      
      enviarStatusCentral(); 
  }
}

void dispararStringRaw(const char* rawStr) { 
  int count = 1;
  for (int i = 0; rawStr[i] != '\0'; i++) {
    if (rawStr[i] == ',') count++;
  }

  Serial.printf("[DEBUG ESP] Preparando para atirar... Array IR com %d posições.\n", count);

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
    
    Serial.println("[DEBUG ESP] POU! Disparo IR executado com sucesso!");
  } else {
    Serial.println("[DEBUG ESP] ERRO CRÍTICO: Sem heap para o array IR!");
  }
}

void dispararCodigoSalvo(String nomeCodigo) {
  String caminhoArquivo = "/" + nomeCodigo + ".txt";
  File f = LittleFS.open(caminhoArquivo, "r");

  if (!f) {
    Serial.printf("[DEBUG ESP] Erro: Arquivo '%s' não encontrado na Flash!\n", caminhoArquivo.c_str());
    return;
  }

  Serial.printf("[DEBUG ESP] Iniciando disparo do agendamento: %s\n", caminhoArquivo.c_str());

  while (f.available()) {
    String linhaRaw = f.readStringUntil('\n');
    linhaRaw.trim(); 
    
    if (linhaRaw.length() > 0) {
      dispararStringRaw(linhaRaw.c_str());
      delay(250); 
    }
  }
  
  f.close();
  Serial.println("[DEBUG ESP] Sequência de disparo do agendamento finalizada!");
}

void checarAgendamentos() {
    time_t agora;
    struct tm infoTempo;
    time(&agora);
    localtime_r(&agora, &infoTempo);

    if (infoTempo.tm_year + 1900 < 2000) {
        return; 
    }

    int8_t horaAtual = infoTempo.tm_hour;
    int8_t minutoAtual = infoTempo.tm_min;
    int8_t diaAtual = infoTempo.tm_wday; 

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

  if (!LittleFS.begin()) {
      Serial.println("Formatando a memória Flash pela primeira vez...");
      LittleFS.format();
      LittleFS.begin();
  }

  // CORREÇÃO: Mostra no monitor serial o que já está salvo assim que o chip liga
  imprimirArquivosFlash();

  if (LittleFS.exists("/nodeID.txt")) { //[cite: 2]
      File f = LittleFS.open("/nodeID.txt", "r"); //[cite: 2]
      myID = f.readString(); //[cite: 2]
      f.close(); //[cite: 2]
  } else {
      myID = "CI000"; //[cite: 2]
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