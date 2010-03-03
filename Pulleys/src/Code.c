#include <pic16f628a.h>

/* ----------------------------------------------------------------------- */
/* Configuration bits                                                      */
/* ----------------------------------------------------------------------- */
typedef unsigned int word;
word at 0x2007 CONFIG = _WDT_OFF & _PWRTE_OFF & _HS_OSC & _MCLRE_OFF & _BOREN_OFF & _LVP_OFF & _DATA_CP_OFF & _CP_OFF;

void isr() interrupt 0 {                                                                                                  
    /* interrupt service routine */
    /* << insert interrupt code >> */
}

/**************************************************************
 **** MISC ROUTINES
 **************************************************************/

#define LED_BIT 0x10

#define KHZ 10000
#define TX_PORT 2
#define RX_PORT 1
#define TX_BIT  (1<<TX_PORT)
#define RX_BIT  (1<<RX_PORT)

#define BAUD    9600
#define BAUD_FACTOR (16L*BAUD)
#define SPBRG_VALUE (unsigned char)(((KHZ*1000L)-BAUD_FACTOR)/BAUD_FACTOR)

void serial_init () {
// --------------------------------------------------
// The serial code setup and base calculations were stolen from
// http://burningsmell.org/pic16f628/src/0004-uart_tx.c
// Thanks! :)
//
    TRISB|=TX_BIT|RX_BIT;   // These need to be 1 for USART to work
    SPBRG=SPBRG_VALUE;      // Baud Rate register, calculated by macro
    BRGH=1;

    SYNC=0;                 // Disable Synchronous/Enable Asynchronous
    SPEN=1;                 // Enable serial port
    TXEN=1;                 // Enable transmission mode
}

void putchar (char c) {
// --------------------------------------------------
    TXREG=c;
    while(!TXIF);   // Wait while the output buffer is full
}

void printf ( const char *string ) {
// --------------------------------------------------
    unsigned char i;
    for (i=0;i<255;i++) {
        if ( string[i] == '\0' ) return;
        putchar(string[i]);
    }
}

void printint ( unsigned int x ) {
// --------------------------------------------------
    char string[6] = {0,0,0,0,0,0};
    char i = 4;
    while ( x > 0 ) {
        string[i--] = '0' + ( x % 10 );
        x /= 10;
    }
    for (;i<5;i++) {
        putchar(string[i]);
    }
}


void printbytehex ( unsigned char x ) {
// --------------------------------------------------
    unsigned char y;
    y = x;
    y >>= 4;
    putchar('0');
    putchar('x');
    putchar(y<10?('0'+y):('A'+y-10));
    x &= 0xf;
    putchar(x<10?('0'+x):('A'+x-10));
}

void printbytebits ( unsigned char x ) {
// --------------------------------------------------
    unsigned char i;
    putchar('0');
    putchar('b');
    for (i=0;i<8;i++) {
        putchar(x & 0x80 ? '1' : '0');
        x <<= 1;
    }
}

void led_init () {
// --------------------------------------------------
// Setup the output pin, etc
//
    TRISB &= ~LED_BIT;
}

void led_on () {
// --------------------------------------------------
// Turn the durn led on!
//
    PORTB |= LED_BIT;
}

void led_off () {
// --------------------------------------------------
// Turn the durn led on!
//
    PORTB &= ~LED_BIT;
}

/**************************************************************
 **** /MISC ROUTINES
 **************************************************************/


/**************************************************************
 **** SPI MASTER BIT BANGING ROUTINES
 **** Clock is idle when low
 **** Sample on clock edge rising
 **** This code will be used to bit bang to a nRF24L01+ based
 **** 2.4GHz wireless tranceiver
 **************************************************************/

#define SPI_MOSI   0x01
#define SPI_MISO   0x02
#define SPI_CLK    0x04
#define SPI_SPEED  2

void spi_init () {
// -----------------------------------------------------
// Setup the PORTA locations for SPI bitbanging
//
    TRISA &= ~( SPI_MOSI | SPI_CLK );
    TRISA |= SPI_MISO;
    PORTA &= ~( SPI_MOSI | SPI_MISO | SPI_CLK );
}

