#include <MIDI.h>
//Serial en MIDI kunnen niet tegelijk.

// Define the select pins
const int selectPins[4] = {2, 3, 4, 5}; // Example pin numbers, update based on your wiring

// Define the analog input pins for each multiplexer
const int muxAnalogPins[5] = {A0, A1, A2, A3, A4}; // Example analog pins, update based on your wiring

// Channel counts per MUX
const int muxChannels[5] = {12, 12, 16, 16, 16};

// Channel remapping array
int remap[16] = {0, 8, 4, 12, 2, 10, 6, 14, 1, 9, 5, 13, 3, 11, 7, 15};

// MIDI setup
MIDI_CREATE_DEFAULT_INSTANCE();

// Function to set the select pins based on channel number
void setMuxChannel(int channel) {
  digitalWrite(selectPins[0], channel & 0x01);
  digitalWrite(selectPins[1], (channel >> 1) & 0x01);
  digitalWrite(selectPins[2], (channel >> 2) & 0x01);
  digitalWrite(selectPins[3], (channel >> 3) & 0x01);
}

// Function to read from all multiplexers
void readAllMuxes(int* muxValues) {
  for (int mux = 0; mux < 5; mux++) {
    for (int i = 0; i < muxChannels[mux]; i++) {
      //int channel = remap[i];
      int channel = i;
      setMuxChannel(channel);
      delayMicroseconds(50); // Allow some time for the mux to settle
      muxValues[mux * 16 + i] = analogRead(muxAnalogPins[mux]);
    }
  }
}

void setup() {
  // Initialize the select pins as outputs
  for (int i = 0; i < 4; i++) {
    pinMode(selectPins[i], OUTPUT);
  }

  // Initialize the MIDI library
  MIDI.begin(MIDI_CHANNEL_OFF);

  //Serial.begin(9600);
}

void loop() {
  static int lastMuxValues[80]; // Max 16 channels per MUX, 5 MUXs
  int muxValues[80]; // Current values

  // Read values from all multiplexers
  readAllMuxes(muxValues);

  // Check for changes and send MIDI messages
  for (int mux = 0; mux < 5; mux++) {
    for (int i = 0; i < muxChannels[mux]; i++) {
      int index = mux * 16 + i;
      int newValue = muxValues[index] / 8; // Scale to 0-127
      int threshold = 2; // Lower threshold for finer control
      if (abs(newValue - lastMuxValues[index]) > threshold) {
        MIDI.sendControlChange(index, newValue, 1); // Send on MIDI channel 1
        lastMuxValues[index] = newValue;
        //        Serial.print("MUX: ");
        //        Serial.print(mux);
        //        Serial.print(" Channel: ");
        //        Serial.print(i);
        //        Serial.print(" Value: ");
        //        Serial.println(newValue); // Debugging
      }
    }
  }

  delay(10); // Short delay for stability
}
