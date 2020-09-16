// =======================================================
// 
// Modified version of RicardoOliveira's FriendDetector:
// https://github.com/RicardoOliveira/FriendDetector
// 
// Modified by: JKCTech
// Date: 26-03-2019
// 
// =======================================================


//sudo chmod a+rw /dev/ttyUSB0



#include "esppl_functions.h"

// Settings
#define LIST_SIZE 4     // Amount of people in your list
#define USE_LEDS true   // Do you want to make use of LEDs as outputs?
#define COOLDOWN 1000 // Last seen timeout to 10 minutes
#define DEVICES 8


// Names of people to track
const String names[LIST_SIZE] = {
  "Person1",
  "Person2",
  "Person3",
  "Person4"
};

// Define their MAC addresses
const uint8_t macs[DEVICES][ESPPL_MAC_LEN] = {
  {0xB8, 0xD7, 0xAF, 0x77, 0x4F, 0x6B}, // note8 - G {0x18, 0x81, 0x0E, 0x42, 0xB3, 0x51} 
  {0x36, 0x38, 0x4A, 0x0C, 0x0F, 0xA6}, // note 10 plus
  {0xF8, 0x27, 0x93, 0x69, 0xBD, 0x58},
  {0x00, 0x27, 0x15, 0xD2, 0xE1, 0xFF},
  {0xB8, 0xD7, 0xAF, 0x77, 0x4F, 0x6B}, 
  {0x88, 0x28, 0xB3, 0xD0, 0x25, 0x9F}, 
  {0xF8, 0x27, 0x93, 0x69, 0xBD, 0x58},
  {0x00, 0x27, 0x15, 0xD2, 0xE1, 0xFF}
};

//Associate MAC ADDRESS to a Person
const int mac_to_person [DEVICES] =  {0, 0, 0, 0, 0, 0, 0};


// Pins per person (If USE_LEDS is set to false, don't bother about this)
const int pins[LIST_SIZE] = {
  D1, D2, D5, D6
};

// Timers to keep track of last seen
unsigned long timers[LIST_SIZE] = {
  0, 0, 0, 0
};

// Setup everything
void setup()
{
  Serial.begin(9600); // Setup serial

  // Set all pins for people to track
  for (int i = 0; i < LIST_SIZE; i++)
    pinMode(pins[i], OUTPUT);

  // Turn them all off (Just to be sure)
  for (int i = 0; i < LIST_SIZE; i++)
    digitalWrite(pins[i], LOW);

  esppl_init(cb); // Init esppl
  delay(2000);
}

// Main Loop
void loop() {
  esppl_sniffing_start(); // Start the sniffing >:)

  // Process every frame we can find
  while (true)
  {
    for (int i = ESPPL_CHANNEL_MIN; i <= ESPPL_CHANNEL_MAX; i++)
    {
      esppl_set_channel(i); // Scan on all available channels
      // Process them all but don't actually do anything in that loop
      while (esppl_process_frames()) {} 
    }
  }
}

// Function to compare mac addresses
bool maccmp(const uint8_t *mac1, const uint8_t *mac2)
{
  for (int i = 0; i < ESPPL_MAC_LEN; i++)
    if (mac1[i] != mac2[i])
      return false;
  return true;
}

// Function run by esppl
void cb(esppl_frame_info *info)
{
  unsigned long currentTime = millis(); // Current time
  
  // Iterate over MAC Addresses
  for (int i = 0; i < DEVICES; i++) {
    // If MAC address is receiver OR sender, person is found.
    if (maccmp(info->sourceaddr, macs[i]) || maccmp(info->receiveraddr, macs[i])) {
      // Print to serial
      Serial.printf("\nDetected %s (ID: %d, Pin: %d) MAC(%d)", names[mac_to_person[i]].c_str(), mac_to_person[i], pins[mac_to_person[i]], i);

      // Do we use the leds? If so, turn them on
      if (USE_LEDS) { turnon(pins[mac_to_person[i]]); }
      timers[mac_to_person[i]] = currentTime; // Set last seen to now
    }
    else
    {
      // If needed, check last seen on user and act accordingly
      if(USE_LEDS && timers[mac_to_person[i]] != 0 && currentTime - timers[mac_to_person[i]] > COOLDOWN)
      {
        timers[mac_to_person[i]] = 0; // Set last seen to 0
        turnoff(pins[mac_to_person[i]]); // Turn their LED off
      }
    }
  }
}

// Turn on pin X
void turnon(int pin) {
  digitalWrite(pin, HIGH);
}

// Turn off pin X
void turnoff(int pin)
{
  digitalWrite(pin, LOW);
}
