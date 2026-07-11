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
    JsonDocument docRaw; // Se usar ArduinoJson v6, use DynamicJsonDocument docRaw(2048);
    DeserializationError erro = deserializeJson(docRaw, msg);
      
    if (!erro) {
        if (docRaw.containsKey("name") && docRaw.containsKey("raw")) {
            String nome = docRaw["name"].as<String>();
            String raw = docRaw["raw"].as<String>();
              
            File f = LittleFS.open("/" + nome + ".txt", "w");
            if (f) {
                f.print(raw);
                f.close();
                Serial.printf("Código IR '%s' salvo no arquivo /%s.txt\n", nome.c_str(), nome.c_str());
            } else {
                Serial.println("Erro ao criar arquivo na Flash!");
            }
        } else {
            Serial.println("Erro: JSON recebido não possui 'name' ou 'raw'.");
        }
    } else {
        // Isso vai te mostrar exatamente se o JSON está chegando quebrado!
        Serial.print("Falha ao ler JSON gigante do Add_Code: ");
        Serial.println(erro.c_str()); 
    }
}

  // --- TRATAMENTO: DISPARO MANUAL (ATUALIZADO) ---
  // --- TRATAMENTO: DISPARO MANUAL (ATUALIZADO) ---
  if (command == "Dispatch" && docBasico["id"] == myID) {
      JsonDocument docRaw;
      deserializeJson(docRaw, msg);

      if (docRaw.containsKey("raw")) {
          String rawString = docRaw["raw"].as<String>();
          
          // 👇 DEBUG AQUI: Confirma que recebeu a string gigante pela rede
          Serial.printf("[DEBUG ESP] Recebido comando manual pela rede. Tamanho da string: %d caracteres.\n", rawString.length());
          
          dispararStringRaw(rawString);
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

void dispararStringRaw(String rawData) {
  const char* rawStr = rawData.c_str(); 
  
  int count = 1;
  for (int i = 0; rawStr[i] != '\0'; i++) {
    if (rawStr[i] == ',') count++;
  }

  // 👇 DEBUG AQUI: Verifica se contou os números corretamente
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
    
    // 👇 DEBUG AQUI: Confirma que não travou e o LED piscou
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

  // Lê o arquivo linha por linha em vez de carregar tudo de uma vez para a RAM
  while (f.available()) {
    String linhaRaw = f.readStringUntil('\n');
    linhaRaw.trim(); // Remove quebras de linha invisíveis (\r)
    
    if (linhaRaw.length() > 0) {
      dispararStringRaw(linhaRaw);
      
      // O mesmo delay de 250ms que colocamos no Qt!
      // Usamos delay() pois ele já chama yield() internamente no ESP8266
      delay(250); 
    }
  }
  
  f.close();
  Serial.println("[DEBUG ESP] Sequência de disparo do agendamento finalizada!");
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
  Serial.setRxBufferSize(2048);
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