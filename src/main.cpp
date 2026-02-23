#include <Arduino.h>
#include <DHTesp.h>
#include <PubSubClient.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

#include <cstdlib> // Define a funcao atoi()

/*
 * Configuração de Teste
 */
#define MODO_EMULACAO true


/*
 * Definições de Hardware
 */
#define DHTPIN 32
#define DHTTYPE DHT22
#define PIN_LED 4
#define PIN_RELAY 23
#define PIN_TRIGGER 25
#define PIN_ECHO 34

/*
 * CERTIFICADO UBIDOTS (ISRG Root X1)
 * Válido até 04 Jun 2035 (correção, ver abaixo)
 *
 * ***********
 * IMPORTANTE: Tentando aumentar a performance, trocamos o certificado Root
 * pelo certificado do Ubidots, que é valido somente até 18 abr 2026.
 */
const char* UBIDOTS_ROOT_CA = \
  "-----BEGIN CERTIFICATE-----\n" \
  "MIID/jCCA4OgAwIBAgISBaefSmooirx6XyayYj0OCvZaMAoGCCqGSM49BAMDMDIx\n" \
  "CzAJBgNVBAYTAlVTMRYwFAYDVQQKEw1MZXQncyBFbmNyeXB0MQswCQYDVQQDEwJF\n" \
  "ODAeFw0yNjAxMTgyMzI3MzVaFw0yNjA0MTgyMzI3MzRaMCExHzAdBgNVBAMTFmlu\n" \
  "ZHVzdHJpYWwudWJpZG90cy5jb20wWTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAAR7\n" \
  "QF4rWhRPTpOzlvDn1AKF/X2Q/TlcOyw5pXI9e3c2rQFKbStEB0cnL0KxPt4Q/khY\n" \
  "PHNxfqlS6eAVv/LF6X/zo4ICiDCCAoQwDgYDVR0PAQH/BAQDAgeAMB0GA1UdJQQW\n" \
  "MBQGCCsGAQUFBwMBBggrBgEFBQcDAjAMBgNVHRMBAf8EAjAAMB0GA1UdDgQWBBRV\n" \
  "5t0w+poRe6DXjekmiKoNEAMotDAfBgNVHSMEGDAWgBSPDROi9i5+0VBsMxg4XVmO\n" \
  "I3KRyjAyBggrBgEFBQcBAQQmMCQwIgYIKwYBBQUHMAKGFmh0dHA6Ly9lOC5pLmxl\n" \
  "bmNyLm9yZy8wgYUGA1UdEQR+MHyCD2FwcC51Ymlkb3RzLmNvbYIQYXBwMi51Ymlk\n" \
  "b3RzLmNvbYIUYnVzaW5lc3MudWJpZG90cy5jb22CFmluZHVzdHJpYWwudWJpZG90\n" \
  "cy5jb22CEHN0ZW0udWJpZG90cy5jb22CF3ViaWRvdHMuaW90LnViaWRvdHMuY29t\n" \
  "MBMGA1UdIAQMMAowCAYGZ4EMAQIBMC0GA1UdHwQmMCQwIqAgoB6GHGh0dHA6Ly9l\n" \
  "OC5jLmxlbmNyLm9yZy85MS5jcmwwggEDBgorBgEEAdZ5AgQCBIH0BIHxAO8AdgDL\n" \
  "OPcViXyEoURfW8Hd+8lu8ppZzUcKaQWFsMsUwxRY5wAAAZvTpQRIAAAEAwBHMEUC\n" \
  "IQDr/KUvDUspRrowwqWaVTeskcWgbI2XCqW+/FSYOyCcmgIgMfsm0PY5+3A6AESU\n" \
  "segq33koyC+pBX5U9Vq2PrCXcJYAdQAOV5S8866pPjMbLJkHs/eQ35vCPXEyJd0h\n" \
  "qSWsYcVOIQAAAZvTpQQ2AAAEAwBGMEQCIDzJrisa6+H5YbPWC2iHGO7eQ8AcHre/\n" \
  "uN2eDublWuVDAiBQTLVRoQnv9J3n5piqerdXlo06HK9Lzb0Q5CRx4aJPAzAKBggq\n" \
  "hkjOPQQDAwNpADBmAjEA2fmDMV60lcSCdGe70M0PLX5L6Tjz24Qpn3vsncYrULH+\n" \
  "T60vFZ7fQJO9PiI0yCEcAjEAlvTBIKS9PzH2HC3ypHtzgfGD7adNCZTqHjZmRiHV\n" \
  "jJKX5/ng6+4JHtBTVUg8NxGy\n" \
  "-----END CERTIFICATE-----\n";

/*
 * Definições de WIFI
 */
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

/*
 * Definições do MQTT
 */
