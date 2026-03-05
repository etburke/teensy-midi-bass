#include <MIDI.h>

void setup() {
  // put your setup code here, to run once:
  analogReadResolution(12);
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
  { 980, 58 },
  { 763, 59 },
  { 550, 60 },
  { 280, 61 },
  { 0, 0 }
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
    // modeMap is indexed by ADC value (0-4095 for 12-bit resolution)
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
        usbMIDI.sendNoteOn(currNote, 99, 1);
      }
      prevNote = currNote;
    }
    sampleCount = 0;
  }
  // MIDI Controllers should discard incoming MIDI messages.
  while (usbMIDI.read()) {
  }
}
