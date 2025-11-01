// Include necessary libraries
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// --- WiFi Configuration ---
const char* ssid = "Nani"; // Hotspot or Wifi Name
const char* password = "1234567 ";// Hotspot or Wifi Password

// --- Pin Definitions ---
// IR Sensors
const int gateIrPin = D3;
const int pad1IrPin = D0;
const int pad2IrPin = D1;
const int pad3IrPin = D2; 

// Servo Motor
const int servoPin = D4;

// --- Billing Configuration ---
const float costPerMinute = 2.5; // Example cost: 2.5 currency units per minute

// --- Global Variables ---
// Create server object on port 80
ESP8266WebServer server(80);

// Set up the LCD - address 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Create servo object
Servo gateServo;

// Gate positions
const int gateOpenAngle = 180;
const int gateClosedAngle = 0;

// Status tracking for charging pads
bool pad1_occupied = false;
bool pad2_occupied = false;
bool pad3_occupied = false;

// Time tracking for each pad (using millis)
unsigned long pad1_startTime = 0;
unsigned long pad2_startTime = 0;
unsigned long pad3_startTime = 0;

// To hold the current system status for the web dashboard
String currentStatusMessage = "Initializing..."; 
String lastBillMessages[3] = {"", "", ""}; // Holds last bill for each pad

// --- Setup Function ---
void setup() {
  Serial.begin(115200);
  
  // Initialize I2C on custom pins (SDA, SCL)
  Wire.begin(D5, D6); 

  // Initialize LCD
  lcd.begin(16, 2); 
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Welcome to EV");
  lcd.setCursor(0, 1);
  lcd.print("Charging Station");
  currentStatusMessage = "Welcome to EV Charging Station";
  delay(2000);

  // Initialize WiFi
  connectToWiFi();

  // Initialize pins with internal pull-up resistors to prevent floating inputs
  pinMode(gateIrPin, INPUT_PULLUP);
  pinMode(pad1IrPin, INPUT_PULLUP);
  pinMode(pad2IrPin, INPUT_PULLUP);
  pinMode(pad3IrPin, INPUT_PULLUP);

  // Attach servo
  gateServo.attach(servoPin);
  gateServo.write(gateClosedAngle); // Start with gate closed
  
  // Register all web routes, including the missing API endpoints
  setupWebServerRoutes(); 
  
  // Start the server
  server.begin();
  Serial.println("HTTP server started");
  updateLcdDisplay();
}

