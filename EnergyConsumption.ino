// --- Energy Consumption Variables ---
float chargingVoltage = 220.0;  // Adjust per your charger
float chargingCurrent = 5.0;    // Adjust per your charger
float pad1_energy = 0.0;
float pad2_energy = 0.0;
float pad3_energy = 0.0;

// --- Inside loop() ---
// Update energy only for pads that are currently charging
unsigned long currentMillis = millis();

if (pad1_occupied) {
    pad1_energy = chargingVoltage * chargingCurrent * ((currentMillis - pad1_startTime) / 3600000.0); // kWh
}

if (pad2_occupied) {
    pad2_energy = chargingVoltage * chargingCurrent * ((currentMillis - pad2_startTime) / 3600000.0); // kWh
}

if (pad3_occupied) {
    pad3_energy = chargingVoltage * chargingCurrent * ((currentMillis - pad3_startTime) / 3600000.0); // kWh
}

// --- Optional: print to Serial for testing ---
Serial.print("Pad1 Energy: "); Serial.print(pad1_energy, 2); Serial.println(" kWh");
Serial.print("Pad2 Energy: "); Serial.print(pad2_energy, 2); Serial.println(" kWh");
Serial.print("Pad3 Energy: "); Serial.print(pad3_energy, 2); Serial.println(" kWh");