unsigned char spi_txrx ( unsigned char tx ) {
// -----------------------------------------------------
// Send and receive a single byte of data
//
   unsigned char rx = 0;
   unsigned char i  = 0;
   unsigned int  j  = 0;

   for ( i = 0; i < 8; i++ ) {

    /* Write the bit we want to send out */
      PORTA = ( tx & 0x80 ) ? ( PORTA |  SPI_MOSI )
                            : ( PORTA & ~SPI_MOSI );
      tx <<= 1;

    /* Wait for half the delay then set the clock high */
      for ( j = 0; j < SPI_SPEED/2; j++ );
      PORTA |= SPI_CLK;

    /* Wait for the other half of the delay */
      for ( j = 0; j < SPI_SPEED/2; j++ );

    /* Read the MISO */
      rx <<= 1;
      rx |= ( PORTA & SPI_MISO ) ? 1 : 0;

    /* Clear the clock bit and we're done! */
      PORTA &= ~SPI_CLK;

   }

   return rx;
}

/**************************************************************
 **** /SPI ROUTINES
 **************************************************************/


/**************************************************************
 **** nRF24L01 ROUTINES
 **************************************************************/

/* SPI COMMAND SET FOR nRF2401+ */

#define NRF_R_REGISTER          0x00
#define NRF_W_REGISTER          0x20 
#define NRF_R_RX_PAYLOAD        0x61 
#define NRF_W_TX_PAYLOAD        0xa0
#define NRF_FLUSH_TX            0xe1
#define NRF_FLUSH_RX            0xe2
#define NRF_REUSE_TX_PL         0xe3 
#define NRF_R_RX_PL_WID         0x30
#define NRF_W_ACK_PAYLOAD       0xa8
#define NRF_W_TX_PAYLOAD_NO_ACK 0xb0
#define NRF_NOP                 0xFF

/* SPI COMMAND SET OPTIONS */
#define NRF_CONFIG              0x00
#define NRF_EN_AA               0x01
#define NRF_EN_RXADDR           0x02
#define NRF_SETUP_AW            0x03
#define NRF_SETUP_RETR          0x04
#define NRF_RF_CH               0x05
#define NRF_RF_SETUP            0x06
#define NRF_STATUS              0x07
#define NRF_OBSERVE_TX          0x08
#define NRF_CD                  0x09
#define NRF_RX_ADDR_P0          0x0A
#define NRF_RX_ADDR_P1          0x0B
#define NRF_RX_ADDR_P2          0x0C
#define NRF_RX_ADDR_P3          0x0D
#define NRF_RX_ADDR_P4          0x0E
#define NRF_RX_ADDR_P5          0x0F
#define NRF_TX_ADDR             0x10
#define NRF_RX_PW_P0            0x11
#define NRF_RX_PW_P1            0x12
#define NRF_RX_PW_P2            0x13
#define NRF_RX_PW_P3            0x14
#define NRF_RX_PW_P4            0x15
#define NRF_RX_PW_P5            0x16
#define NRF_FIFO_STATUS         0x17

/* Bit Mnemonics */
#define NRF_MASK_RX_DR  0b01000000
#define NRF_MASK_TX_DS  0b00100000
#define NRF_MASK_MAX_RT 0b00010000
#define NRF_EN_CRC      0b00001000
#define NRF_CRCO        0b00000100
#define NRF_PWR_UP      0b00000010
#define NRF_PRIM_RX     0b00000001

#define NRF_ENAA_P5     5
#define NRF_ENAA_P4     4
#define NRF_ENAA_P3     3
#define NRF_ENAA_P2     2
#define NRF_ENAA_P1     1
#define NRF_ENAA_P0     0

#define NRF_ERX_P5      5
#define NRF_ERX_P4      4
#define NRF_ERX_P3      3
#define NRF_ERX_P2      2
#define NRF_ERX_P1      1
#define NRF_ERX_P0      0

#define NRF_AW          0

#define NRF_ARD         4
#define NRF_ARC         0

#define NRF_PLL_LOCK    4
#define NRF_RF_DR       3
#define NRF_RF_PWR      1
#define NRF_LNA_HCURR   0        

#define NRF_RX_DR       1<<6
#define NRF_TX_DS       1<<5
#define NRF_MAX_RT      1<<4
#define NRF_RX_P_NO     1<<1
#define NRF_TX_FULL     0

#define NRF_PLOS_CNT    4
#define NRF_ARC_CNT     0

#define NRF_TX_REUSE    6
#define NRF_FIFO_FULL   5
#define NRF_TX_EMPTY    4
#define NRF_RX_FULL     1
#define NRF_RX_EMPTY    0

