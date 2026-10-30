#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define DHTPIN 4
#define DHTTYPE DHT22
#define LED_PIN 2
#define LDRPIN 34

const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "3.135.234.101"; 

// 1. Correção do Tópico: 'device001' alterado para minúsculo para alinhar com o Python
const char* topico_telemetria = "/TEF/device0XX/attrs";   //Aqui ajustar o seu numero de indentificação (ID) 0XX 
const char* topico_comando = "/TEF/device0XX/cmd"; //Aqui ajustar o seu numero de indentificação (ID) 0XX      

DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastMsg = 0;
const long interval = 2000;
int pwm = 0;
int leituraLDR = 0;

void callback(char* topic, byte* payload, unsigned int length) 
{
    // Alocação dinâmica segura na stack e terminação da string
    char msg[length + 1];
    memcpy(msg, payload, length);
    msg[length] = '\0';
    
    Serial.printf("Comando recebido no topico %s: %s\n", topic, msg);
    
    // 2. Correção de Payload: Inclusão do '001' no prefixo esperado
    if (strcmp(msg, "device0XX@on|") == 0)           //Aqui ajustar o seu numero de indentificação (ID) 0XX
    {
        digitalWrite(LED_PIN, HIGH);
        Serial.println("Ação: LED LIGADO");
    } 
    else if (strcmp(msg, "device0XX@off|") == 0)    //Aqui ajustar o seu numero de indentificação (ID) 0XX
    {
        digitalWrite(LED_PIN, LOW);
        Serial.println("Ação: LED DESLIGADO");
    } 
    else {
        // 3. Mitigação de Falha Silenciosa: Tratamento de exceção local
        Serial.println("Ação: Comando não reconhecido. Ignorado.");
    }
}

void setup() 
{
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW); 
    digitalWrite(LDRPIN, 0);
    
    dht.begin();
    
    WiFi.begin(ssid, password);
    Serial.print("Conectando WiFi");
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi Conectado!");
    
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
}

void reconnect() {
    while (!client.connected()) 
    {
        Serial.print("Conectando ao MQTT...");
        
        char clientId[30];
        snprintf(clientId, sizeof(clientId), "ESP32_Wokwi_%04X", random(0xffff));
        
        if (client.connect(clientId)) 
        { 
            Serial.println("Conectado!");
            // Assinatura de tópico efetuada com a nova constante
            client.subscribe(topico_comando);
        } 
        else 
        {
            Serial.print("Falha, rc=");
            Serial.print(client.state());
            Serial.println(" Retentando em 5s...");
            delay(5000);
        }
    }
}

void loop() 
{
    if (!client.connected()) reconnect();
    
    client.loop(); 

    unsigned long now = millis();
    if (now - lastMsg > interval) 
    {
        lastMsg = now;
        
        // Valor da temperatura e umidade
        float t = dht.readTemperature();
        float h = dht.readHumidity();

        // Valor do LDR
        leituraLDR = analogRead(LDRPIN);

        // se o ldr não estiver funcionando printar
        if (isnan(leituraLDR)) 
        {
          Serial.println("Falha ao ler o LDR");
          return;
        }
        
        if (isnan(t) || isnan(h)) 
        {
            Serial.println("Falha ao ler o DHT!");
            return;
        }

        char buffer[50];

        // print da temperatura e umidade
        Serial.println(leituraLDR);
        snprintf(buffer, sizeof(buffer), "T|%.1f|H|%.1f|L|%d", t, h, leituraLDR); 

        // print do ldr
        // snprintf(buffer, sizeof(buffer), "l|%.1f", ldrValue);
        
        client.publish(topico_telemetria, buffer);
        Serial.printf("Publicado: %s\n", buffer);
    }
}