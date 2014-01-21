#include <OptrexLCD.h>
#include <MsTimer2.h>

// This example displays an unsigned counter, starting from 2000,
// incrementing it with each LCD refresh (this is not too fast).
// Additionally, it will alternate with showing the word "Clear",
// then "Secure", and then both, and finally neither of those
// words (at the left of the display).
//
// Before running this, either wire the OptrexLCD IM50240
// (PWB50240-CEM) to the pins shown below, or modify the pin
// assignments to match your own wiring. The Optrex LCD
// wiring is documented in OptrexLCD.h 
//
// Note that the MsTimer2 used by this driver, uses timer 2

void
setup(void) {
  /*
   * Change these to your own ATmega168 I/O pin preferences, if necessary :
   * (The values shown are the defaults ATmega168 I/O pin numbers)
   */
  OptrexLCD::set_M_pin(2);        // Arduino D0 - Wired to 50% duty cycle clock (M), pin 4 on LCD PCB
  OptrexLCD::set_Shift_pin(3);    // Arduino D1 - Wired to Shift clock,              pin 3 on LCD PCB  
  OptrexLCD::set_Latch_pin(4);    // Arduino D2 - Wired to Latch clock,              pin 2 on LCD PCB
  OptrexLCD::set_Din_pin(5);      // Arduino D3 - Wired to Input data,               pin 1 on LCD PCB

  OptrexLCD::initialize();        // Start the OptrexLCD driver
}

void loop() {
  unsigned counter = 2000;              // Arbitrarily start our counter at 2000

  for(;;) {                             // Repeat forever, using our counter above..
    OptrexLCD::wait_latched();          // Block until a LCD latch event occurs (1 refresh)
    OptrexLCD::write_uint(counter++);   // Convert counter to 4 digits and display that until further notice

    switch ( counter % 4 ) {            // Calculate a remainder, and use that to...
    case 0 :                            // Show "CLEAR" but not "SECURE"
      OptrexLCD::set_clear(true);
      OptrexLCD::set_secure(false);
      break;
    case 1 :                            // Don't show "CLEAR", but show "SECURE"
      OptrexLCD::set_clear(false);
      OptrexLCD::set_secure(true);
      break;
    case 2 :                            // Show both "CLEAR" and "SECURE"
      OptrexLCD::set_clear(true);
      OptrexLCD::set_secure(true);
      break;
    case 3 :                            // Don't show "CLEAR" or "SECURE"
      OptrexLCD::set_clear(false);
      OptrexLCD::set_secure(false);
    }
  }
}

// End OptrexLCD driver example