#define NRF_DPL_P5      5
#define NRF_DPL_P4      4
#define NRF_DPL_P3      3

#define NRF_CS                  0x08
#define NRF_CE                  0x80
#define NRF_ADDR                0xe7, 0xe7, 0xe7, 0xe7

void nrf_chip_enable () {
// --------------------------------------------------
// Enable the nordic radio
//
    PORTB |= NRF_CE;
}

void nrf_chip_disable () {
// --------------------------------------------------
// Disable the nordic radio
//
    PORTB &= ~NRF_CE;
}

void nrf_chip_select () {
// --------------------------------------------------
// Notify that there'll be an SPI command coming in
// (Not exactly but this is how it is effectively 
// used in our case ;)
//
    PORTA |= NRF_CS;
}

void nrf_chip_deselect () {
// --------------------------------------------------
// Done sending the command
// (Not exactly but this is how it is effectively 
// used in our case ;)
//
    PORTA &= ~NRF_CS;
}


void nrf_config ( unsigned char config_key, unsigned char config_setting) {
// --------------------------------------------------
// Set the configuration of a single item in the nordic tranciever
//
    nrf_chip_deselect();
    spi_txrx( NRF_W_REGISTER | config_key );
    spi_txrx( config_setting );
    nrf_chip_select();
}

void nrf_init () {
// --------------------------------------------------
// Setup the tranceiver to communicate with the nordic
// fob (at the moment)
//
// Init the spi engine first. W00t!
    spi_init();

// Setup the tranceiver pins
    TRISA &= ~NRF_CS;
    PORTA &= ~NRF_CS;
    TRISB &= ~NRF_CE;
    PORTB &= ~NRF_CE;

// Let's start configuring the tranciever!
    nrf_chip_deselect();
    nrf_chip_disable();

    nrf_config( NRF_CONFIG,   0x39 );
    nrf_config( NRF_EN_AA,    0x00 );
    nrf_config( NRF_SETUP_AW, 0x03 );
    nrf_config( NRF_RF_SETUP, 0x07 );
    nrf_config( 0x31, 0x04 );
    nrf_config( NRF_RF_CH,    0x02 );
    nrf_chip_deselect();
    spi_txrx( NRF_W_REGISTER | NRF_RX_ADDR_P0 );
    spi_txrx( 0xe7 );
    spi_txrx( 0xe7 );
    spi_txrx( 0xe7 );
    spi_txrx( 0xe7 );
    nrf_chip_select();

    nrf_config( NRF_CONFIG,   0x3b );

    nrf_chip_enable();
    nrf_chip_select();
}

unsigned char nrf_status () {
// --------------------------------------------------
// Get an update from the transceiver to see if we're
// awaiting any data
//
    unsigned char rx;
    nrf_chip_deselect();
    rx = spi_txrx( NRF_NOP );
    nrf_chip_select();
    return rx;
}

/**************************************************************
 **** /nRF24L01 ROUTINES
 **************************************************************/


void main() {
    unsigned char status;
    unsigned int i;
    unsigned int iteration = 0;
    CMCON = 0x07; 

// Let us know we're alive
    serial_init();
    led_init();
    led_on();
    for ( i = 0; i < 60000; i++ ) {
        i += 10;
        i -= 10;
    };
    nrf_init();
    led_off();

// Now sit and wait for some response to come in.
    while (1) {

        iteration++;

// If the radio status is true. We need to pull the data out and
// clear the radio for the next chunk of data
        status = nrf_status();
        if ( status & 0x40 ) {
            printf( " IN LOOP!" );
            led_on();

            nrf_chip_disable();

// Get the data from the chip...
            nrf_chip_deselect();
            spi_txrx( NRF_R_RX_PAYLOAD );
            status = spi_txrx( NRF_NOP );
            printbytehex(status);putchar(' ');
            status = spi_txrx( NRF_NOP );
            printbytehex(status);putchar(' ');
            status = spi_txrx( NRF_NOP );
            printbytehex(status);putchar(' ');
            status = spi_txrx( NRF_NOP );
            printbytehex(status);putchar(' ');
            nrf_chip_select();

// Enable comm with SPI device
            nrf_chip_deselect();
            spi_txrx( NRF_FLUSH_RX );
            nrf_chip_select();

// Clear the RF FIFO interrupt
            nrf_config( NRF_STATUS, 0x40 );
            nrf_chip_enable();
            printf("\r\n");

        }
    }
}
