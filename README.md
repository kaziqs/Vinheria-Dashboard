# Vinheria Agnello: monitoramento IoT da adega

Solução IoT completa para acompanhar a adega da Vinheria Agnello: um **ESP32** lê temperatura, umidade e luminosidade, envia os dados por **MQTT**, e um **Dashboard customizado no Node-RED** exibe as leituras em gráficos em tempo real, gerencia o histórico e permite ligar e desligar um LED remotamente.

Integrantes: 
| ALUNOS | RM |
| --- | :---: |
| `Diego Rayhan Jalhium Machado` | 569129 |
| `Gabriel Almeida dos Santos` | 573690 |
| `Kaue Tsuyoshi Horoiwa` | 571192 |
| `Roberto Dantas Melo Filho` | 566716 |

---

## 1. Como funciona

```
 ESP32 (Wokwi)                     Servidor / seu computador
┌──────────────────┐               ┌─────────────────────────────────────────────────┐
│ DHT22  temp+umid │   publica     │  Broker MQTT  ──►  Node-RED (Dashboard)         │
│ LDR    luz       │ ──────────►   │  (porta 1883)      ├─► Salvar Histórico (.txt)  │
│ LED    aviso     │ ◄──────────   │                    └─► Gráficos e Controles     │
└──────────────────┘    comandos   └─────────────────────────────────────────────────┘
```

| Etapa | O que acontece |
|---|---|
| 1 | O ESP32 lê os sensores e publica no tópico `/TEF/device0XX/attrs` uma mensagem como `T|24.5|H|65.0|L|1800|TS|...`. |
| 2 | O Node-RED recebe a telemetria através do nó MQTT de entrada conectado ao **servidor**. |
| 3 | O fluxo do Node-RED separa os dados para alimentar os gráficos em tempo real e armazena as leituras em arquivo de log (`.txt`). |
| 4 | Os botões do Dashboard enviam comandos (`device0XX@on|` ou `device0XX@off|`) para o tópico `/TEF/device0XX/cmd`; o ESP32 recebe e aciona o LED. |

---

## 2. Estrutura do projeto

```
vinheria-iot-dashboard/
├── node-red-flow.json             Fluxo do Node-RED (Dashboard + MQTT + Histórico)
└── firmware/
    ├── sketch.ino                 Código do ESP32
    ├── libraries.txt              Bibliotecas utilizadas do projeto
    └── diagram.json               Circuito do Wokwi (ESP32 + DHT22 + LDR + LED)
```

---

## 3. Requisitos

- Node-RED instalado com os pacotes `node-red-dashboard`
- Uma conta gratuita no [Wokwi](https://wokwi.com) para simular o ESP32
- Acesso ao broker MQTT
- Navegador atual para acessar o painel do Node-RED (Dashboard UI)

---

## 4. Passo a passo para replicar

### 4.1 Importar o fluxo no Node-RED

1. Abra o seu Node-RED.
2. Clique no menu superior direito (três traços) e selecione **Import**.
3. Cole o código JSON do fluxo do dashboard ou arraste o arquivo correspondente e clique em **Import**.
4. **Troque o ID**: substitua `device0XX` pelo número de identificação correto do seu grupo nos nós MQTT e nos botões de comando. Certifique-se de que o broker MQTT esteja nomeado como **servidor** apontando para o IP correto.

### 4.2 Montar o ESP32 no Wokwi

1. Entre no [projeto](https://wokwi.com/projects/478335014343399425), crie uma cópia do projeto.
2. Abra o arquivo `sketch.ino` do projeto e insira o código do ESP32 configurado com os tópicos corretos (`/TEF/device0XX/attrs` e `/TEF/device0XX/cmd`).
3. Clique em **Play**. O monitor serial deve exibir a conexão bem-sucedida ao Wi-Fi e ao broker MQTT, publicando os pacotes de telemetria.

Ligações do circuito:

| Componente | Pino do componente | Pino do ESP32 |
|---|---|---|
| DHT22 | VCC / SDA / GND | 3V3 / GPIO 4 / GND |
| LDR (módulo) | VCC / AO / GND | 3V3 / GPIO 34 / GND |
| LED (com resistor de 220 ohms) | anodo / catodo | GPIO 2 / GND |

### 4.3 Rodar o Dashboard

1. No Node-RED, certifique-se de que o fluxo está implantado clicando no botão **Deploy**.
2. Acesse o painel visual do Node-RED (geralmente em `http://localhost:1880/ui` ou de acordo com a sua configuração de servidor).
3. Verifique se os gráficos de Temperatura, Umidade e Luminosidade estão recebendo os dados em tempo real.

---

## 5. Usando o dashboard

- **Gráficos em Tempo Real**: Exibição individual da evolução temporal de Temperatura, Umidade e Luminosidade (LDR).
- **Controle de Atuador**: Botões dedicados para **Ligar LED** e **Desligar LED** de forma remota via MQTT.

---

## 6. API

O fluxo opera diretamente com mensagens MQTT e nós nativos de manipulação de arquivos e interface gráfica do Node-RED:

- **Tópico de Telemetria (`/TEF/device0XX/attrs`)**: Recebe strings contendo os valores separados por barras (ex: `T|25.5|H|60.2|L|4095|TS|...`).
- **Tópico de Comando (`/TEF/device0XX/cmd`)**: Envia instruções de acionamento para o atuador (LED).

---

## 7. Decisões de projeto

- **MQTT** escolhido pela leveza e eficiência na comunicação assíncrona entre o ESP32 e o servidor.
- **Node-RED** utilizado para centralizar o recebimento de mensagens, a exibição de gráficos interativos na interface web e a persistência em arquivo de forma visual e ágil, dispensando códigos complexos em servidores web tradicionais.
- **Tratamento por blocos nativos/funções** para separar os atributos da string de telemetria e direcioná-los corretamente para os gráficos correspondentes.

---
