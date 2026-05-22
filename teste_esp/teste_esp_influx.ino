#include <WiFi.h>
#include <InfluxDbClient.h>
#include <WebServer.h>

// 1. CONFIGURAÇÃO DO SEU WI-FI (DEVE SER A MESMA REDE DO SEU PC!)
const char* WIFI_SSID = "GlobalNETAP601";
const char* WIFI_PASSWORD = "nala0205";

// 2. CONFIGURAÇÃO DO INFLUXDB DOCKER (IP CORRETO DO SEU PC)
#define INFLUXDB_URL "http://192.168.18.6:8086" 
#define INFLUXDB_TOKEN "QNXw-_kW27ZTqvhDZLTyeyaMC3mfZnPgtrw5L9LjVdnTZhJtfJz9usmV_6JNOtDadQsIbV3qVS46nA25kUQ4ng==" 
#define INFLUXDB_ORG "jardim_iot" 
#define INFLUXDB_BUCKET "jardim_local"

const int LED_BOMBA = 2;

// Variáveis da simulação
float umidadeSimulada = 60.0;
float nivelAguaSimulado = 100.0;
float tempo = 0.0;

// Inicializa o cliente do InfluxDB e o Servidor Web na porta 80
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);
Point dadosSensores("leitura_jardim");
WebServer server(80); 

// --- FUNÇÕES DO SERVIDOR WEB (O que fazer quando receber os comandos) ---

// 1. Página inicial que cria o botão na tela do seu navegador
void lidarPaginaPrincipal() {
  String html = "<html><head><meta charset='UTF-8'><title>Painel do Jardim</title>";
  html += "<style>body{font-family:Arial; text-align:center; margin-top:50px; background:#f4f4f4;}";
  html += ".btn{padding:20px 40px; font-size:24px; color:white; border:none; border-radius:8px; cursor:pointer; margin:10px;}";
  html += ".on{background-color:#2ecc71;} .off{background-color:#e74c3c;}</style></head><body>";
  html += "<h1>Controle do Jardim IoT</h1>";
  
  // Mostra o status atual da bomba na página
  if (digitalRead(LED_BOMBA) == HIGH) {
    html += "<p>Status Atual: <strong>LIGADA</strong></p>";
    html += "<a href='/bomba/desligar'><button class='btn off'>DESLIGAR BOMBA</button></a>";
  } else {
    html += "<p>Status Atual: <strong>DESLIGADA</strong></p>";
    html += "<a href='/bomba/ligar'><button class='btn on'>LIGAR BOMBA</button></a>";
  }
  
  html += "</body></html>";
  server.send(200, "text/html", html);
}

// 2. Rota para LIGAR a bomba
void ligarBomba() {
  digitalWrite(LED_BOMBA, HIGH);
  Serial.println("[WEB] Comando Recebido: LIGAR BOMBA");
  server.sendHeader("Location", "/"); // Redireciona de volta para a página principal
  server.send(303);
}

// 3. Rota para DESLIGAR a bomba
void desligarBomba() {
  digitalWrite(LED_BOMBA, LOW);
  Serial.println("[WEB] Comando Recebido: DESLIGAR BOMBA");
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BOMBA, OUTPUT);
  digitalWrite(LED_BOMBA, LOW);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  
  Serial.println("\n[Wi-Fi] Conectado!");
  Serial.print("[Wi-Fi] IP DO ESP32: ");
  Serial.println(WiFi.localIP()); // <-- ATENÇÃO A ESTE IP NO MONITOR SERIAL!

  client.setInsecure();
  if (client.validateConnection()) Serial.println("[InfluxDB] Conectado!");

  // Configura os caminhos (URLs) que o servidor do ESP32 vai aceitar
  server.on("/", lidarPaginaPrincipal);
  server.on("/bomba/ligar", ligarBomba);
  server.on("/bomba/desligar", desligarBomba);
  
  server.begin(); // Liga o servidor web no ESP32
  Serial.println("[WEB] Servidor HTTP do ESP32 iniciado!");
}

unsigned long ultimoEnvioBanco = 0;

void loop() {
  server.handleClient(); // Fica checando a cada milissegundo se você clicou no botão web

  // Envia dados para o InfluxDB a cada 2 segundos sem travar o botão (sem usar delay longo)
  if (millis() - ultimoEnvioBanco >= 2000) {
    ultimoEnvioBanco = millis();

    tempo += 0.05;
    umidadeSimulada = 60.0 + (20.0 * sin(tempo));

    // Lógica corrigida do reservatório
    if (digitalRead(LED_BOMBA) == HIGH) { 
      nivelAguaSimulado -= 2.5; // Esvazia o barril se a bomba ligar!
    } else {
      nivelAguaSimulado += 0.2; // Enche devagar se estiver desligada
    }
    nivelAguaSimulado = constrain(nivelAguaSimulado, 0.0, 100.0);
    int statusBomba = digitalRead(LED_BOMBA);

    dadosSensores.clearFields();
    dadosSensores.addField("umidade", umidadeSimulada);
    dadosSensores.addField("nivel_reservatorio", nivelAguaSimulado);
    dadosSensores.addField("status_bomba", statusBomba);

    if (!client.writePoint(dadosSensores)) {
      Serial.println(client.getLastErrorMessage());
    }
  }
}