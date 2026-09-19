# Q CORE Terminal - Personal AI Assistant

An ultra-portable, hardware-driven personal AI assistant built using the **ESP32-S3**, a 1.8-inch ST7735 TFT display, and a custom 2-button smartphone-style grid keyboard. The device connects securely via a local Wi-Fi hotspot to stream raw data prompts straight to the high-performance **Groq Cloud API**.

🎥 **Project Video:** You can watch the full build walkthrough, wiring assembly, and live demonstrations on our official YouTube channel at **Q CORE ROBOTICS**.

---

## 🚀 Standout Features

- **Top-Aligned Output Tray:** Prompts and live terminal text entries remain pinned permanently to the top of the display viewport.
- **Smartphone Grid Keyboard:** Custom 5 × 8 matrix layout separating alphanumeric arrays from system macro keys (`SPC`, `DEL`, `ENT`).
- **2-Button Navigation Logic:** 
  - **Button 1 (GPIO 4):** Shifts the cursor highlight frame right. Automatically wraps downward to the next row at the boundary.
  - **Button 2 (GPIO 5):** Tactile validation key that clicks, executes, or types the currently highlighted selection.
- **Freeze-Frame Intercept Answer Screen:** The AI response stays safely locked on-screen until you tap any hardware button to resume typing mode.
- **Q CORE Frosted UI Menu:** Uses a retro pixel-dithered grid interlacing filter pattern to render a custom translucent overlay window labeled **"Q CORE"**.
- **Low-Power Precision Tuning:** Software power reduction patches throttle maximum radio transmit thresholds to prevent circuit voltage drops across typical USB connection links.

---

## 🔌 Hardware Circuit Connections

| Component | Pin Label | ESP32-S3 Target Pin | Wire Purpose |
| :--- | :--- | :--- | :--- |
| **1.8" TFT Display** | **GND** | **GND** | Common Ground |
| *(Adafruit ST7735)* | **VCC / VDD** | **3.3V** | Screen Logic Power |
| | **SCL / SCLK** | **GPIO 12** | SPI Hardware Clock Line |
| | **SDA / MOSI** | **GPIO 11** | SPI Hardware Data Line |
| | **RES / RST** | **GPIO 13** | Hardware Reset Pin |
| | **DC / RS / AO**| **GPIO 9** | Command / Data Toggle |
| | **CS** | **GPIO 10** | Chip Select Controller |
| | **BLK / LED** | **3.3V** | Backlight Array Power |
| **Button 1 (CYCLE)** | **Top-Left Leg** | **GPIO 4** | Column Right/Wrap Movement |
| *(4-Legged Tactile)* | **Bot-Right Leg**| **GND** | Ground Common Return |
| **Button 2 (SELECT)**| **Top-Left Leg** | **GPIO 5** | Immediate Character Click |
| *(4-Legged Tactile)* | **Bot-Right Leg**| **GND** | Ground Common Return |

> ⚠️ **Wiring Tip:** Ensure buttons are wired diagonally opposite across their 4 legs to prevent permanent grounding traps. Keep jumper wire arrays running down and away from the S3's black rectangular 3D ceramic antenna block to maximize Wi-Fi range.

---

## 🛠️ Required Software Dependencies

Before flashing the code using the Arduino IDE, open your **Library Manager** (`Ctrl + Shift + I`) and install the following core assets:

1. **Adafruit GFX Library** (by Adafruit)
2. **Adafruit ST7735 and ST7789 Library** (by Adafruit)

---

## 💻 Arduino IDE Build Configuration Profiles

To minimize compilation time and protect the hardware serial bus interface from unexpected lockouts, match these settings inside the **Tools** menu dropdown exactly:

- **Board:** `ESP32S3 Dev Module` (or your specific S3 variant layout profile)
- **USB CDC On Boot:** `Enabled` *(Crucial for active terminal diagnostics printouts)*
- **Upload Mode:** `USB-OTG CDC (Hardware)`
- **Flash Mode:** `DIO`
- **Compiler Directives Optimization:** The firmware relies on injected structural `#pragma GCC optimize ("O0")` directives to skip heavy compression processing, lowering verification run times down to ~5 seconds.

---

## 📡 Switching API Models

Changing the primary intelligence layer of your assistant is entirely modular. Simply modify the single text string global parameter near the top of your `.ino` script tab:

```cpp
// Update this target string identifier to point to any free Groq model
const char* groqModel = "openai/gpt-oss-20b"; 
```

Recommended production references include `"openai/gpt-oss-20b"` or `"qwen/qwen3.6-27b"`.

---

## 📜 Project Licensing

This project is open-source and maintained under the **MIT License**. Check out more robotics infrastructure designs and automated hardware code bases directly on **Q CORE ROBOTICS**.
# Personal-groq-AI-assistant
