#pragma GCC optimize ("O0")
#pragma GCC optimize ("-fno-inline")

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <SPI.h>
#include <Adafruit_GFX.h>    
#include <Adafruit_ST7735.h> 

// Hardware Button Mapping (Safe GPIO Pins)
#define BTN_LR      4   // Button 1: Shifts cursor Right along the row
#define BTN_SELECT  5   // Button 2: Clicks and types/enters the highlighted key

// Display Hardware Configuration (Adafruit ST7735)
#define TFT_CS     10
#define TFT_DC      9
#define TFT_MOSI   11   // SDA Data line
#define TFT_SCLK   12   // SCL Clock line
#define TFT_RST    13   // RES Reset line

// =========================================================================
// 🌐 NETWORK & API CONFIGURATION (REPLACE WITH YOUR DETAILS BEFORE FLASHING)
// =========================================================================
const char* ssid      = "YOUR_WIFI_SSID";
const char* password  = "YOUR_WIFI_PASSWORD";
const char* apiKey    = "YOUR_GROQ_API_KEY"; // Secure gsk_... key format
const char* groqUrl   = "https://groq.com";

// 🧠 AI MODEL CONFIGURATION
// Swap this string parameter to use any active model on the Groq network cluster
const char* groqModel = "openai/gpt-oss-20b"; 
// =========================================================================

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// Smartphone Grid Layout Mapping (5 Rows x 8 Columns Matrix)
#define GRID_ROWS 5
#define GRID_COLS 8
const char gridKeypad[GRID_ROWS][GRID_COLS] = {
  {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H'},
  {'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P'},
  {'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X'},
  {'Y', 'Z', '1', '2', '3', '4', '5', '6'},
  {'7', '8', '9', '0', '_', '<', '=', 'M'} // '_' = Space, '<' = Delete, '=' = Enter, 'M' = Q Core Menu
};

// Interface State Variables
int currentGridCol = 0;
int currentGridRow = 0;
String currentInputText = "";
bool menuOpen = false;
bool displayResponseMode = false; // Tracks if the viewport is showing an active AI response

unsigned long lastButtonPressTime = 0;

void renderSystemLayout();
void refreshGridSelection();
void renderTranslucentMenu();
String queryGroq(String prompt);

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(BTN_LR, INPUT_PULLUP);
  pinMode(BTN_SELECT, INPUT_PULLUP);

  // Initialize display hardware pipeline
  tft.initR(INITR_BLACKTAB); 
  tft.setRotation(1); 
  tft.fillScreen(ST77XX_BLACK);
  
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(10, 10);
  tft.print("Booting Q Core Keyboard...");

  // Bring up local Wi-Fi transmission configurations with low-power safety margins
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.setMinSecurity(WIFI_AUTH_WPA_PSK); 
  WiFi.setTxPower(WIFI_POWER_8_5dBm); 
  WiFi.begin(ssid, password);
  
  unsigned long startWait = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - startWait < 12000)) { 
    delay(500); 
    yield();
  }

  renderSystemLayout();
}

void loop() {
  unsigned long currentMillis = millis();
  bool stateChanged = false;

  // BUTTON 1 INTERCEPT: Move cursor highlight right
  if (digitalRead(BTN_LR) == LOW) {
    if (currentMillis - lastButtonPressTime > 200) {
      lastButtonPressTime = currentMillis;

      // Rule: If displaying an AI response window, pressing ANY key exits back to layout typing mode
      if (displayResponseMode) {
        displayResponseMode = false;
        renderSystemLayout();
        return;
      }

      currentGridCol++;
      if (currentGridCol >= GRID_COLS) {
        currentGridCol = 0;
        currentGridRow = (currentGridRow + 1) % GRID_ROWS;
      }
      stateChanged = true;
    }
  }

  // BUTTON 2 INTERCEPT: Type/Select the hovered character element
  if (digitalRead(BTN_SELECT) == LOW) {
    if (currentMillis - lastButtonPressTime > 250) {
      lastButtonPressTime = currentMillis;

      // Rule: If displaying an AI response window, pressing ANY key exits back to layout typing mode
      if (displayResponseMode) {
        displayResponseMode = false;
        renderSystemLayout();
        return;
      }

      char keyAction = gridKeypad[currentGridRow][currentGridCol];
      
      if (keyAction == '=') { // ENTER KEY CLICKED -> EXECUTE HTTPS REQUEST
        tft.fillRect(0, 0, 160, 42, ST77XX_BLACK);
        tft.setCursor(4, 4);
        tft.setTextColor(ST77XX_CYAN);
        tft.print("Q Core Processing...");
        
        String response = queryGroq(currentInputText);
        
        // Render full viewport terminal reply
        tft.fillScreen(ST77XX_BLACK);
        tft.setCursor(4, 4);
        tft.setTextColor(ST77XX_WHITE);
        tft.print(response);
        
        currentInputText = "";        // Reset variable tray parameters for next cycle
        displayResponseMode = true;   // Shift core control into frozen display loop
        return;
      }
      else if (keyAction == '<') { // DELETE
        if (currentInputText.length() > 0) currentInputText.remove(currentInputText.length() - 1);
      }
      else if (keyAction == '_') { // SPACEBAR
        if (currentInputText.length() < 32) currentInputText += " ";
      }
      else if (keyAction == 'M') { // MENU TOGGLE
        menuOpen = !menuOpen;
        renderSystemLayout();
      }
      else { // TYPICAL ALPHANUMERIC CHARACTER ENTRY
        if (currentInputText.length() < 32) currentInputText += keyAction;
      }
      stateChanged = true;
    }
  }

  if (stateChanged && !displayResponseMode) {
    if (menuOpen) {
      renderTranslucentMenu();
    } else {
      // Re-render upper text box element tray window cleanly
      tft.fillRect(0, 0, 160, 42, ST77XX_BLACK);
      tft.drawFastHLine(0, 43, 160, ST77XX_WHITE);
      tft.setCursor(4, 4);
      tft.setTextColor(ST77XX_GREEN);
      tft.print("> " + currentInputText);
      refreshGridSelection();
    }
  }
  delay(10);
  yield();
}

