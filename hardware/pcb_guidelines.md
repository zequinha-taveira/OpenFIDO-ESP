# Diretrizes de Layout da PCB (Guidelines)

## 1. Interface USB (Crítico)
- **Par Diferencial:** As trilhas D+ e D- devem ser roteadas como um par diferencial.
- **Impedância:** Tente manter a impedância diferencial em **90 Ohms**.
  - Para PCB FR-4 padrão (1.6mm), trilhas de ~0.25mm com espaçamento de ~0.15mm geralmente funcionam bem (use uma calculadora de impedância como Saturn PCB Toolkit).
- **Comprimento:** Mantenha as trilhas D+ e D- com o mesmo comprimento (length matching) com tolerância de < 5mm.
- **Proteção ESD:** O chip USBLC6-2 deve ser colocado o mais próximo possível do conector USB. As trilhas devem ir do Conector -> ESD -> MCU (sem "stubs").

## 2. Alimentação (Power)
- **Capacitores de Desacoplamento:**
  - Coloque o capacitor de 100nF o mais próximo possível do pino 3V3 do ESP32.
  - Coloque os capacitores de entrada e saída do LDO próximos aos pinos do LDO.
- **Trilhas de Energia:** Use trilhas mais largas (min 0.5mm) para VBUS e 3V3.
- **Plano de Terra (GND):** Use um plano de terra sólido na camada inferior (Bottom Layer) e conecte os componentes ao GND com vias curtas.

## 3. Antena (Se usar Wi-Fi futuramente)
- O ESP32-S2-MINI-1 tem antena na PCB.
- **Keep-out Zone:** Não coloque cobre (trilhas, planos ou componentes) sob a área da antena do módulo em NENHUMA das camadas da PCB.
- Deixe a antena "para fora" da borda da placa se possível.

## 4. Mecânica
- **Botão:** Posicione o botão em um local acessível para o dedo do usuário.
- **LED:** Posicione o LED onde possa ser visto (ou use um guia de luz/furo na case).
- **Formato:** O formato "Stick" (pen drive) é o mais comum. Largura típica de 14mm a 18mm.

## 5. Fabricação (DFM)
- **Tamanho Mínimo:** Componentes 0402 ou 0603 são bons para montagem manual.
- **Vias:** Tamanho padrão 0.3mm furo / 0.6mm anel.