const char* MQTT_BROKER = "industrial.api.ubidots.com";
const int   MQTT_PORT   = 8883; // Usando SSL
const char* MQTT_CLIENTID = "esp32_wokwi_fiap_alb01";
const char* MQTT_PUB_TOPIC = "/v1.6/devices/caixa_dagua";
const char* MQTT_SUB_TOPIC_PARA = "/v1.6/devices/caixa_dagua/para/lv";
const char* UBIDOTS_TOKEN = "BBUS-XLhTkIxve44XgAGjT1PLgsrmkO4Hor";

/*
 * Definições do ThingSpeak
 */
const char* REST_THINGSPEAK = "https://api.thingspeak.com/update";
const char* THINGSPEAK_CH_API_KEY = "4A8IK8FMD2CSEKAJ";


/*
 * Variaveis
 */

// Variáveis de controle de estado
bool bomba_ligada = false;
bool parar = false;
unsigned long ultimo_tempo_principal = 0; // Usada para controlar tempo de repeticao do laco principal
unsigned long ultimo_tempo_thingspeak = 0; // Usada para controlar tempo de envio ao ThingSpeak

// Variáveis de Emulação
float emulNivel = 100.0;
float tx_esv = -0.5;
float emulTemp = 25.0;
int stepEmul = 0;
bool modeEsv = true; // true = esvaziando, false = enchendo

// Variaveis de rede
WiFiClientSecure cliente_esp;
PubSubClient mqtt(cliente_esp);

// Sensor DHT
DHTesp sensor_dht;


/*
 * Funções ligadas à camada de sensoriamento
 */

void setupWaterLevelSensor() {
  pinMode(PIN_TRIGGER, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
}

float readWaterLevel() {
  const float DIST_SENS_AGUA_MAX = 19;
  const float DIST_SENS_AGUA_MIN = 85;

  digitalWrite(PIN_TRIGGER, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIGGER, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIGGER, LOW);
  long tempo_eco = pulseIn(PIN_ECHO, HIGH);
  float med_distancia = tempo_eco * 0.034 / 2.0;

  if (med_distancia < DIST_SENS_AGUA_MAX ) med_distancia = DIST_SENS_AGUA_MAX;
  if (med_distancia > DIST_SENS_AGUA_MIN ) med_distancia = DIST_SENS_AGUA_MIN;

  float dif_dist_max_min = (DIST_SENS_AGUA_MIN - DIST_SENS_AGUA_MAX);
  float med_nivel = (med_distancia - DIST_SENS_AGUA_MIN) * (-1.0)
                    * (100.0 / dif_dist_max_min);
  return med_nivel;
}

/*
 * Funções ligadas à camada de processamento e controle
 */

void setupBomba() {
  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_RELAY, LOW);
}

// Retorna true se a bomba está ligada e false se a bomba está desligada
bool atualizarBomba(bool bomba_estava_ligada, bool parada_solicitada,
                    float nivel_agua) {
  bool bomba_ligada = bomba_estava_ligada;
  if (parada_solicitada) {
    Serial.println("[AVISO] Sistema Parado pelo Utilizador");
    bomba_ligada = false;
  } else {
    if (nivel_agua < 50.0) {
      bomba_ligada = true;
    }
    if (nivel_agua > 99.0) {
      bomba_ligada = false;
    }
  }
  digitalWrite(PIN_RELAY, bomba_ligada ? HIGH : LOW);
  return bomba_ligada;
}


/*
 * Funções ligadas à camada de comunicação
 */

