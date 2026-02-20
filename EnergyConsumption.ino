#include <iostream>
#include <chrono>
#include <thread>

using namespace std;
using namespace std::chrono;

int main() {
    // --- Prototype Charger Values ---
    float chargingVoltage = 3.7;   // volts (wireless prototype)
    float chargingCurrent = 2.0;   // amps
    float power = chargingVoltage * chargingCurrent; // watts

    // --- Sensor states ---
    bool pad_occupied = false;     // current sensor state
    bool previous_state = false;   // last sensor state

    // --- Energy variables ---
    steady_clock::time_point start_time;

    cout << "Waiting for vehicle...\n\n";

    while (true) {

        // 🔹 SIMULATION INPUT (CHANGE MANUALLY FOR TESTING)
        // Uncomment ONLY ONE at a time

        // pad_occupied = true;   // vehicle placed on pad
        // pad_occupied = false;  // vehicle removed from pad

        // ---- Vehicle just arrived ----
        if (pad_occupied && !previous_state) {
            start_time = steady_clock::now();
            cout << "Vehicle detected. Charging started." << endl;
        }

        // ---- Vehicle just left ----
        if (!pad_occupied && previous_state) {
            auto end_time = steady_clock::now();
            auto charging_time_sec =
                duration_cast<seconds>(end_time - start_time).count();

            // Energy calculation (Wh)
            float energy_consumed = (power * charging_time_sec) / 3600;

            cout << "Vehicle removed. Charging stopped." << endl;
            cout << "Energy Consumed: " << energy_consumed << " Wh\n" << endl;
        }

        previous_state = pad_occupied;

        // Delay of 1 second (same as time.sleep(1))
        this_thread::sleep_for(seconds(1));
    }

    return 0;
}