// --- Main Loop ---
void loop() {
  server.handleClient(); // Handle incoming client requests

  // Read sensor states
  // Assuming IR sensor outputs LOW when an object is detected
  bool gateSensorTriggered = (digitalRead(gateIrPin) == LOW);
  bool new_pad1_status = (digitalRead(pad1IrPin) == LOW);
  bool new_pad2_status = (digitalRead(pad2IrPin) == LOW);
  bool new_pad3_status = (digitalRead(pad3IrPin) == LOW);

  // --- Pad 1 Status Change Logic ---
  if (new_pad1_status != pad1_occupied) {
    if (new_pad1_status) { // Car has arrived
      pad1_occupied = true;
      pad1_startTime = millis();
    } else { // Car has left
      pad1_occupied = false;
      unsigned long duration = (millis() - pad1_startTime) / 1000;
      unsigned long minutes = duration / 60;
      unsigned long seconds = duration % 60;
      float bill = (duration / 60.0) * costPerMinute;
      
      // Update the message for the new web container
      lastBillMessages[0] = "Last Charge (Pad 1): " + String(minutes) + "m " + String(seconds) + "s. Bill: ₹" + String(bill, 2); // <-- MODIFIED
      
      // Show a simple Thank You message on the LCD (no billing)
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Thank You!");
      lcd.setCursor(0, 1);
      lcd.print("Pad 1 is Free");
      delay(4000); // Show message on LCD
    }
    updateLcdDisplay();
  }

  // --- Pad 2 Status Change Logic ---
  if (new_pad2_status != pad2_occupied) {
    if (new_pad2_status) { // Car has arrived
      pad2_occupied = true;
      pad2_startTime = millis();
    } else { // Car has left
      pad2_occupied = false;
      unsigned long duration = (millis() - pad2_startTime) / 1000;
      unsigned long minutes = duration / 60;
      unsigned long seconds = duration % 60;
      float bill = (duration / 60.0) * costPerMinute;

      lastBillMessages[1] = "Last Charge (Pad 2): " + String(minutes) + "m " + String(seconds) + "s. Bill: ₹" + String(bill, 2); // <-- MODIFIED
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Thank You!");
      lcd.setCursor(0, 1);
      lcd.print("Pad 2 is Free");
      delay(4000);
    }
    updateLcdDisplay();
  }

  // --- Pad 3 Status Change Logic ---
  if (new_pad3_status != pad3_occupied) {
    if (new_pad3_status) { // Car has arrived
      pad3_occupied = true;
      pad3_startTime = millis();
    } else { // Car has left
      pad3_occupied = false;
      unsigned long duration = (millis() - pad3_startTime) / 1000;
      unsigned long minutes = duration / 60;
      unsigned long seconds = duration % 60;
      float bill = (duration / 60.0) * costPerMinute;
      
      lastBillMessages[2] = "Last Charge (Pad 3): " + String(minutes) + "m " + String(seconds) + "s. Bill: ₹" + String(bill, 2); // <-- MODIFIED
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Thank You!");
      lcd.setCursor(0, 1);
      lcd.print("Pad 3 is Free");
      delay(4000);
    }
    updateLcdDisplay();
  }

  int occupiedPads = pad1_occupied + pad2_occupied + pad3_occupied;

  // Gate control logic
  if (gateSensorTriggered) {
    if (occupiedPads < 3) {
      // Find the first available pad
      int freePadNumber = 0;
      if (!pad1_occupied) {
        freePadNumber = 1;
      } else if (!pad2_occupied) {
        freePadNumber = 2;
      } else if (!pad3_occupied) {
        freePadNumber = 3;
      }

      // Display welcome and navigation message
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Welcome!");
      lcd.setCursor(0, 1);
      lcd.print("Go to Pad: " + String(freePadNumber));
      currentStatusMessage = "Welcome! Please proceed to Pad " + String(freePadNumber);

      Serial.println("Vehicle at gate, space available. Opening gate.");
      gateServo.write(gateOpenAngle);
      delay(4000); // Keep gate open and message displayed for 4 seconds
      gateServo.write(gateClosedAngle);
      
      delay(1000); // Brief delay before reverting to normal checks
      updateLcdDisplay(); // Show status again after gate sequence

    } else {
      Serial.println("Vehicle at gate, no space. Gate remains closed.");
      // Display "Full" message briefly on LCD
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Station is Full!");
      lcd.setCursor(0,1);
      lcd.print("Please wait...");
      currentStatusMessage = "Station is Full! Please wait.";
      delay(2000);
      updateLcdDisplay();
    }
  }

  // A small delay to prevent rapid, unnecessary updates
  delay(100); 
}

// --- WiFi Connection Function ---
void connectToWiFi() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");
  currentStatusMessage = "Connecting to WiFi...";
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  
  int dotCount = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    lcd.print(".");
    dotCount++;
    if(dotCount > 15) { // Reset line if it gets too long
      lcd.setCursor(0,1);
      lcd.print("                ");
      lcd.setCursor(0,1);
      dotCount = 0;
    }
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connected!");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  currentStatusMessage = "Connected! IP: " + WiFi.localIP().toString();
  delay(3000);
}


// --- LCD Update Function ---
void updateLcdDisplay() {
  int freePads = 3 - (pad1_occupied + pad2_occupied + pad3_occupied);
  int fullPads = (pad1_occupied + pad2_occupied + pad3_occupied);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Pads Free: " + String(freePads));
  lcd.setCursor(0, 1);
  lcd.print("Pads Full: " + String(fullPads));
  currentStatusMessage = "Pads Free: " + String(freePads) + " | Pads Full: " + String(fullPads);
}