void connectWiFi() {
  Serial.print("[WiFi] Conectando...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("conectado!");
}

// Callback acionado por mqtt.loop() quando houver uma mudança no tópico monitorado
void mqttCallback(char* topico, byte* payload, unsigned int length) {
  // Converte o que chegou para char array
  char string_dados[length + 1];
  for (unsigned int i = 0; i < length; i++) {
    string_dados[i] = (char)payload[i];
  }
  string_dados[length] = '\0'; // Fecha a string

  Serial.print("[MQTT] Chegou mensagem no topico: ");
  Serial.println(topico);

  // É o botão de PARADA?
  if (strstr(topico, "para") != NULL) {
    if (atoi(string_dados) == 1) {
      Serial.println(">>> COMANDO: PARAR SISTEMA");
      parar = true;
      digitalWrite(PIN_LED, HIGH);
    } else {
      Serial.println(">>> COMANDO: LIBERAR SISTEMA");
      parar = false;
      digitalWrite(PIN_LED, LOW);
    }
  }
}

void connectMQTT() {
  mqtt.setServer(MQTT_BROKER, MQTT_PORT);
  mqtt.setCallback(mqttCallback);

  while (!mqtt.connected()) {
    Serial.print("[MQTT] Conectando ao Ubidots (SSL)...");

    if (mqtt.connect(MQTT_CLIENTID, UBIDOTS_TOKEN, "")) {
      Serial.println(" conectado!");

      // Assina os dois tópicos desejados
      mqtt.subscribe(MQTT_SUB_TOPIC_PARA);

    } else {
      Serial.print(" falhou (rc=");
      Serial.print(mqtt.state());
      Serial.println("). Tentando novamente em 2s...");
      delay(2000);
    }
  }
}

void sendUbidots(float temperatura, float umidade, float nivel, bool status) {
  char msg[255];

  sprintf(msg,
          "{\"temperatura\": %3.2f, \"umidade\": %3.2f, \"med_nivel\": %3.2f, \"bomba\": %i}",
          temperatura, umidade, nivel, (int)status);

  mqtt.publish(MQTT_PUB_TOPIC, msg);
  Serial.println(String("[MQTT] Topico: ") + MQTT_PUB_TOPIC);
  Serial.println(String("[MQTT] Mensagem: ") + msg);
}

void sendThingSpeak(float temperatura, float umidade, float nivel,
                    bool status) {
  // Cria um cliente temporário apenas para o ThingSpeak
  WiFiClientSecure cliente_thingspeak;
  cliente_thingspeak.setInsecure(); // Ignora validação SSL

  HTTPClient http;
  String url = String(REST_THINGSPEAK) + "?api_key=" + THINGSPEAK_CH_API_KEY +
  "&field1=" + String(temperatura) +
  "&field2=" + String(umidade) +
  "&field3=" + String(nivel) +
  "&field4=" + String(status ? 1 : 0);

  // Conecta usando o WiFiClientSecure
  Serial.println("[HTTP] " + url);
  if (http.begin(cliente_thingspeak, url)) {
    int codigo_resposta_http = http.GET();
    if (codigo_resposta_http > 0) {
      String resposta = http.getString();
      Serial.print("[HTTP] Código de resposta: ");
      Serial.println(codigo_resposta_http);
      Serial.print("[HTTP] Resposta: ");
      Serial.println(resposta);
    } else {
      Serial.print("[HTTP] Erro na requisição: ");
      Serial.println(codigo_resposta_http);
    }
    http.end();
  } else {
    Serial.println("[ThingSpeak] Falha de conexão");
  }
}

void setup() {
  Serial.begin(115200);
  sensor_dht.setup(DHTPIN, DHTesp::DHT22);

  connectWiFi();

  // Carrega o certificado ROOT para o cliente do MQTT
  cliente_esp.setCACert(UBIDOTS_ROOT_CA);

  connectMQTT();

  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, LOW);
  setupWaterLevelSensor();
  setupBomba();
}

void loop() {
  static float nivel_agua = 0.0;
  static float umidade = 0.0;
  static float temperatura = 0.0;

  if (!mqtt.connected()) {
    connectMQTT();
  }
  mqtt.loop();

  unsigned long tempo_atual = millis();

  // Comandos executados a cada 3 segundos
  if (tempo_atual - ultimo_tempo_principal > 3000) {
    ultimo_tempo_principal = tempo_atual;

    if (MODO_EMULACAO) {
      // --- 1. Simula dinâmica do nível da água ---
      if (emulNivel > 99 && modeEsv) {
        tx_esv = -0.5; // Começa a esvaziar
        modeEsv = false;
      } else if (emulNivel < 2 && !modeEsv && bomba_ligada) {
        tx_esv = 1.2; // Começa a encher
        modeEsv = true;
      } else if (!modeEsv && !bomba_ligada) {
        tx_esv = -0.1; // Queda lenta por evaporação
      }
      emulNivel += tx_esv;

      // --- 2. Temperatura Realista (Ciclo Assimétrico) ---
      // Usamos um ajuste na senóide para achatar a "noite" e alongar o "dia"
      float fase = sin(stepEmul * 0.01);
      if (fase < 0) {
        // Simula a noite: queda de temperatura mais suave e estável
        emulTemp = 24.0 + (3.0 * fase);
      } else {
        // Simula o dia: subida de temperatura mais pronunciada
        emulTemp = 24.0 + (8.0 * fase);
      }
      stepEmul++;

      // --- 3. Umidade Inversamente Proporcional ---
      float baseUmid = 110.0; // Ponto de ajuste para a inversão
      float emulUmid = (baseUmid - (2.2 * emulTemp)) + (random(-2, 2));
      emulUmid = constrain(emulUmid, 20.0, 95.0);

      // Atribuição para as variáveis do sistema
      nivel_agua = emulNivel;
      umidade = 50.0 + random(-5, 5); // Umidade com ruído
      temperatura = emulTemp;
    } else {
      nivel_agua = readWaterLevel();
      umidade = sensor_dht.getHumidity();
      temperatura = sensor_dht.getTemperature();
      if (isnan(umidade) || isnan(temperatura)) {
        Serial.println("[AVISO] Não foi possível ler sensor DHT22");
        umidade = 20.0;
        temperatura = 20.0;
      }
    }

    bomba_ligada = atualizarBomba(bomba_ligada, parar, nivel_agua);

    sendUbidots(temperatura, umidade, nivel_agua, bomba_ligada);
  }

  // Comandos executados a cada 20 segundos
  if (tempo_atual - ultimo_tempo_thingspeak > 20000) {
    ultimo_tempo_thingspeak = tempo_atual;
    sendThingSpeak(temperatura, umidade, nivel_agua, bomba_ligada);
  }
}
