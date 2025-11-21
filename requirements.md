# Documento de Requisitos - Token de Autenticação Física Open-Source

## 1. Visão Geral
Desenvolver um token de segurança física (chave de segurança) de baixo custo, open-source, compatível com o padrão U2F (Universal 2nd Factor) e preparado para FIDO2, utilizando um único microcontrolador.

## 2. Requisitos Funcionais

### 2.1. Padrões de Autenticação
- **U2F (CTAP1):** Obrigatório. Suporte completo para registro e autenticação.
- **FIDO2 (CTAP2):** Opcional (meta futura). Suporte a WebAuthn, credenciais residentes e PIN.

### 2.2. Interface de Comunicação
- **USB HID:** Interface primária. O dispositivo deve ser reconhecido como um dispositivo de interface humana (HID) padrão, sem necessidade de drivers adicionais no host.
- **Conector:** USB Tipo-A ou USB Tipo-C (definido no design da PCB).

### 2.3. Interação com Usuário
- **Botão Físico:** Para confirmação de presença do usuário (User Presence - UP).
- **Feedback Visual:** LED de status para indicar operação (aguardando toque, processando, erro).

### 2.4. Criptografia e Segurança
- **Algoritmos:**
  - ECC P-256 (secp256r1) para assinatura digital.
  - SHA-256 para hashing.
- **Geração de Chaves:**
  - TRNG (True Random Number Generator) de hardware para geração de seeds e nonces.
  - DRBG (Deterministic Random Bit Generator) semeado pelo TRNG.
- **Armazenamento:**
  - Chave privada do dispositivo (Attestation Key) armazenada de forma segura (proteção de leitura na Flash).
  - Contador monotônico global persistente.
  - "Key Wrapping": Chaves privadas geradas para cada serviço não devem ser armazenadas internamente (para permitir armazenamento infinito), mas sim criptografadas e enviadas ao servidor (Key Handle), ou derivadas deterministicamente de uma chave mestre secreta.

## 3. Requisitos de Hardware

### 3.1. Microcontrolador (MCU)
- **Arquitetura:** Único chip (SoC).
- **USB:** Suporte nativo a USB 2.0 Full Speed (Device).
- **Recursos de Segurança:**
  - TRNG de hardware (Crítico).
  - Proteção de leitura de Flash (RDP/Flash Encryption).
- **Desempenho:** Capaz de realizar assinatura ECC P-256 em tempo razoável (< 1s, idealmente < 200ms).

### 3.2. Eletrônica
- **Alimentação:** 5V via USB.
- **Tensão Lógica:** 3.3V (LDO interno ou externo).
- **Proteção:** Proteção contra ESD nas linhas de dados USB.

## 4. Requisitos Não-Funcionais

### 4.1. Licenciamento
- **Hardware e Firmware:** 100% Open-Source.
- **Licença:** MIT ou Apache 2.0.

### 4.2. Custo e Fabricação
- **BOM (Bill of Materials):** Baixo custo (< $5 USD para componentes principais em lote).
- **Montagem:** Componentes SMD acessíveis para montagem manual ou PCBA de baixo custo.

## 5. Restrições
- Não utilizar chips de segurança proprietários (Secure Elements) fechados que exijam NDAs, a menos que haja uma versão "open" ou acessível (ex: ATECC608B é aceitável como opcional, mas o foco é MCU único).