// --- Web Server Handlers ---
// In the handleRoot() function...
void handleRoot() {
  // Build the HTML for the web page
  String html = "<!DOCTYPE html><html><head><title>EV Charging Station</title>";
  html += "<meta charset='UTF-8'>"; // <-- ADD THIS LINE
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>";
  
  // CSS Variables for consistent theming
  html += ":root {";
  html += "  --primary-green: #2d5016;";
  html += "  --secondary-green: #4a7c59;";
  html += "  --accent-green: #6faa6f;";
  html += "  --light-green: #a8e6a3;";
  html += "  --very-light-green: #e8f5e8;";
  html += "  --blue-accent: #4a90e2;";
  html += "  --orange-accent: #f39c12;";
  html += "  --red-accent: #e74c3c;";
  html += "  --white: #ffffff;";
  html += "  --light-gray: #f8f9fa;";
  html += "  --medium-gray: #6c757d;";
  html += "  --dark-gray: #343a40;";
  html += "  --shadow: 0 8px 32px rgba(45, 80, 22, 0.1);";
  html += "  --shadow-hover: 0 12px 48px rgba(45, 80, 22, 0.15);";
  html += "}";

  // Global styles
  html += "* { box-sizing: border-box; margin: 0; padding: 0; }";
  html += "body {";
  html += "  font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;";
  html += "  background: linear-gradient(135deg, #e8f5e8 0%, #d4f1d4 100%);";
  html += "  min-height: 100vh;";
  html += "  padding: 20px;";
  html += "  line-height: 1.6;";
  html += "}";

  // Main container
  html += ".main-container {";
  html += "  max-width: 1200px;";
  html += "  margin: 0 auto;";
  html += "  display: grid;";
  html += "  gap: 24px;";
  html += "  grid-template-areas: 'header' 'status' 'bills' 'pads';";
  html += "}";

  // Header section
  html += ".header-section {";
  html += "  grid-area: header;";
  html += "  background: linear-gradient(135deg, var(--primary-green) 0%, var(--secondary-green) 100%);";
  html += "  color: var(--white);";
  html += "  padding: 32px;";
  html += "  border-radius: 20px;";
  html += "  text-align: center;";
  html += "  box-shadow: var(--shadow);";
  html += "  position: relative;";
  html += "  overflow: hidden;";
  html += "}";

  html += ".header-section::before {";
  html += "  content: '';";
  html += "  position: absolute;";
  html += "  top: -50%;";
  html += "  left: -50%;";
  html += "  width: 200%;";
  html += "  height: 200%;";
  html += "  background: linear-gradient(45deg, transparent 30%, rgba(255,255,255,0.1) 50%, transparent 70%);";
  html += "  transform: rotate(45deg);";
  html += "  animation: shine 3s infinite;";
  html += "}";

  html += ".header-title {";
  html += "  font-size: 2.5rem;";
  html += "  font-weight: 700;";
  html += "  margin-bottom: 8px;";
  html += "  position: relative;";
  html += "  z-index: 1;";
  html += "}";

  html += ".header-subtitle {";
  html += "  font-size: 1.1rem;";
  html += "  opacity: 0.9;";
  html += "  position: relative;";
  html += "  z-index: 1;";
  html += "}";

  // System status section
  html += ".system-status-section {";
  html += "  grid-area: status;";
  html += "  background: var(--white);";
  html += "  border-radius: 16px;";
  html += "  padding: 24px;";
  html += "  box-shadow: var(--shadow);";
  html += "  transition: all 0.3s ease;";
  html += "}";

  html += ".system-status-section:hover {";
  html += "  transform: translateY(-2px);";
  html += "  box-shadow: var(--shadow-hover);";
  html += "}";

  html += ".status-title {";
  html += "  font-size: 1.3rem;";
  html += "  font-weight: 600;";
  html += "  color: var(--primary-green);";
  html += "  margin-bottom: 12px;";
  html += "}";

  html += ".status-message {";
  html += "  background: linear-gradient(135deg, var(--very-light-green) 0%, var(--light-green) 100%);";
  html += "  padding: 16px;";
  html += "  border-radius: 12px;";
  html += "  color: var(--primary-green);";
  html += "  font-weight: 500;";
  html += "  border-left: 4px solid var(--accent-green);";
  html += "}";

  // Bills section
  html += ".bills-section {";
  html += "  grid-area: bills;";
  html += "  background: var(--white);";
  html += "  border-radius: 16px;";
  html += "  padding: 24px;";
  html += "  box-shadow: var(--shadow);";
  html += "  transition: all 0.3s ease;";
  html += "}";

  html += ".bills-section:hover {";
  html += "  transform: translateY(-2px);";
  html += "  box-shadow: var(--shadow-hover);";
  html += "}";

  html += ".bills-title {";
  html += "  font-size: 1.3rem;";
  html += "  font-weight: 600;";
  html += "  color: var(--primary-green);";
  html += "  margin-bottom: 16px;";
  html += "}";

  html += ".bill-item {";
  html += "  display: flex;";
  html += "  justify-content: space-between;";
  html += "  align-items: center;";
  html += "  background: linear-gradient(135deg, var(--light-gray) 0%, #f0f4f0 100%);";
  html += "  padding: 16px;";
  html += "  margin-bottom: 12px;";
  html += "  border-radius: 12px;";
  html += "  border-left: 4px solid var(--orange-accent);";
  html += "  transition: all 0.3s ease;";
  html += "  animation: fadeIn 0.5s ease-out;";   
  html += "}";

  html += ".bill-item:hover {";
  html += "  transform: translateX(4px);";
  html += "  box-shadow: 0 4px 12px rgba(243, 156, 18, 0.2);";
  html += "}";

  html += ".bill-text {";
  html += "  flex: 1;";
  html += "  color: var(--dark-gray);";
  html += "  font-weight: 500;";
  html += "}";

  html += ".reset-btn {";
  html += "  background: linear-gradient(135deg, var(--orange-accent) 0%, #e67e22 100%);";
  html += "  color: var(--white);";
  html += "  padding: 8px 16px;";
  html += "  border-radius: 20px;";
  html += "  text-decoration: none;";
  html += "  font-size: 0.9rem;";
  html += "  font-weight: 500;";
  html += "  transition: all 0.3s ease;";
  html += "  border: none;";
  html += "  cursor: pointer;";
  html += "}";

  html += ".reset-btn:hover {";
  html += "  transform: scale(1.05);";
  html += "  box-shadow: 0 4px 12px rgba(243, 156, 18, 0.3);";
  html += "}";

  // Charging pads section
  html += ".pads-section {";
  html += "  grid-area: pads;";
  html += "  display: grid;";
  html += "  grid-template-columns: repeat(auto-fit, minmax(320px, 1fr));";
  html += "  gap: 20px;";
  html += "}";

  html += ".charging-pad {";
  html += "  background: var(--white);";
  html += "  border-radius: 16px;";
  html += "  padding: 24px;";
  html += "  box-shadow: var(--shadow);";
  html += "  transition: all 0.3s ease;";
  html += "  position: relative;";
  html += "  overflow: hidden;";
  html += "}";

  html += ".charging-pad:hover {";
  html += "  transform: translateY(-4px);";
  html += "  box-shadow: var(--shadow-hover);";
  html += "}";

  html += ".pad-header {";
  html += "  display: flex;";
  html += "  justify-content: space-between;";
  html += "  align-items: center;";
  html += "  margin-bottom: 16px;";
  html += "}";

  html += ".pad-title {";
  html += "  font-size: 1.2rem;";
  html += "  font-weight: 600;";
  html += "  color: var(--primary-green);";
  html += "}";

  html += ".status-badge {";
  html += "  padding: 6px 16px;";
  html += "  border-radius: 20px;";
  html += "  color: var(--white);";
  html += "  font-size: 0.9rem;";
  html += "  font-weight: 500;";
  html += "  transition: all 0.3s ease;";
  html += "  position: relative;";
  html += "}";

  html += ".status-available {";
  html += "  background: linear-gradient(135deg, var(--accent-green) 0%, #27ae60 100%);";
  html += "  animation: pulse-green 2s infinite;";
  html += "}";

  html += ".status-occupied {";
  html += "  background: linear-gradient(135deg, var(--red-accent) 0%, #c0392b 100%);";
  html += "  animation: pulse-red 2s infinite;";
  html += "}";

  html += ".pad-details {";
  html += "  background: linear-gradient(135deg, var(--very-light-green) 0%, #f8fdf8 100%);";
  html += "  padding: 16px;";
  html += "  border-radius: 12px;";
  html += "  border-left: 4px solid var(--blue-accent);";
  html += "}";

  html += ".detail-row {";
  html += "  display: flex;";
  html += "  justify-content: space-between;";
  html += "  margin-bottom: 8px;";
  html += "}";

  html += ".detail-row:last-child {";
  html += "  margin-bottom: 0;";
  html += "}";

  html += ".detail-label {";
  html += "  color: var(--medium-gray);";
  html += "  font-weight: 500;";
  html += "}";

  html += ".detail-value {";
  html += "  color: var(--dark-gray);";
  html += "  font-weight: 600;";
  html += "}";

  // Animations
  html += "@keyframes shine {";
  html += "  0% { transform: translateX(-100%) translateY(-100%) rotate(45deg); }";
  html += "  100% { transform: translateX(100%) translateY(100%) rotate(45deg); }";
  html += "}";

  html += "@keyframes pulse-green {";
  html += "  0%, 100% { box-shadow: 0 0 0 0 rgba(111, 170, 111, 0.7); }";
  html += "  50% { box-shadow: 0 0 0 10px rgba(111, 170, 111, 0); }";
  html += "}";

  html += "@keyframes pulse-red {";
  html += "  0%, 100% { box-shadow: 0 0 0 0 rgba(231, 76, 60, 0.7); }";
  html += "  50% { box-shadow: 0 0 0 10px rgba(231, 76, 60, 0); }";
  html += "}";

  html += "@keyframes slideInFromLeft {";
  html += "  0% { transform: translateX(-100%); opacity: 0; }";
  html += "  100% { transform: translateX(0); opacity: 1; }";
  html += "}";

  html += "@keyframes fadeIn {";
  html += "  0% { opacity: 0; transform: translateY(20px); }";
  html += "  100% { opacity: 1; transform: translateY(0); }";
  html += "}";

  // Responsive design
  html += "@media (max-width: 768px) {";
  html += "  .header-title { font-size: 2rem; }";
  html += "  .main-container { padding: 0 10px; gap: 16px; }";
  html += "  .header-section, .system-status-section, .bills-section, .charging-pad { padding: 20px; }";
  html += "  .pads-section { grid-template-columns: 1fr; }";
  html += "}";

  html += "</style>";
  
  // JavaScript for dynamic updates
  html += "<script>";
  html += "let updateInterval;";
  
  html += "async function updateSystemStatus() {";
  html += "  try {";
  html += "    const response = await fetch('/api/system-status');";
  html += "    const data = await response.text();";
  html += "    const statusElement = document.querySelector('.status-message');";
  html += "    if (statusElement && statusElement.innerHTML !== data) {";
  html += "      statusElement.style.animation = 'fadeIn 0.5s ease';";
  html += "      statusElement.innerHTML = data;";
  html += "    }";
  html += "  } catch (error) {";
  html += "    console.log('Status update failed:', error);";
  html += "  }";
  html += "}";

  html += "async function updateBills() {";
  html += "  try {";
  html += "    const response = await fetch('/api/bills');";
  html += "    const data = await response.text();";
  html += "    const billsContainer = document.querySelector('.bills-container');";
  html += "    if (billsContainer && billsContainer.innerHTML !== data) {";
  html += "      billsContainer.style.animation = 'fadeIn 0.5s ease';";
  html += "      billsContainer.innerHTML = data;";
  html += "    }";
  html += "  } catch (error) {";
  html += "    console.log('Bills update failed:', error);";
  html += "  }";
  html += "}";

  html += "async function updatePadStatus() {";
  html += "  try {";
  html += "    const response = await fetch('/status');";
  html += "    const data = await response.json();";
  html += "    ";
  html += "    ['pad1', 'pad2', 'pad3'].forEach((padName, index) => {";
  html += "      const padElement = document.getElementById(padName);";
  html += "      const isOccupied = data[padName + '_occupied'];";
  html += "      const statusBadge = padElement.querySelector('.status-badge');";
  html += "      const padDetails = padElement.querySelector('.pad-details');";
  html += "      ";
  html += "      if (isOccupied) {";
  html += "        statusBadge.className = 'status-badge status-occupied';";
  html += "        statusBadge.textContent = 'Occupied';";
  html += "        if (!padDetails) {";
  html += "          const detailsHTML = `<div class='pad-details'><div class='detail-row'><span class='detail-label'>Duration:</span><span class='detail-value' id='time-${padName}'>Calculating...</span></div><div class='detail-row'><span class='detail-label'>Current Bill:</span><span class='detail-value' id='bill-${padName}'>Calculating...</span></div></div>`;";
  html += "          padElement.querySelector('.pad-header').insertAdjacentHTML('afterend', detailsHTML);";
  html += "        }";
  html += "      } else {";
  html += "        statusBadge.className = 'status-badge status-available';";
  html += "        statusBadge.textContent = 'Available';";
  html += "        if (padDetails) {";
  html += "          padDetails.remove();";
  html += "        }";
  html += "      }";
  html += "    });";
  html += "  } catch (error) {";
  html += "    console.log('Pad status update failed:', error);";
  html += "  }";
  html += "}";

  html += "async function updateTimersAndBills() {";
  html += "  try {";
  html += "    const response = await fetch('/api/timers');";
  html += "    const data = await response.json();";
  html += "    ";
  html += "    Object.keys(data).forEach(padKey => {";
  html += "      const timeElement = document.getElementById('time-' + padKey);";
  html += "      const billElement = document.getElementById('bill-' + padKey);";
  html += "      if (timeElement && data[padKey]) {";
  html += "        timeElement.textContent = data[padKey].time;";
  html += "        billElement.textContent = '₹' + data[padKey].bill;"; // <-- MODIFIED
  html += "      }";
  html += "    });";
  html += "  } catch (error) {";
  html += "    console.log('Timer update failed:', error);";
  html += "  }";
  html += "}";

  html += "function startUpdates() {";
  html += "  updateSystemStatus();";
  html += "  updateBills();";
  html += "  updatePadStatus();";
  html += "  updateTimersAndBills();";
  html += "  ";
  html += "  updateInterval = setInterval(() => {";
  html += "    updateSystemStatus();";
  html += "    updateBills();";
  html += "    updatePadStatus();";
  html += "    updateTimersAndBills();";
  html += "  }, 2000);";
  html += "}";

  html += "document.addEventListener('DOMContentLoaded', startUpdates);";
  html += "document.addEventListener('visibilitychange', () => {";
  html += "  if (document.hidden) {";
  html += "    clearInterval(updateInterval);";
  html += "  } else {";
  html += "    startUpdates();";
  html += "  }";
  html += "});";
  html += "</script>";

  html += "</head><body>";
  html += "<div class='main-container'>";
  
  // Header Section
  html += "<div class='header-section'>";
  html += "<h1 class='header-title'>EV Charging Station</h1>";
  html += "<p class='header-subtitle'>Sustainable Energy for a Greener Tomorrow</p>";
  html += "</div>";

  // System Status Section
  html += "<div class='system-status-section'>";
  html += "<h2 class='status-title'>System Status</h2>";
  html += "<div class='status-message'>" + currentStatusMessage + "</div>";
  html += "</div>";

  // Bills Section
  html += "<div class='bills-section'>";
  html += "<h2 class='bills-title'>Recent Charging Sessions</h2>";
  html += "<div class='bills-container'>";
  for (int i = 0; i < 3; i++) {
    if (lastBillMessages[i] != "") {
      html += "<div class='bill-item'>";
      html += "<span class='bill-text'>" + lastBillMessages[i] + "</span>";
      html += "<a href='/reset?pad=" + String(i + 1) + "' class='reset-btn'>Clear</a>";
      html += "</div>";
    }
  }
  html += "</div>";
  html += "</div>";

  // Charging Pads Section
  html += "<div class='pads-section'>";
  
  unsigned long currentTime = millis();
  
  // Generate pad HTML for each charging pad
  for (int padNum = 1; padNum <= 3; padNum++) {
    bool isOccupied = (padNum == 1) ? pad1_occupied : (padNum == 2) ? pad2_occupied : pad3_occupied;
    unsigned long startTime = (padNum == 1) ? pad1_startTime : (padNum == 2) ? pad2_startTime : pad3_startTime;
    
    html += "<div class='charging-pad' id='pad" + String(padNum) + "'>";
    html += "<div class='pad-header'>";
    html += "<h3 class='pad-title'>Charging Pad " + String(padNum) + "</h3>";
    
    if (isOccupied) {
      html += "<span class='status-badge status-occupied'>Occupied</span>";
      html += "</div>";
      
      // Add charging details
      unsigned long duration = (currentTime - startTime) / 1000;
      unsigned long minutes = duration / 60;
      unsigned long seconds = duration % 60;
      float bill = (duration / 60.0) * costPerMinute;
      
      html += "<div class='pad-details'>";
      html += "<div class='detail-row'>";
      html += "<span class='detail-label'>Duration:</span>";
      html += "<span class='detail-value' id='time-pad" + String(padNum) + "'>" + String(minutes) + "m " + String(seconds) + "s</span>";
      html += "</div>";
      html += "<div class='detail-row'>";
      html += "<span class='detail-label'>Current Bill:</span>";
      html += "<span class='detail-value' id='bill-pad" + String(padNum) + "'>₹" + String(bill, 2) + "</span>"; // <-- MODIFIED
      html += "</div>";
      html += "</div>";
    } else {
      html += "<span class='status-badge status-available'>Available</span>";
      html += "</div>";
    }
    
    html += "</div>";
  }
  
  html += "</div>"; // Close pads-section
  html += "</div>"; // Close main-container
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}

