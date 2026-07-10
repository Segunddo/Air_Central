#include "node.h"

// Inicialização dos objetos globais
painlessMesh mesh;
Preferences preferences;
IRsend irsend(IR_SEND_PIN);

String myID; 
bool powerStatus = false;
int temperaturaAtual = 25;
int temperaturaAlvo = 25;

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
  
  // --- TRATAMENTO: MUDANÇA DE ID ---
  if (command == "Change_ID" && docBasico["id"] == myID) {
    String novoID = docBasico["new_id"].as<String>();

    if (novoID.length() > 0) {
        Serial.printf("Alterando ID de '%s' para '%s'\n", myID.c_str(), novoID.c_str());
        myID = novoID;

        // Muda a nível de flash
        preferences.putString("nodeID", myID);

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
      mapaCodigosIR.clear(); // Limpa os códigos velhos também
      Serial.println("Memória de agendamentos e códigos limpa.");
  }

  // --- TRATAMENTO: SALVAR CÓDIGO NOVO ---
  if (command == "Add_Code" && docBasico["id"] == myID) {
      // Como o Add_Code traz uma string muito longa, precisamos de um buffer maior
      JsonDocument docRaw; 
      DeserializationError erro = deserializeJson(docRaw, msg);
      
      if (!erro && docRaw.containsKey("name") && docRaw.containsKey("raw")) {
          String nome = docRaw["name"].as<String>();
          String raw = docRaw["raw"].as<String>();
          
          mapaCodigosIR[nome] = raw; // Salva no dicionário
          Serial.printf("Código IR '%s' salvo na RAM.\n", nome.c_str());
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
  // Verifica se o código existe na memória
  if (mapaCodigosIR.find(nomeCodigo) == mapaCodigosIR.end()) {
    Serial.printf("Erro: O código '%s' não foi encontrado na memória!\n", nomeCodigo.c_str());
    return;
  }

  // Pega a string gigante correspondente ao nome
  const char* rawStr = mapaCodigosIR[nomeCodigo].c_str(); 
  
  int count = 1;
  for (int i = 0; rawStr[i] != '\0'; i++) {
    if (rawStr[i] == ',') count++;
  }

  uint16_t* pRaw = new (std::nothrow) uint16_t[count]; 
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
    Serial.printf("Disparo IR '%s' executado com sucesso!\n", nomeCodigo.c_str());
  } else {
    Serial.println("ERRO CRÍTICO: Sem heap para o array IR!");
  }
}

void checarAgendamentos() {
  time_t agora;
  struct tm infoTempo;
  
  // Pega o tempo atual do sistema
  time(&agora); 
  // Quebra os segundos em horas, minutos, dias, etc.
  localtime_r(&agora, &infoTempo); 

  int horaAtual = infoTempo.tm_hour;
  int minutoAtual = infoTempo.tm_min;
  int diaDaSemana = infoTempo.tm_wday; // 0 = Domingo, 1 = Segunda...

  // Aqui entra a sua lógica de comparar com os agendamentos salvos!
  // printf("Hora atual: %02d:%02d\n", horaAtual, minutoAtual);
}

void taskChecadorDeTempo(void *pvParameters) {
    // Array para converter o número do dia do sistema no formato do Qt
    const char* diasDaSemana[] = {"Dom", "Seg", "Ter", "Qua", "Qui", "Sex", "Sab"};

    while (true) {
        time_t agora;
        struct tm infoTempo;
        time(&agora);
        localtime_r(&agora, &infoTempo);

        int horaAtual = infoTempo.tm_hour;
        int minutoAtual = infoTempo.tm_min;
        String diaAtual = diasDaSemana[infoTempo.tm_wday];

        // Percorre todos os agendamentos salvos na memória
        for (auto &agenda : listaAgendamentos) {
            
            // Verifica se é o dia e a hora exata
            if (agenda.diaDaSemana == diaAtual && agenda.hora == horaAtual && agenda.minuto == minutoAtual) {
                
                // Só dispara se ainda não tiver disparado neste minuto
                if (!agenda.jaDisparou) {
                    Serial.printf("Hora de disparar! Código: %s\n", agenda.nomeCodigo.c_str());
                    
                    // AQUI VOCÊ CHAMA A SUA FUNÇÃO QUE BUSCA O RAW NA PREFERENCES E DISPARA O IR
                    // dispararCodigoSalvo(agenda.nomeCodigo); 

                    agenda.jaDisparou = true; // Trava para não disparar de novo
                }
            } else {
                // Se o tempo já passou (o minuto virou), destrava para o próximo dia/semana
                agenda.jaDisparou = false; 
            }
        }

        // Manda a Task dormir por 5 segundos para liberar o processador para a Mesh
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

// =======================================================================
// Ciclo de Vida do Microcontrolador
// =======================================================================
void setup() {
  preferences.begin("config", false);
  myID = preferences.getString("nodeID", "CI000");

  Serial.begin(115200);
  mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT);
  mesh.setDebugMsgTypes(0);
  irsend.begin();
  mesh.onReceive(&mensagensRecebidas);

  xTaskCreate(taskChecadorDeTempo, "ChecaTempo", 4096, NULL, 1, NULL);
}

void loop() {
  mesh.update();
}