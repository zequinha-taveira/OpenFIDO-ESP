# Arquitetura do Sistema - Token U2F/FIDO2 (ESP32-S2)

## 1. Diagrama de Blocos de Hardware

```mermaid
graph TD
    subgraph "Conector USB"
        VBUS[VBUS (+5V)]
        GND[GND]
        DP[D+]
        DM[D-]
    end

    subgraph "Regulação de Energia"
        LDO[LDO 3.3V]
        C_IN[Capacitor Entrada 10uF]
        C_OUT[Capacitor Saída 10uF]
    end

    subgraph "Proteção"
        ESD[Proteção ESD (USBLC6-2)]
    end

    subgraph "Microcontrolador (ESP32-S2)"
        MCU_USB[USB OTG PHY]
        MCU_CORE[Xtensa LX7 Core]
        MCU_CRYPTO[Crypto Accelerator]
        MCU_TRNG[TRNG]
        MCU_FLASH[SPI Flash (4MB)]
        MCU_GPIO[GPIOs]
    end

    subgraph "Interface do Usuário"
        BTN[Botão Tátil (User Presence)]
        LED[LED RGB (WS2812B)]
    end

    %% Conexões de Energia
    VBUS --> LDO
    VBUS --> C_IN
    LDO --> MCU_CORE
    LDO --> C_OUT
    GND --> MCU_CORE

    %% Conexões USB
    DP --> ESD
    DM --> ESD
    ESD --> MCU_USB

    %% Conexões UI
    BTN --> MCU_GPIO
    MCU_GPIO --> LED
```

## 2. Especificação de Hardware

### 2.1. Microcontrolador
- **Modelo:** Espressif ESP32-S2 (SoC) ou ESP32-S2-MINI-1 (Módulo).
- **Justificativa:** USB Nativo, TRNG de hardware, Aceleração Criptográfica (ECC, SHA), Baixo custo.
- **Clock:** 240 MHz.
- **Flash:** 4MB (Mínimo).

### 2.2. Interface USB
- **Pinos:** GPIO 20 (D+), GPIO 19 (D-).
- **Proteção:** USBLC6-2SC6 ou SRV05-4 (Array de diodos TVS para proteção ESD).
- **Resistores:** O ESP32-S2 possui resistores de pull-up/pull-down internos para USB, mas recomenda-se verificar o datasheet para necessidade de resistores de série (0Ω ou 22Ω) para casamento de impedância.

### 2.3. Alimentação
- **Regulador (LDO):** AMS1117-3.3 (SOT-223) ou XC6206 (SOT-23) para menor footprint.
- **Filtragem:** Capacitores cerâmicos de 10µF e 100nF na entrada e saída do LDO e pinos de alimentação do MCU.

### 2.4. Interface de Usuário
- **Botão (User Presence):**
  - Tipo: Tátil (Tact Switch) ou Capacitivo (Touch Pad do ESP32).
  - Pino Sugerido: GPIO 0 (Boot) ou GPIO 1. *Nota: Usar GPIO 0 economiza pinos, mas requer cuidado para não entrar em modo de boot ao conectar.*
- **LED de Status:**
  - Tipo: LED RGB Endereçável (WS2812B / NeoPixel).
  - Pino Sugerido: GPIO 18.
  - Cores:
    - **Azul Piscando:** Aguardando toque (Autenticação).
    - **Verde:** Sucesso.
    - **Vermelho:** Erro.

## 3. Arquitetura de Firmware

```mermaid
graph TD
    subgraph "Camada de Aplicação"
        U2F_APP[App U2F / CTAP1]
        FIDO2_APP[App FIDO2 / CTAP2 (Futuro)]
        MGMT[Gerenciamento de Dispositivo]
    end

    subgraph "Camada de Protocolo"
        HID_HANDLER[Manipulador HID]
        CMD_PARSER[Parser de Comandos APDU]
    end

    subgraph "Camada de Serviços"
        CRYPTO_SVC[Serviço Criptográfico (mbedTLS / Hardware)]
        STORAGE_SVC[Gerenciador de Armazenamento (NVS)]
        UI_SVC[Controle de LED e Botão]
    end

    subgraph "Camada de Abstração de Hardware (ESP-IDF)"
        TINYUSB[TinyUSB Stack]
        HAL_GPIO[GPIO Driver]
        HAL_RNG[RNG Driver]
        HAL_FLASH[SPI Flash Driver]
    end

    %% Fluxo
    TINYUSB --> HID_HANDLER
    HID_HANDLER --> CMD_PARSER
    CMD_PARSER --> U2F_APP
    U2F_APP --> CRYPTO_SVC
    U2F_APP --> STORAGE_SVC
    U2F_APP --> UI_SVC
    CRYPTO_SVC --> HAL_RNG
    STORAGE_SVC --> HAL_FLASH
    UI_SVC --> HAL_GPIO
```

### 3.1. Mapa de Memória (Partições)
| Nome | Tipo | Subtipo | Tamanho | Descrição |
| :--- | :--- | :--- | :--- | :--- |
| nvs | data | nvs | 0x4000 | Armazenamento não-volátil (Configurações) |
| otadata | data | ota | 0x2000 | Dados de seleção de boot OTA |
| phy_init | data | phy | 0x1000 | Dados de calibração RF (se usar WiFi, pode remover se desabilitar RF) |
| factory | app | factory | 1M | Firmware principal |
| storage | data | spiffs | Restante | Armazenamento de chaves (se necessário) e contadores |

### 3.2. Segurança de Dados
- **Chave de Attestation:** Gerada na fábrica ou no primeiro boot, armazenada em partição NVS criptografada (Flash Encryption).
- **Contador Global:** Armazenado na NVS, incrementado a cada autenticação.
- **Key Wrapping:** O "Key Handle" retornado ao navegador conterá a chave privada criptografada com uma "Master Key" do dispositivo (AES-256-GCM), garantindo que o dispositivo não precise armazenar chaves infinitas.

## 4. Próximos Passos (Fase 3)
1. Desenhar o esquemático no KiCad/EasyEDA.
2. Selecionar os part numbers exatos dos componentes (BOM).
3. Roteamento da PCB.
