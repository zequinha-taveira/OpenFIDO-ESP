# OpenFIDO-ESP 🔐

**OpenFIDO-ESP** is an open-source, low-cost hardware security key (Authenticator) based on the **ESP32-S2** microcontroller. It supports **FIDO U2F** (CTAP1) and **FIDO2** (CTAP2/WebAuthn) protocols, allowing secure passwordless authentication on the web.

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![Status](https://img.shields.io/badge/status-Prototype-orange.svg)
![Build](https://github.com/YOUR_USERNAME/OpenFIDO-ESP/workflows/ESP-IDF%20Build/badge.svg)

## 🚀 Features

- **Protocol Support**:
  - **U2F (CTAP1)**: Fully compatible with legacy U2F (Chrome, Firefox, Linux, Windows).
  - **FIDO2 (CTAP2)**: Basic support for WebAuthn (Passwordless).
- **Hardware**:
  - Based on **ESP32-S2** (Native USB + Hardware Crypto).
  - **True Random Number Generator (TRNG)** for secure key generation.
  - **Hardware Accelerated ECC (P-256)** and **SHA-256**.
  - Minimal BOM (< $5 USD).
- **Security**:
  - **Key Wrapping**: Infinite key storage (keys are wrapped and stored on the server, not the device).
  - **User Presence**: Physical button requirement for all operations.

## 📂 Repository Structure

- `firmware/`: ESP-IDF firmware source code.
- `hardware/`: Schematics, BOM, and PCB guidelines.
- `tests/`: Python scripts for automated testing.
- `docs/`: Additional documentation.

## 🛠️ Getting Started

### Prerequisites
- **ESP-IDF v4.4+** installed.
- **Python 3** (for tests).

### Build & Flash
1. Navigate to the firmware directory:
   ```bash
   cd firmware
   ```
2. Build the project:
   ```bash
   idf.py build
   ```
3. Flash to the ESP32-S2 (Hold BOOT button while plugging in):
   ```bash
   idf.py -p COMx flash monitor
   ```

### Testing
Run the automated test suite:
```bash
pip install -r tests/requirements.txt
python tests/test_device.py
```

## 🔌 Hardware Connections

| ESP32-S2 Pin | Function | Note |
| :--- | :--- | :--- |
| **IO 19** | USB D- | Connect to USB D- |
| **IO 20** | USB D+ | Connect to USB D+ |
| **IO 0** | Button | Active Low (GND when pressed) |
| **IO 18** | LED | WS2812B Data In |

*See [hardware/schematic.md](hardware/schematic.md) for full details.*

## 📄 License

This project is licensed under the **MIT License** - see the [LICENSE](LICENSE) file for details.

---
*Disclaimer: This is a DIY project for educational and development purposes. Use at your own risk in production environments.*
