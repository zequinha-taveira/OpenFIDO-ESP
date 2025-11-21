# Design do Jig de Teste (Test Fixture)

Um "Jig de Teste" é uma ferramenta essencial para testar as placas rapidamente sem precisar soldar fios ou conectar cabos USB manualmente com cuidado excessivo.

## 1. Mecânica
- **Estrutura:** Impressão 3D (PLA/PETG) ou Usinagem CNC em Acrílico.
- **Agulhas de Teste (Pogo Pins):** Usar agulhas modelo **P75-E2** (cabeça cônica) ou **P75-H2** (cabeça serrilhada).
- **Alinhamento:** Pinos guia (Dowel Pins) que entram nos furos de montagem da PCB para garantir que os Pogo Pins toquem os Test Points (TPs) corretamente.

## 2. Pontos de Teste (Test Points) na PCB
A PCB deve ter pads circulares de cobre exposto (TP) de 1.0mm a 1.5mm de diâmetro para os seguintes sinais:

| Sinal | Função | Tipo de Pogo Pin |
| :--- | :--- | :--- |
| **VBUS** | Alimentação 5V | P75-E2 |
| **GND** | Terra | P75-E2 |
| **D+** | USB Data+ | P75-E2 |
| **D-** | USB Data- | P75-E2 |
| **IO0** | Boot Mode | P75-E2 |
| **RST** | Reset (EN) | P75-E2 |
| **TXD0** | UART Log | P75-E2 (Opcional) |
| **RXD0** | UART Log | P75-E2 (Opcional) |

## 3. Hardware do Jig
O Jig deve conter:
1.  **Hub USB:** Para conectar ao PC de teste.
2.  **Botão de "Start":** Para iniciar o script de teste no PC.
3.  **Grampo (Toggle Clamp):** Para pressionar a PCB contra os Pogo Pins com pressão constante.

## 4. Fluxo de Teste (Operador)
1.  Colocar a PCB no Jig.
2.  Fechar o Grampo (Clamp).
3.  O PC detecta a conexão USB (ou Serial se estiver gravando).
4.  Script roda automaticamente:
    - Grava Firmware (se virgem).
    - Verifica Enumeração.
    - Pede para pressionar botão (LED pisca).
    - Valida Crypto.
5.  Tela fica VERDE (Passou) ou VERMELHA (Falhou).
6.  Abrir Grampo e retirar PCB.
