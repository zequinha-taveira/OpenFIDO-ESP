# Comparativo de Microcontroladores para Token U2F

| Característica | **Raspberry Pi RP2040** | **Espressif ESP32-S2** | **ST STM32F411** | **Nordic nRF52840** |
| :--- | :--- | :--- | :--- | :--- |
| **Arquitetura** | Dual-core Cortex-M0+ @ 133MHz | Single-core Xtensa LX7 @ 240MHz | Cortex-M4F @ 100MHz | Cortex-M4F @ 64MHz |
| **Flash / RAM** | Externa (QSPI) / 264KB SRAM | Interna (até 4MB) / 320KB SRAM | 512KB Flash / 128KB RAM | 1MB Flash / 256KB RAM |
| **USB Nativo** | Sim (Device/Host) | Sim (OTG) | Sim (OTG) | Sim (Device) |
| **TRNG (Hardware)** | **Não** (Apenas ROSC - requer "whitening") | **Sim** | **Não** (F411 não tem, F405/F415 tem) | **Sim** |
| **Aceleração Crypto** | Não (Software bit-banging ou asm otimizado) | Sim (AES, SHA, RSA, ECC*) | Não (Software) | Sim (CryptoCell cc310) |
| **Segurança Flash** | Baixa (Flash externa é vulnerável a dump) | Média (Flash Encryption + Secure Boot) | Alta (RDP Level 1/2) | Alta (Access Control Lists) |
| **Custo (Chip)** | Muito Baixo (~$0.70) | Baixo (~$1.50) | Médio (~$3.00 - clones mais baratos) | Alto (~$4.00) |
| **Disponibilidade** | Excelente | Excelente | Boa (mas sofreu na crise) | Boa |
| **Ecossistema** | Pico SDK, Arduino, Rust | ESP-IDF, Arduino | STM32Cube, libopencm3 | nRF5 SDK, Zephyr |

## Análise Detalhada

### 1. Raspberry Pi RP2040
- **Prós:** Muito barato, fácil de soldar (QFN), documentação incrível, comunidade ativa.
- **Contras:** **Sem Flash interna** (segurança física fraca, chaves podem ser lidas do chip flash externo facilmente), sem TRNG real (o ROSC passa nos testes estatísticos com pós-processamento, mas não é um TRNG criptográfico dedicado).
- **Veredito:** Ótimo para protótipos e aprendizado, mas **inseguro para um produto final sério** devido à flash externa desprotegida.

### 2. Espressif ESP32-S2
- **Prós:** USB nativo, aceleradores criptográficos (ECC, SHA), TRNG real, Flash Encryption e Secure Boot transparentes. Custo muito competitivo.
- **Contras:** Documentação de USB as vezes confusa no IDF, consumo de energia mais alto que Cortex-M.
- **Veredito:** **Forte candidato.** Oferece o melhor equilíbrio entre segurança (Flash Encryption), recursos (Crypto HW) e preço.

### 3. STM32F411 (Black Pill)
- **Prós:** Padrão da indústria, muito robusto.
- **Contras:** O modelo F411 **NÃO possui TRNG** (apenas o F405/F407/L4 possuem). Implementar U2F seguro sem TRNG é arriscado. Preço subiu.
- **Veredito:** Descartado na versão F411 por falta de TRNG. O STM32L432KC seria uma alternativa melhor (tem TRNG e USB), mas é mais caro.

### 4. Nordic nRF52840
- **Prós:** O "padrão ouro" para chaves de segurança (usado no Google Titan, YubiKey Bio, etc.). Tem ARM TrustZone (alguns modelos), CryptoCell-310 (aceleração total), TRNG robusto, baixo consumo.
- **Contras:** Preço mais elevado, pacote BGA ou QFN complexo, toolchain (Zephyr/nRF Connect) tem curva de aprendizado mais íngreme que Arduino/Pico.
- **Veredito:** A melhor escolha técnica, mas talvez "overkill" ou caro para um projeto DIY de baixo custo inicial.

## Recomendação Final

**Vencedor Custo-Benefício: ESP32-S2 (ou S3)**
- Possui os requisitos críticos de segurança (TRNG, Flash Encryption) que o RP2040 não tem.
- Mais barato que STM32 e nRF52.
- USB nativo funciona bem.

**Vencedor "Padrão Ouro": nRF52840**
- Se o orçamento permitir e a complexidade de soldagem não for problema (ou usar módulos pré-soldados), é a melhor plataforma de segurança.

**Decisão Sugerida para o Projeto:**
Iniciar com **ESP32-S2** (ou **ESP32-S3** se quiser BLE futuro) devido à facilidade de obter placas de desenvolvimento (Wemos S2 Mini, ESP32-S3-DevKitC) e robustez de segurança integrada.
