 /*
   MIDIUSB_test.ino

   Created: 4/6/2015 10:47:08 AM
   Author: gurbrinder grewal
   Modified by Arduino LLC (2015)
*/

#include "MIDIUSB.h"


const uint8_t ACTIVE = HIGH;
const uint8_t SILENT = LOW;

#define USE_DEBUG  1 

// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).

// Define the list of I/O pins used by the application to play notes.
// Put all the I/O PIN numbers here!
const uint8_t PROGMEM pinIO[] = { 2,3,4,5,6,7,8,9,10,11,12,13 };

// Define an index into the I/O pin array each MIDI note.
// Put the array index of the pin in the pinIO array here (eg, 2 if the  
// note is played using the third pin defined in the array). This is used 
// by playNote() to work out which pin to drive ACTIVE or SILENT.
const uint8_t PROGMEM noteIO[128] =
{
  // C  C#   D  D#   E   F   F#  G   G#  A  A#  B       Notes 
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, //   0- 11: Octave  0
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, //  12- 23: Octave  1
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11,  //  24- 35: Octave  2
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, //  36- 47: Octave  3
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, //  48- 59: Octave  4
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, //  60- 71: Octave  5
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, //  72- 83: Octave  6
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, //  84- 95: Octave  7
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, //  96-107: Octave  8
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, // 108-119: Octave  9
     0,  1,  2,  3,  4,  5,  6,  7                      // 120-127: Octave 10
};

const char* pitch_name(byte pitch) {
  static const char* names[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
  return names[pitch % 12];
}

int pitch_octave(byte pitch) {
  return (pitch / 12) - 2;
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println("Setup 1");

  for (uint8_t i = 0; i < 3; i++){
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
  }
  
  Serial.println("Setup 2");

  Serial.println(sizeof(pinIO));
  
  for (uint8_t i = 0; i < sizeof(pinIO) - 1; i++){
    uint8_t pin = pgm_read_byte(pinIO + i);
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    
    Serial.print("init pin: ");
    Serial.println(pin);
  }

}


void playNote(uint8_t note, bool state)
{
  if (note > 128) return;

  uint8_t idx = pgm_read_byte(noteIO + note);
  uint8_t pin = pgm_read_byte(pinIO + idx);

  digitalWrite(pin, state);

#if USE_DEBUG 
  Serial.print("pin: ");
  Serial.print(pin);
  Serial.print(",  state: ");
  Serial.println(state);
#endif
}


void noteOn(byte channel, byte pitch, byte velocity) {
  digitalWrite(LED_BUILTIN, HIGH);

#if USE_DEBUG 
  Serial.print("Note On: ");
  Serial.print(pitch_name(pitch));
  Serial.print(pitch_octave(pitch));
  Serial.print(", channel=");
  Serial.print(channel);
  Serial.print(", velocity=");
  Serial.println(velocity);
#endif

  playNote(pitch, ACTIVE);

}

void noteOff(byte channel, byte pitch, byte velocity) {
  digitalWrite(LED_BUILTIN, LOW);

#if USE_DEBUG 
  Serial.print("Note Off: ");
  Serial.print(pitch_name(pitch));
  Serial.print(pitch_octave(pitch));
  Serial.print(", channel=");
  Serial.print(channel);
  Serial.print(", velocity=");
  Serial.println(velocity);
#endif

  playNote(pitch, SILENT);
  
}
// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}

void loop() {
  midiEventPacket_t rx;
  do {
    rx = MidiUSB.read();
    if (rx.header != 0) {
      switch (rx.header) {
        case 0:
          break; //No pending events

        case 0x9:
          noteOn(
            rx.byte1 & 0xF,  //channel
            rx.byte2,        //pitch
            rx.byte3         //velocity
          );
          break;

        case 0x8:
          noteOff(
            rx.byte1 & 0xF,  //channel
            rx.byte2,        //pitch
            rx.byte3         //velocity
          );
          break;

        case 0xB:
          controlChange(
            rx.byte1 & 0xF,  //channel
            rx.byte2,        //control
            rx.byte3         //value
          );
          break;

        default:
          Serial.print("Unhandled MIDI message: ");
          Serial.print(rx.header, HEX);
          Serial.print("-");
          Serial.print(rx.byte1, HEX);
          Serial.print("-");
          Serial.print(rx.byte2, HEX);
          Serial.print("-");
          Serial.println(rx.byte3, HEX);
      }
    }
  } while (rx.header != 0);
}
