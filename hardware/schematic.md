# Esquemático - Token U2F ESP32-S2

Este documento descreve as conexões pino-a-pino para o design do hardware. Use este guia para desenhar o esquemático no KiCad, EasyEDA ou Altium.

## 1. Microcontrolador (U1)
**Componente:** ESP32-S2-MINI-1 (Módulo) ou ESP32-S2 (SoC QFN)
*Assumindo ESP32-S2-MINI-1 para facilidade de soldagem, mas as conexões são análogas para o SoC.*

| Pino (Módulo) | Nome | Conexão | Notas |
| :--- | :--- | :--- | :--- |
| 1 | GND | GND | Plano de Terra |
| 2 | GND | GND | Plano de Terra |
| 3 | 3V3 | +3.3V (Saída do LDO) | Desacoplamento: 10uF + 100nF |
| ... | ... | ... | ... |
| 13 | IO0 | Botão (SW1) + Pull-up 10k | Boot Mode (Baixo no boot = Download) |
| 14 | IO46 | NC | (Input only) |
| ... | ... | ... | ... |
| 20 | IO19 | USB_D- (Pino 2 do USB) | Par Diferencial |
| 21 | IO20 | USB_D+ (Pino 3 do USB) | Par Diferencial |
| ... | ... | ... | ... |
| 30 | IO18 | LED RGB (DIN) | WS2812B Data In |
| 35 | TXD0 | Header de Debug (Opcional) | Para logs UART |
| 36 | RXD0 | Header de Debug (Opcional) | Para logs UART |
| EN | EN | Circuito RC (10k Pull-up + 1uF GND) | Reset / Enable |

## 2. Interface USB (J1)
**Componente:** Conector USB Tipo-A (PCB Edge ou SMD) ou Tipo-C (Receptáculo 16-pin)

| Pino USB | Nome | Conexão | Notas |
| :--- | :--- | :--- | :--- |
| VBUS | +5V | Entrada do LDO (U2) | Proteção ESD necessária |
| D- | Data- | U3 (ESD) -> ESP32 IO19 | Impedância 90 ohms |
| D+ | Data+ | U3 (ESD) -> ESP32 IO20 | Impedância 90 ohms |
| GND | GND | GND | |
| Shield | Shield | GND (via Resistor 1M // Cap 4.7nF) | Opcional, ou direto ao GND |

*Se usar USB-C:* Conectar resistores de 5.1k (R_CC1, R_CC2) do CC1 e CC2 para o GND para ser reconhecido como "Device".

## 3. Regulação de Energia (U2)
**Componente:** AMS1117-3.3 ou XC6206P332MR

| Pino | Função | Conexão | Componentes Passivos |
| :--- | :--- | :--- | :--- |
| IN | Entrada | VBUS (+5V) | Cap Cerâmico 10uF (C1) |
| GND | Terra | GND | |
| OUT | Saída | +3.3V | Cap Cerâmico 10uF (C2) + 100nF (C3) |

## 4. Proteção ESD (U3)
**Componente:** USBLC6-2SC6 (SOT-23-6)

| Pino | Função | Conexão |
| :--- | :--- | :--- |
| 1 | I/O 1 | USB D+ (Conector) |
| 2 | GND | GND |
| 3 | I/O 2 | USB D- (Conector) |
| 4 | I/O 2 | ESP32 IO19 (D-) |
| 5 | VBUS | VBUS (+5V) |
| 6 | I/O 1 | ESP32 IO20 (D+) |

## 5. Interface de Usuário
### Botão (SW1)
- **Tipo:** Tact Switch Momentâneo (NO)
- **Pino 1:** IO0 (ESP32)
- **Pino 2:** GND
- **Nota:** O IO0 já possui pull-up interno fraco, mas recomenda-se um externo de 10k para evitar resets espúrios por ruído.

### LED RGB (D1)
- **Tipo:** WS2812B-2020 (Tamanho 2020 ou 3535)
- **VDD:** +3.3V (Nota: WS2812B oficial pede 5V, mas funciona bem em 3.3V. Se instável, usar pino VBUS com diodo para baixar para ~4.3V ou usar WS2812B-V5).
- **GND:** GND
- **DIN:** IO18 (ESP32)
- **DOUT:** NC
- **Capacitor:** 100nF entre VDD e GND (perto do LED).

## Notas de Design
- **Strapping Pins:** Cuidado com IO0, IO45, IO46. IO0 deve estar ALTO no boot para rodar o firmware. O botão aterra o IO0, então o usuário NÃO deve segurar o botão ao plugar o dispositivo (a menos que queira entrar no modo de gravação).
