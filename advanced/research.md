# Pesquisa de Versão Avançada (OpenFIDO-ESP Pro)

Este documento detalha os componentes e estratégias para criar uma versão "Pro" do token, com segurança física reforçada e conectividade sem fio.

## 1. Secure Element (Elemento Seguro)
Para proteção contra ataques físicos (ex: decapping, side-channel), as chaves privadas devem ser armazenadas em um chip dedicado.

### Opção A: Microchip ATECC608B (Recomendado)
- **Interface:** I2C.
- **Custo:** ~$0.80 USD.
- **Recursos:** Armazenamento seguro de até 16 chaves, aceleração ECC P-256, SHA-256, AES-128.
- **Vantagem:** Biblioteca "CryptoAuthLib" da Microchip é compatível com ESP-IDF.
- **Integração:** O ESP32 envia o hash para o ATECC608B, que assina internamente e retorna apenas a assinatura. A chave privada nunca sai do chip.

### Opção B: NXP SE050
- **Interface:** I2C.
- **Custo:** ~$1.50 USD.
- **Recursos:** Mais poderoso (suporta RSA-4096, ECC-521), roda JavaCard applets.
- **Desvantagem:** Documentação mais restrita (requer NDA para alguns detalhes) e stack de software mais pesado.

## 2. NFC (Near Field Communication)
Para autenticação "Tap-to-Login" em celulares.

### Opção A: NFC Passivo (NTAG I2C Plus) - *Não serve para U2F completo*
- O NTAG I2C atua como uma tag passiva. O celular lê uma URL.
- **Limitação:** Não suporta a comunicação bidirecional APDU necessária para U2F/FIDO2 padrão. Serve apenas para "WebAuthn via URL" (menos seguro/comum).

### Opção B: NFC Ativo / Card Emulation (NXP PN532 ou PN7150)
- **Interface:** I2C/SPI/UART.
- **Modo:** Card Emulation (HCE).
- **Funcionamento:** O chip NFC emula um cartão inteligente ISO14443-4. O ESP32 recebe as APDUs do leitor NFC e responde.
- **Desafio:** Requer antena sintonizada (13.56 MHz) na PCB.

## 3. BLE (Bluetooth Low Energy)
Para autenticação sem fio em desktops/celulares (FIDO2 over BLE).

### Opção A: ESP32-S3 ou ESP32-C3
- **Vantagem:** O ESP32-S2 **não tem Bluetooth**. Migrar para o **ESP32-S3** ou **C3** adiciona BLE nativo sem custo extra de hardware.
- **Protocolo:** FIDO2 define um perfil BLE específico (FIDO Service UUID `0xFFFD`).
- **Bateria:** O uso de BLE exige bateria (LiPo) + circuito de carga (TP4056), transformando o "stick" USB em um "keyfob" com bateria.

## Conclusão e Roadmap
Para a versão "Pro", recomenda-se:
1.  **Migrar MCU:** Trocar ESP32-S2 por **ESP32-C3** (menor, mais barato, tem BLE).
2.  **Adicionar SE:** Incluir **ATECC608B** no barramento I2C.
3.  **NFC:** Adicionar **PN7150** (se o custo permitir) ou focar apenas em BLE para mobile.