// API endpoint for system status updates
void handleSystemStatus() {
  server.send(200, "text/plain", currentStatusMessage);
}

// API endpoint for bills updates
void handleBillsAPI() {
  String html = "";
  for (int i = 0; i < 3; i++) {
    if (lastBillMessages[i] != "") {
      html += "<div class='bill-item'>";
      html += "<span class='bill-text'>" + lastBillMessages[i] + "</span>";
      html += "<a href='/reset?pad=" + String(i + 1) + "' class='reset-btn'>Clear</a>";
      html += "</div>";
    }
  }
  server.send(200, "text/html", html);
}

// API endpoint for timer updates
void handleTimersAPI() {
  unsigned long currentTime = millis();
  String json = "{";
  
  if (pad1_occupied) {
    unsigned long duration = (currentTime - pad1_startTime) / 1000;
    unsigned long minutes = duration / 60;
    unsigned long seconds = duration % 60;
    float bill = (duration / 60.0) * costPerMinute;
    json += "\"pad1\":{\"time\":\"" + String(minutes) + "m " + String(seconds) + "s\",\"bill\":\"" + String(bill, 2) + "\"},";
  }
  
  if (pad2_occupied) {
    unsigned long duration = (currentTime - pad2_startTime) / 1000;
    unsigned long minutes = duration / 60;
    unsigned long seconds = duration % 60;
    float bill = (duration / 60.0) * costPerMinute;
    json += "\"pad2\":{\"time\":\"" + String(minutes) + "m " + String(seconds) + "s\",\"bill\":\"" + String(bill, 2) + "\"},";
  }
  
  if (pad3_occupied) {
    unsigned long duration = (currentTime - pad3_startTime) / 1000;
    unsigned long minutes = duration / 60;
    unsigned long seconds = duration % 60;
    float bill = (duration / 60.0) * costPerMinute;
    json += "\"pad3\":{\"time\":\"" + String(minutes) + "m " + String(seconds) + "s\",\"bill\":\"" + String(bill, 2) + "\"}";
  }
  
  // Remove trailing comma if present
  if (json.endsWith(",")) {
    json.remove(json.length() - 1);
  }
  
  json += "}";
  server.send(200, "application/json", json);
}


// Handle request to reset a bill
void handleReset() {
  String padToReset = server.arg("pad");
  if (padToReset == "1") {
    lastBillMessages[0] = "";
  } else if (padToReset == "2") {
    lastBillMessages[1] = "";
  } else if (padToReset == "3") {
    lastBillMessages[2] = "";
  }
  // Redirect back to the main page
  server.sendHeader("Location", "/");
  server.send(302, "text/plain", "");
}

// Handle status request (for API use)
void handleStatus() {
  String json = "{";
  json += "\"pad1_occupied\":" + String(pad1_occupied ? "true" : "false") + ",";
  json += "\"pad2_occupied\":" + String(pad2_occupied ? "true" : "false") + ",";
  json += "\"pad3_occupied\":" + String(pad3_occupied ? "true" : "false");
  json += "}";
  server.send(200, "application/json", json);
}

// This function defines ALL the routes your webpage needs to function dynamically.
void setupWebServerRoutes() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/reset", HTTP_GET, handleReset);
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/api/system-status", HTTP_GET, handleSystemStatus);
  server.on("/api/bills", HTTP_GET, handleBillsAPI);
  server.on("/api/timers", HTTP_GET, handleTimersAPI);
}