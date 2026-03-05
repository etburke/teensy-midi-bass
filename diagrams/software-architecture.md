# Software Architecture

## Component Overview

```mermaid
graph TD
    subgraph ISR["IntervalTimer ISR — fires every 1ms"]
        S_READ["analogRead A6–A9\n(all 4 piezo pins)"]
        S_PEAK["Update piezoPeak per string\n(rolling max)"]
        S_RST["Reset peak if peakConsumed flag set"]
        S_READ --> S_PEAK --> S_RST
    end

    subgraph MAIN["Main Loop"]
        M_FRET["analogRead A0\naccumulate 1000 samples"]
        M_MODE["mode() — find most frequent\nADC value in sample window"]
        M_NOTE["noteFromRaw()\nfindNoteBucket() — binary search\nlookup table → MIDI note number"]
        M_DIFF["currNote != prevNote?"]
        M_OFF["usbMIDI.sendNoteOff(prevNote)"]
        M_VEL["consumeVelocity(stringIndex)\nread piezoPeak, map 0–4095 → 1–127\nset peakConsumed = true"]
        M_ON["usbMIDI.sendNoteOn(currNote, velocity)"]
        M_FLUSH["usbMIDI.read() — discard\nincoming MIDI"]

        M_FRET --> M_MODE --> M_NOTE --> M_DIFF
        M_DIFF -->|"yes, prevNote > 0"| M_OFF
        M_DIFF -->|"yes, currNote > 0"| M_VEL --> M_ON
        M_DIFF -->|"no change"| M_FLUSH
        M_OFF --> M_FLUSH
        M_ON --> M_FLUSH
    end

    subgraph SHARED["Shared Volatile State"]
        V_PEAK["piezoPeak[4]\n(volatile int)"]
        V_CONS["peakConsumed[4]\n(volatile bool)"]
    end

    ISR -->|"writes"| V_PEAK
    ISR -->|"reads/clears"| V_CONS
    MAIN -->|"reads"| V_PEAK
    MAIN -->|"sets"| V_CONS
```

## Timing Diagram

```mermaid
sequenceDiagram
    participant Timer as IntervalTimer (1ms)
    participant Peak as piezoPeak[]
    participant Loop as Main Loop
    participant MIDI as USB MIDI

    Note over Timer,Peak: String is plucked
    Timer->>Peak: analogRead → store max (burst captured over ~10–50ms)
    Timer->>Peak: analogRead → no change (peak already higher)
    Timer->>Peak: analogRead → no change

    Note over Loop: 1000-sample fret window completes
    Loop->>Loop: mode() → noteFromRaw() → currNote changed
    Loop->>MIDI: sendNoteOff(prevNote)
    Loop->>Peak: consumeVelocity() — read peak, map to 1–127
    Peak-->>Loop: velocity value
    Loop->>Peak: set peakConsumed = true
    Loop->>MIDI: sendNoteOn(currNote, velocity)

    Note over Timer,Peak: Next tick — ISR sees peakConsumed, resets piezoPeak to 0
    Timer->>Peak: reset → 0
```

## Data Flow

```mermaid
flowchart LR
    FRET["Fret press\n(conductive contact)"] -->|"3.3V via mux"| ADC0["A0 ADC\n12-bit"]
    ADC0 --> SAMP["1000-sample\naccumulator"]
    SAMP --> MODE["mode filter\n(most frequent value)"]
    MODE --> LUT["Lookup table\nADC value → MIDI note"]

    SADDLE["Ghost saddle\n(piezo burst)"] --> OPAMP["Op-amp buffer\n+ peak rectifier"]
    OPAMP --> ADC6["A6–A9 ADC\n12-bit"]
    ADC6 -->|"every 1ms"| ISR["IntervalTimer ISR\nrolling peak"]
    ISR -->|"at note-on"| VEL["velocity\n1–127"]

    LUT --> NOTE["MIDI note\n40–61"]
    NOTE --> MIDI["usbMIDI\nnoteOn / noteOff"]
    VEL --> MIDI
    MIDI --> USB["USB MIDI\nto DAW"]
```
