#include <OptrexLCD.h>
#include <MsTimer2.h>

/*
 * Written on September 22, 2008 by Warren W. Gay VE3WWG.
 * ve3wwg@gmail.com
 *
 * Software License: Public Domain
 */
namespace OptrexLCD {

    /*
     * Pin Assignments : Alter these prior to calling OptrexLCD::initialize()
     */
    static byte LCD_M            =  2;         // LCD clock (30-90 Hz) pin #
    static byte LCD_Shift        =  3;         // Shift clock pin #
    static byte LCD_Latch        =  7;         // Data latch pin #
    static byte LCD_Din          = 10;         // LCD Data in pin #

    static byte LCD_Shift_ms     =  3;         // Time in milliseconds for each shift clock change

    static volatile byte LCD_bits    = 40;     // Bits to shift out
    static volatile byte bitbuf[5];            // LCD bit buffer (40 bits of segment select)

    static volatile byte M_State     =  0;     // M state
    static volatile byte Shift_State =  0;     // Shift line state
    static volatile byte Latch_State =  0;     // Current latch state

    static volatile void (*latchcb)(void) = 0; // Optional user callback after each latch

/*
 * Mapping table for ASCII to 7-segment display, starting
 * with '0' :
 */
static byte segmap[] = {
    0x7E,      // '0'
    0x0C,      // '1'
    0xB6,      // '2'
    0x9E,      // '3'
    0xCC,      // '4'
    0xDA,      // '5'
    0xFA,      // '6'
    0x0E,      // '7'
    0xFE,      // '8'
    0xDE,      // '9'
    0xEE,      // 'A'
    0xF8,      // 'b'
    0x72,      // 'C'
    0xBC,      // 'D'
    0xF2,      // 'E'
    0xE2,      // 'F'
    0x80,      // '-'                   OPTREX_SEG_MINUS
    0x10,      // '_' (underscore)      OPTREX_SEG_UNDERSCORE
    0x02,      // Clear (digit 0 only)  OPTREX_SEG_CLEAR
    0x04       // Secure(digit 0 only)  OPTREX_SEG_SECURE
};

#   define OPTREX_SEG_MINUS         16
#   define OPTREX_SEG_UNDERSCORE    17
#   define OPTREX_SEG_CLEAR         18
#   define OPTREX_SEG_SECURE        19
    
static byte data[] = { 0x02, 0x00, 0x00, 0x00, 0x00 };

static void timer_routine(void);       
static void show_overflow(void);       
static byte ascii2seg(byte ascii_ch);

/*
 * Establish which ATmega168 I/O pin is used for the
 * M clock. Default = 2.
 */
void
set_M_pin(byte pin_M) {
    LCD_M = pin_M;
}

/*
 * Establish which ATmega168 I/O pin is used for the data
 * shift clock (M freq X 2). AKA CLK2. Default = 3.
 */
void
set_Shift_pin(byte pin_Shift) {
    LCD_Shift = pin_Shift;
}

/*
 * Establish which ATmega168 I/O pin is used for the
 * latch signal. Default = 7.
 */
void
set_Latch_pin(byte pin_Latch) {
    LCD_Latch = pin_Latch;
}

/*
 * Establish which ATmega168 I/O pin is used for the
 * data in (Din) line. Default = 10.
 */
void
set_Din_pin(byte pin_Din) {
    LCD_Din = pin_Din;
}

/*
 * Alter the timer value in milliseconds.
 * Default = 3 msec.
 */
void
set_timer_ms(byte msec) {
    if ( msec > 0 )
        LCD_Shift_ms = msec;
}

/*
 * This routine initializes the timer and starts the
 * LCD refresh routine. Make sure that you assign the
 * correct pin numbers first, or accept the defaults.
 */
void
initialize(void) {
    pinMode(OptrexLCD::LCD_M,OUTPUT);
    pinMode(OptrexLCD::LCD_Shift,OUTPUT);
    pinMode(OptrexLCD::LCD_Latch,OUTPUT);
    pinMode(OptrexLCD::LCD_Din,OUTPUT);

    digitalWrite(OptrexLCD::LCD_M,LOW);
    digitalWrite(OptrexLCD::LCD_Shift,LOW);
    digitalWrite(OptrexLCD::LCD_Latch,LOW);
    digitalWrite(OptrexLCD::LCD_Din,LOW);

    MsTimer2::set(OptrexLCD::LCD_Shift_ms,timer_routine);
    MsTimer2::start();
}

/*
 * Install an optional user callback. When established,
 * the callback function usercb() is invoked after
 * every latching of information in the display.
 *
 * For example, this can be used to increment a
 * user counter.
 */
void
set_latch_callback(volatile void (*usercb)(void)) {
    latchcb = usercb;
}

/*
 * Clear the entire display (make it blank):
 */
void
clear(void) {

    cli();
    for ( byte x = 0; x < 5; ++x )
        data[x] = bitbuf[x] = 0;
    LCD_bits = 40;
    sei();
}

/*
 * Display all minus signs to represent a numeric
 * overflow (value too large to fit in four digit
 * display).
 */
void
show_overflow(void) {
    write("----");
}

/*
 * Write up to four text characters to the display.
 * Note that hexadecimal, '-', '_', and blank are
 * the only characters supported.
 */
void
write(char text[4]) {
    cli();
    for ( byte x = 0; x<4; ++x )
        data[1+3-x] = ascii2seg(text[x]);
    sei();
}

/*
 * Write a four digit unsigned value into the
 * LCD display (with leading zeros). The range
 * of values supported are 0000 to 9999.
 */
void
write_uint(unsigned v) {
    byte r;
    char buf[4];
    
    if ( v > 9999 ) {
        show_overflow();
        return;
    }
    
    for ( byte x=4; x-- > 0; ) {
        r = v % 10;
        v /= 10;
        buf[x] = '0' + r;
    }
    write(buf);
}

/*
 * Write a four digit signed value into the
 * LCD display. Range of values supported are
 * -999 to 9999.
 */
void
write_int(int v) {
    byte r;
    byte c = 0;
    char buf[4];
    
    if ( v < -999 || v > 9999 ) {
        show_overflow();
        return;
    }
    
    if ( v < 0 ) {
        buf[0] = '-';
        c = 1;
        v = -v;
    }
    for ( byte x=4; x-- > c; ) {
        r = v % 10;
        v /= 10;
        buf[x] = '0' + r;
    }
    write(buf);
}

/*
 * Internal routine to toggle bits in data[0];
 */
static void
set_by_mask(boolean b,byte selector) {
    byte mask = segmap[selector];

    cli();
    if ( b != false )
        data[0] |= mask;
    else
        data[0] &= ~mask;
    sei();
}

/*
 * Set or reset the "Secure" word display :
 */
void
set_secure(boolean b) {
    set_by_mask(b,OPTREX_SEG_SECURE);
}

/*
 * Set or reset the "Clear" word display :
 */
void
set_clear(boolean b) {
    set_by_mask(b,OPTREX_SEG_CLEAR);
}

/*
 * Convert one ASCII character into 7-segment display :
 */
byte 
ascii2seg(byte ascii_ch) {
    byte selector = 0;
  
    if ( ascii_ch >= '0' && ascii_ch <= '9' ) 
        return segmap[ascii_ch - '0'];

    if ( ascii_ch >= 'A' && ascii_ch <= 'F' )
        return segmap[ascii_ch - 'A' + 10];

    if ( ascii_ch >= 'a' && ascii_ch <= 'f' )
        return segmap[ascii_ch - 'a' + 10];
        
    switch ( ascii_ch ) {
    case ' ' :
        return 0x00;
    case '-' :
        selector = OPTREX_SEG_MINUS;
        break;
    case '_' :
        selector = OPTREX_SEG_UNDERSCORE;
        break;
    default :
        selector = OPTREX_SEG_UNDERSCORE;
    }

    return segmap[selector];
}

/*
 * LCD Timer service routine :
 *
 * Here we must :
 *      1. Change the state of the data shift clock
 *      2. Change the M clock (at half the rate)
 *      3. Shift out one data bit
 *      4. Activate the latch when the last bit is clocked out
 *      5. Re-start the next refresh, after the latch has been clocked
 */
void
timer_routine() {
        byte x;                                                 // General index
        byte b;                                                 // Current data bit to LCD display
        byte t;                                                 // Carry bit in shift operation
        boolean cb_flag = false;                                // Callback flag

        Shift_State ^= 1;                                       // Toggle shift pin status
        digitalWrite(LCD_Shift,Shift_State);                    // Send to shift pin

        if ( Shift_State != 0 ) {                               // On shift rise, toggle M state
                M_State ^= 1;                                   
                digitalWrite(LCD_M,M_State);                    // Send new M state to M pin
        } 

        if ( Latch_State != 0 ) {                               // Is latch pin active?
                Latch_State = 0;                                // Yes, reset it low
                digitalWrite(LCD_Latch,Latch_State);            // Send new state to Latch pin
                cb_flag = true;                                 // Signal for possible user callback
        } else if ( LCD_bits > 0 ) {                            // else any Data bits to send?
                if ( Shift_State != 0 ) {                               // Yes, but is shift state rising?
                        b = bitbuf[0] & 0x80;                           // Yes, extract next data bit to send
                        digitalWrite(LCD_Din,b != 0 ? HIGH : LOW);      // Send it to the Din pin

                        for ( x=0; x<5; ++x ) {                         // Shift the 40 bit buffer left
                                if ( x+1 < 5 )                          // Extract carry bit from next byte
                                        t = ( bitbuf[x+1] & 0x80 ) ? 1 : 0;
                                else    t = 0;                          // else just use zero
                                bitbuf[x] = (bitbuf[x] << 1) | t;       // Shift left 1 bit, + or in carry bit
                        }
                } else  {                                               // Shift pin has gone LOW
                        --LCD_bits;                                     // Decrement # of data bits remaining
                        if ( LCD_bits == 0 ) {                          // Was this the last bit?
                                Latch_State = 1;                        // Yes, then set latch high to clock in data
                                digitalWrite(LCD_Latch,Latch_State);    // Latch the shifted data into HD44100
                        }
                }
        } else if ( Latch_State == 0 ) {                                // If no data, and latch is LOW...
                for ( x = 0; x < 5; ++x )                               // Refresh bitbuf[] with current data
                    bitbuf[x] = data[x];                                // All 5 bytes
                LCD_bits = 40;                                          // Start first bit on next timer interrupt
        }
        
        if ( cb_flag == true && latchcb != 0 )                          // If we have a user callback, invoke it..
            latchcb();                                                  // User routine: do not tarry too long
}

/*
 * Return control, only after a latch event occurs :
 */
void
wait_latched(void) {

    while ( Latch_State == 0 )
        ;
    while ( Latch_State != 0 )
        ;
}

}

/* End OptrexLCD.cpp */
