#include <WiFi.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

// 1. CONFIGURAÇÃO DO SEU WI-FI (DEVE SER A MESMA REDE DO SEU PC!)
const char* WIFI_SSID = "João";
const char* WIFI_PASSWORD = "joaoreal02";

// 2. CONFIGURAÇÃO DO INFLUXDB DOCKER (IP CORRETO DO SEU PC)
//#define INFLUXDB_URL "http://10.126.73.122:8086" 
#define INFLUXDB_URL "http://192.168.18.6:8086"
#define INFLUXDB_TOKEN "QNXw-_kW27ZTqvhDZLTyeyaMC3mfZnPgtrw5L9LjVdnTZhJtfJz9usmV_6JNOtDadQsIbV3qVS46nA25kUQ4ng==" 
#define INFLUXDB_ORG "jardim_iot" 
#define INFLUXDB_BUCKET "jardim_local"

const int LED_BOMBA = 2;

// ─── VARIÁVEIS DE MEMÓRIA E SIMULAÇÃO ───
bool statusBombaAnterior = false; // Guarda o último estado para evitar repetições no Serial
float umidadeSimulada = 50.0;     // Começa em um valor médio de 50%
float nivelAguaSimulado = 100.0;  // Começa com o tanque cheio
float tempo = 0.0;

// Inicializa o cliente do InfluxDB e o ponto de dados
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);
Point dadosSensores("leitura_jardim");

unsigned long ultimoTempo = 0;

void setup() {
  Serial.begin(115200);
  pinMode(LED_BOMBA, OUTPUT);
  digitalWrite(LED_BOMBA, LOW); // Garante a bomba desligada ao iniciar

  // Conexão Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); 
    Serial.print("."); 
  }
  
  Serial.println("\n[Wi-Fi] Conectado com sucesso!");
  client.setInsecure();
  
  if (client.validateConnection()) {
    Serial.println("[InfluxDB] Conectado com sucesso!");
  } else {
    Serial.print("[InfluxDB] Erro de conexão: ");
    Serial.println(client.getLastErrorMessage());
  }
}

void loop() {
  // Executa o bloco a cada 15 segundos (15000ms) para estabilidade total da rede
  if (millis() - ultimoTempo >= 15000) {
    ultimoTempo = millis();

    // ─── 1. LER COMANDO DO NODE-RED NO INFLUXDB ───
    String query = "from(bucket: \"jardim_local\") |> range(start: -5m) |> filter(fn: (r) => r[\"_measurement\"] == \"controle_bomba\") |> filter(fn: (r) => r[\"_field\"] == \"status_bomba\") |> last()";
    
    FluxQueryResult resultado = client.query(query);
    
    if (resultado.next()) {
      FluxValue valor = resultado.getValueByName("_value");
      bool comandoBomba = valor.getBool(); // Lê o booleano puro (true/false)
      
      // SÓ ENTRA AQUI SE HOUVER MUDANÇA DE ESTADO (Evita spam no Serial)
      if (comandoBomba != statusBombaAnterior) {
        if (comandoBomba) {
          digitalWrite(LED_BOMBA, HIGH);
          Serial.println("[STATUS] Bomba LIGADA via Banco de Dados");
        } else {
          digitalWrite(LED_BOMBA, LOW);
          Serial.println("[STATUS] Bomba DESLIGADA via Banco de Dados");
        }
        
        // Atualiza a memória de estado estável
        statusBombaAnterior = comandoBomba; 
      }
    }
    resultado.close(); // Fecha a consulta e libera a porta

    // Pequena pausa técnica (200ms) para o Windows fechar o socket da Query
    // antes de o ESP32 abrir o socket do Write. Isso evita o "Connection Refused".
    delay(200); 

    // ─── 2. SIMULAÇÃO DOS SENSORES (LÓGICA REALISTA) ───
    
    // Gera um ruído analógico natural (varia suavemente entre -1.0 e +1.0)
    float oscilacaoNatural = sin(millis() / 5000.0) * 1.0; 

    if (statusBombaAnterior == true) { 
      // ── SE A BOMBA ESTIVER LIGADA ──
      
      nivelAguaSimulado -= 1.5; // Tanque esvazia de forma moderada
      Serial.println("[SIMULAÇÃO] Bomba Ativa: Tanque esvaziando...");

      umidadeSimulada += 2.0;   // Umidade sobe porque o jardim está sendo regado
      if (umidadeSimulada > 95.0) umidadeSimulada = 95.0; // Teto máximo de umidade

    } else {
      // ── SE A BOMBA ESTIVER DESLIGADA ──
      
      nivelAguaSimulado += 1.5; // Tanque recupera nível na MESMA velocidade
      Serial.println("[SIMULAÇÃO] Bomba Inativa: Tanque recuperando nível...");

      // Umidade tende a voltar devagar para a média seca do ambiente (50%)
      if (umidadeSimulada > 50.0) {
        umidadeSimulada -= 0.5; 
      } else if (umidadeSimulada < 48.0) {
        umidadeSimulada += 0.5; 
      }
    }

    // Aplica a oscilaçãozinha para o gráfico da umidade não ficar totalmente reto
    umidadeSimulada += oscilacaoNatural;
    
    // Travas de segurança para garantir limites de 0% a 100%
    nivelAguaSimulado = constrain(nivelAguaSimulado, 0.0, 100.0);
    umidadeSimulada = constrain(umidadeSimulada, 10.0, 95.0);

    // Mostra os valores calculados no Monitor Serial antes de enviar
    Serial.print(" -> Dados Atuais | Tanque: "); Serial.print(nivelAguaSimulado);
    Serial.print("% | Umidade: "); Serial.print(umidadeSimulada); Serial.println("%");

    // ─── 3. ENVIA OS DADOS ATUALIZADOS PARA O BANCO ───
    dadosSensores.clearFields();
    dadosSensores.addField("umidade", umidadeSimulada);
    dadosSensores.addField("nivel_reservatorio", nivelAguaSimulado);
    dadosSensores.addField("status_bomba", (int)statusBombaAnterior); // 1 para ligada, 0 para desligada

    if (!client.writePoint(dadosSensores)) {
      Serial.print("[Erro Envio] ");
      Serial.println(client.getLastErrorMessage());
    }
  }
}