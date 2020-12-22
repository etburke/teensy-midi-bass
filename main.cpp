#include <MIDI.h>

void setup() {
  // put your setup code here, to run once:

}

int analogPin = 0;
int raw = 0;
int Vin = 5;
float Vout = 0;
float R1 = 2700;
float R2 = 0;
float buffer = 0;

float getResistance() {
  raw = analogRead(analogPin);
  if (raw) {
    buffer = raw * Vin;
    Vout = (buffer) / 1024.0;
    buffer = (Vin / Vout) - 1;
    R2 = R1 * buffer;
  }
  return R2;
}

int minResistance = 0;
int maxResistance = 44968;
int minMidiNote = 40;
int maxMidiNote = 61;
int noteIntervals = maxMidiNote - minMidiNote;
int resistanceInterval = maxResistance / noteIntervals;

int midiNoteFromResistance(int resistance) {
  float ratio = (float(resistance) + float(resistanceInterval / 2)) / (float(maxResistance));
  Serial.println(ratio);
  if (ratio <= 1) {
    int midiRange = maxMidiNote - minMidiNote;
    return minMidiNote + (ratio * midiRange);  
  }
  return 0;
}

int prevNote = 0;

void loop() {
  // put your main code here, to run repeatedly:
  usbMIDI.sendNoteOff(prevNote, 0, 1);
  float sampledResistance = getResistance();
  int currNote = midiNoteFromResistance(sampledResistance);
  
  Serial.print("Sampled Resistance: ");
  Serial.println(sampledResistance);

  Serial.print("Midi Note: ");
  Serial.println(currNote);
  
  usbMIDI.sendNoteOn(currNote, 99, 1);
  prevNote = currNote;
  delay(1000);
  // MIDI Controllers should discard incoming MIDI messages.
  while (usbMIDI.read()) {
  }
}
