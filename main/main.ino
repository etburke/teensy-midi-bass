#include <MIDI.h>

// --- Piezo / velocity config ---
// Wire each Ghost saddle preamp output to these pins (one per string).
// String order: E=0, A=1, D=2, G=3
const int PIEZO_PINS[4] = { A6, A7, A8, A9 };
const int NUM_STRINGS = 4;

// Peak values updated by the timer ISR, consumed at note-on.
volatile int piezoPeak[NUM_STRINGS] = { 0, 0, 0, 0 };
// Set true by main loop after consuming a peak so the ISR can reset it.
volatile bool peakConsumed[NUM_STRINGS] = { true, true, true, true };

IntervalTimer piezoTimer;

void samplePiezos() {
  for (int s = 0; s < NUM_STRINGS; s++) {
    if (peakConsumed[s]) {
      piezoPeak[s] = 0;
      peakConsumed[s] = false;
    }
    int val = analogRead(PIEZO_PINS[s]);
    if (val > piezoPeak[s]) {
      piezoPeak[s] = val;
    }
  }
}

// Returns MIDI velocity (1–127) from the current peak for the given string,
// then marks the peak as consumed so the ISR will reset it next cycle.
int consumeVelocity(int stringIndex) {
  int peak = piezoPeak[stringIndex];
  peakConsumed[stringIndex] = true;
  return max(1, map(peak, 0, 4095, 1, 127));
}

// --- Fret / note config ---
void setup() {
  analogReadResolution(12);
  // Sample piezos every 1ms via hardware timer.
  piezoTimer.begin(samplePiezos, 1000);
}

int analogPin = 0;
int raw = 0;
int noteRange = 22;
int rawNoteMap[23][2] = {
  { 4095, 40 },
  { 3100, 41 },
  { 3030, 42 },
  { 2950, 43 },
  { 2880, 44 },
  { 2800, 45 },
  { 2700, 46 },
  { 2610, 47 },
  { 2510, 48 },
  { 2400, 49 },
  { 2270, 50 },
  { 2140, 51 },
  { 1960, 52 },
  { 1780, 53 },
  { 1640, 54 },
  { 1490, 55 },
  { 1340, 56 },
  { 1170, 57 },
  {  980, 58 },
  {  763, 59 },
  {  550, 60 },
  {  280, 61 },
  {    0,  0 }
};

int findNoteBucket(int val, int index) {
  if (index < noteRange) {
    if (val > (rawNoteMap[index][0] - (rawNoteMap[index][0] - rawNoteMap[index + 1][0]) / 2)) {
      return rawNoteMap[index][1];
    }
    return findNoteBucket(val, index + 1);
  }
  return 0;
}

int noteFromRaw(int rawValue) {
  Serial.println(rawValue);
  int note = findNoteBucket(rawValue, 0);
  return note;
}

int mode(int array[], int arrayLength) {
  static int modeMap[4096];
  memset(modeMap, 0, sizeof(modeMap));
  int maxEl = array[0];
  int maxCount = 1;

  for (int i = 0; i < arrayLength; i++) {
    int el = array[i];
    modeMap[el]++;
    if (modeMap[el] > maxCount) {
      maxEl = el;
      maxCount = modeMap[el];
    }
  }
  return maxEl;
}

int prevNote = 0;
const int sampleSize = 1000;
int sampleCount = 0;
int samples[sampleSize];

// TODO: expand to per-string fret scanning; for now string 0 (E string) is used.
const int ACTIVE_STRING = 0;

void loop() {
  samples[sampleCount] = analogRead(analogPin);
  sampleCount = sampleCount + 1;

  if (sampleCount > sampleSize) {
    int sampleMode = mode(samples, sampleSize);
    int currNote = noteFromRaw(sampleMode);
    if (currNote != prevNote) {
      if (prevNote > 0) {
        usbMIDI.sendNoteOff(prevNote, 0, 1);
      }
      if (currNote > 0) {
        Serial.println(currNote);
        int velocity = consumeVelocity(ACTIVE_STRING);
        usbMIDI.sendNoteOn(currNote, velocity, 1);
      }
      prevNote = currNote;
    }
    sampleCount = 0;
  }

  while (usbMIDI.read()) {}
}
