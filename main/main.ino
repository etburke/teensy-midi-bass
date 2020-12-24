#include <MIDI.h>

void setup() {
  // put your setup code here, to run once:

}

int analogPin = 0;
int raw = 0;
int noteRange = 22;
int rawNoteMap[23][2] = {
  { 1024, 40 },
  { 775, 41 },
  { 757, 42 },
  { 738, 43 },
  { 720, 44 },
  { 699, 45 },
  { 667, 46 },
  { 643, 47 },
  { 619, 48 },
  { 592, 49 },
  { 562, 50 },
  { 530, 51 },
  { 481, 52 },
  { 437, 53 },
  { 401, 54 },
  { 360, 55 },
  { 320, 56 },
  { 276, 57 },
  { 231, 58 },
  { 182, 59 },
  { 133, 60 },
  { 70, 61 },
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

int prevNote = 0;

void loop() {
  usbMIDI.sendNoteOff(prevNote, 0, 1);
  raw = analogRead(analogPin);
  int currNote = noteFromRaw(raw);
  if (currNote > 0) {
    Serial.println(currNote);
    usbMIDI.sendNoteOn(currNote, 99, 1);
    prevNote = currNote; 
  }
  delay(100);
  // MIDI Controllers should discard incoming MIDI messages.
  while (usbMIDI.read()) {
  }
}
