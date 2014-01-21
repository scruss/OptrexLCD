#include "Arduino.h"
#ifndef OptrexLCD_h
#define OptrexLCD_h 1

// #include <wiring.h>


/* LCD Driver for OPTREX IM50240 (Active Surplus, Toronto, ON)
 *
 * Written on September 22, 2008 by Warren W. Gay VE3WWG.
 * ve3wwg@gmail.com
 *
 * Software License: Public Domain
 *
 * 4 Digit (7 Segment) LCD display, with
 * no decimal point, and two special displays
 * of the word "Secure" and/or "Clear". 
 *
 * It probably was an alarm system LCD display
 * for numeric code entry.
 *
 * 50% duty cycle clock (30 - 200Hz)
 *
 * The Optrex PCB interface pinout is as follows:
 * 
 *  1 - input data              (Din)
 *  2 - latch clock             (Latch)
 *  3 - shift clock             (Shift)
 *  4 - input square waves of
 *      50% duty cycle          (M)
 *  5 - NC
 *  6 - Vcc +5v                 Power
 *  7 - GND -                   Ground
 *  8 - SW1 CLEAR               Push button foil pattern (LOW when shorted)
 *  9 - SW2 SECURE              Push button foil pattern (LOW when shorted)
 * 10 - CHASSIS GND             Ground
 *
 * Organized as 5 bytes of data (40 bits)
 *
 * Digit 0 (byte 0) activates the SECURE and/or CLEAR displays.
 *
 *      0x02    CLEAR
 *      0x04    SECURE
 *
 * Bytes 1 to 4 are digits 4 to 1 (reversed).
 *
 * LCD SEGMENTS :
 * ==============
 *
 * MSB              LSB
 *   7 6 5 4  3 2 1 0
 *   c b d e  f g a x
 *
 * where:
 *        a
 *       ---
 *     b|   |g
 *       -c-    
 *     d|   |f
 *       ---
 *        e            x = don't care (not used)
 *
 * USER APPLICATION USAGE OF THIS LIBRARY :
 * ========================================
 *
 *  Within setup() :
 *  ----------------
 *
 *  1.  You must #include <MsTimer2.h>, since it is used by
 *      this driver.
 *  2.  Configure your I/O pins using calls to:
 *
 *      OptrexLCD::set_M_pin(byte pin_M);
 *      OptrexLCD::set_Shift_pin(byte pin_Shift);
 *      OptrexLCD::set_Latch_pin(byte pin_Latch);
 *      OptrexLCD::set_Din_pin(byte pin_Din);
 *
 *  3.  Optional and not normally recommended, configure:
 *
 *      OptrexLCD::set_timer_ms(byte msec);
 *
 *  4.  Optional but not normally required, establish a "latch callback":
 *
 *      OptrexLCD::set_latch_callback(volatile void (*usercb)(void));
 *
 *  5.  Start the LCD display driver :
 *
 *      OptrexLCD::initialize();
 *
 *  Within loop() :
 *  ---------------
 *
 *  Invoke any of the following routines as required :
 *
 *  OptrexLCD::clear();
 *  OptrexLCD::write(char text[4]);
 *  OptrexLCD::write_int(int v);
 *  OptrexLCD::write_uint(unsigned v);
 *
 *  OptrexLCD::set_secure(boolean b);
 *  OptrexLCD::set_clear(boolean b);
 *
 *  To pause your program until the current display information is
 *  latched, invoke :
 *
 *  OptrexLCD::wait_latched();
 *
 *  Example :
 *  ---------
 * 
 *  void loop(void) {
 *      unsigned counter = 0;
 * 
 *      OptrexLCD::clear();
 *
 *      for (;;) {
 *          OptrexLCD::write_uint(counter++);
 *          OptrexLCD::wait_latched();
 *      }
 *  }
 */
namespace OptrexLCD {
    //
    // Optrex LCD Configuration (pre initialization() only)
    //
    void set_M_pin(byte pin_M);                              // Defaults to pin 2
    void set_Shift_pin(byte pin_Shift);                      // Defaults to pin 3
    void set_Latch_pin(byte pin_Latch);                      // Defaults to pin 7
    void set_Din_pin(byte pin_Din);                          // Defaults to pin 10
    void set_timer_ms(byte msec);                            // Alter timer ms value (default = 3), range 1-255
    void set_latch_callback(volatile void (*usercb)(void));  // Set a user callback, called after the display is latched

    //
    // Optrex LCD Initialization and Startup
    //
    void initialize(void);                                   // Start display driver

    //
    // Optrex LCD Write routines
    //
    void clear(void);                                        // Clear entire display, including "Secure" and "Clear" words
    void write(char text[4]);                                // Write ASCII text to 4 digit portion of display
    void write_int(int v);                                   // Write a signed int to 4 digits (range 9999 to -999)
    void write_uint(unsigned v);                             // Write an unsigned int to 4 digits

    //
    // Control of "Secure" and "Clear" displays :
    //
    void set_secure(boolean b);                              // Enable/Disable display of word "Secure"
    void set_clear(boolean b);                               // Enable/Disable display of word "Clear"

    //
    // Special purpose
    //
    void wait_latched(void);                                 // Block until data is latched in LCD display
}

#endif // OptrexLCD_h */

/* End OptrexLCD.h */