void renderSystemLayout() {
  tft.fillScreen(ST77XX_BLACK);
  if (menuOpen) {
    renderTranslucentMenu();
    return;
  }
  
  tft.drawFastHLine(0, 43, 160, ST77XX_WHITE);
  tft.setCursor(4, 4);
  tft.setTextColor(ST77XX_GREEN);
  tft.print("> " + currentInputText);

  // Print localized layout typography maps inside the smartphone grid keyboard
  for (int r = 0; r < GRID_ROWS; r++) {
    for (int c = 0; c < GRID_COLS; c++) {
      int posX = c * 20 + 4;
      int posY = 48 + (r * 15);
      tft.setCursor(posX, posY);
      tft.setTextColor(ST77XX_WHITE);
      
      if (r == 4 && c == 4) tft.print("SPC");
      else if (r == 4 && c == 5) tft.print("DEL");
      else if (r == 4 && c == 6) tft.print("ENT");
      else tft.print(gridKeypad[r][c]);
    }
  }
  refreshGridSelection();
}

void refreshGridSelection() {
  for (int r = 0; r < GRID_ROWS; r++) {
    for (int c = 0; c < GRID_COLS; c++) {
      int posX = c * 20 + 1;
      int posY = 46 + (r * 15);
      if (r == currentGridRow && c == currentGridCol) {
        tft.drawRect(posX, posY, 19, 13, ST77XX_YELLOW); // Cursor box focus line highlight
      } else {
        tft.drawRect(posX, posY, 19, 13, ST77XX_BLACK);  
      }
    }
  }
}

void renderTranslucentMenu() {
  int mX = 10, mY = 20, mW = 140, mH = 90;
  
  // Custom alpha transparency simulation via alternate geometric dot grid mapping
  for (int y = mY; y < (mY + mH); y++) {
    for (int x = mX; x < (mX + mW); x++) {
      if ((x + y) % 2 == 0) tft.drawPixel(x, y, 0x000F); // Custom navy overlay
    }
  }
  tft.drawRect(mX, mY, mW, mH, ST77XX_BLUE);
  tft.setTextColor(ST77XX_CYAN);
  tft.setCursor(mX + 15, mY + 15);
  tft.print("LABEL: Q CORE");
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(mX + 15, mY + 40);
  tft.print("Status: Connected");
  tft.setCursor(mX + 15, mY + 60);
  tft.print("Select 'M' to Exit Menu");
}

String queryGroq(String prompt) {
  if (WiFi.status() != WL_CONNECTED) return "Link Offline";
  WiFiClientSecure client;
  client.setInsecure(); 
  HTTPClient http;
  
  if (!http.begin(client, groqUrl)) return "Host Err";
  http.addHeader("Authorization", "Bearer " + String(apiKey));
  http.addHeader("Content-Type", "application/json");
  
  String jsonBody = "{\"model\":\"" + String(groqModel) + "\",\"messages\":[{\"role\":\"user\",\"content\":\"" + prompt + "\"}]}";
  
  int httpResponseCode = http.POST(jsonBody);
  String result = "API Error";
  
  if (httpResponseCode > 0) {
    String payload = http.getString();
    int contentKeyIndex = payload.indexOf("\"content\":");
    if (contentKeyIndex != -1) {
      int startQuote = payload.indexOf("\"", contentKeyIndex + 10);
      int endQuote = payload.indexOf("\"", startQuote + 1);
      if (startQuote != -1 && endQuote != -1) {
        result = payload.substring(startQuote + 1, endQuote);
      }
    }
  } else {
    result = "HTTP Err: " + String(httpResponseCode);
  }
  http.end();
  return result;
}
