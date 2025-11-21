# Guia de Manufatura (Produção em Massa)

## 1. Fabricação da PCB (Panelização)
Para reduzir custos de montagem (PCBA), a PCB deve ser fabricada em painéis.

- **Tamanho do Painel:** Recomendado 2x5 (10 unidades) ou 5x5 (20 unidades).
- **V-Cut:** Usar V-Cut (Scoring) para separar as placas após a montagem.
- **Fiduciais:** Adicionar marcas fiduciais nas bordas do painel (Rails) para alinhamento da máquina Pick & Place.
- **Tooling Holes:** Furos de 3mm nos cantos dos trilhos (Rails) para fixação.

## 2. Montagem (PCBA)
- **Stencil:** Usar stencil de aço inoxidável com espessura de 0.10mm ou 0.12mm.
- **Solda:** Pasta de solda Lead-Free (SAC305) para conformidade RoHS.
- **Componentes Críticos:**
  - **USB Connector:** Garantir solda robusta nos pads de fixação (Shield) para resistência mecânica.
  - **ESP32-S2:** Verificar curtos nos pinos do QFN/Módulo via Raio-X (se possível) ou inspeção visual rigorosa.

## 3. Gravação de Firmware (Flashing)
Existem duas estratégias para gravar o firmware em lote:

### Estratégia A: Pré-gravação (Recomendada para > 1k unidades)
Enviar o arquivo `.bin` combinado (Bootloader + Partition Table + App) para o fornecedor do chip ou distribuidor (ex: DigiKey/Mouser) para que gravem os chips ESP32 antes da soldagem.

### Estratégia B: Gravação na Linha (In-Circuit)
Usar um Hub USB industrial e um script Python para gravar múltiplos dispositivos simultaneamente.

**Comando de Gravação (Lote):**
```bash
esptool.py -p COMx -b 921600 write_flash 0x1000 bootloader.bin 0x8000 partition-table.bin 0x10000 openfido.bin
```

## 4. Controle de Qualidade (QC)
Cada unidade deve passar por um teste funcional "Go/No-Go" antes de ser embalada.
- **Teste 1:** Enumeração USB (Verificar VID/PID).
- **Teste 2:** Teste do Botão (Operador pressiona, LED fica Verde).
- **Teste 3:** Teste Criptográfico (Script roda um Register/Auth rápido).
