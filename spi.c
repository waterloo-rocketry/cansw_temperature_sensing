#include "spi.h"

int next = 0;

void spi_init(void){

    // Configure Peripheral Pin Select registers

    // SCK is B1
    RB1PPS = 0b011110;
    // set as output
    TRISB1 = 0;

    // SDO is C5
    RC5PPS = 0b011111;
    ANSELC5 = 0; // Enable digital input buffer

    // SDI is C7
    SPI1SDIPPS = 0b010111;
    ANSELC7 = 0; // Enable digital input buffer



    // SPI Clock Select
    // Fbaud = Fclock / (2*(BAUD+1)
    SPI1CLK = 0; // Fclock = FOSC = 12MHz

    // BAUD = Fclock/(2*Fbaud) - 1
    // want Fbaud = 1MHz, 12/(2*1) - 1 = 5
    SPI1BAUD = 0x05;

    SPI1CON0bits.MST = 1; // set mode to master
    SPI1CON0bits.BMODE = 1; // sets bit mode to constant width

    SPI1CON1bits.CKE = 1; // change data on falling edge

    SPI1CON2bits.TXR = 1; // require transmit for transfer
    SPI1CON2bits.RXR = 1; // require receive for transfer

    // Enable SPI
    SPI1CON0bits.EN = 1; // enable SPI

}

bool spi_transfer(uint8_t* send_data, uint8_t* receive_data,
                  uint8_t datalen, void (*cs_write)(uint8_t)){

    if(!send_data || !receive_data){
        return false; // passed a null pointer
    }
    if(SPI1CON2bits.BUSY){
        return false; // transfer already in progress
    }

    SPI1STATUSbits.CLRBF = 1; // ensure buffers are clear
    SPI1TCNTL = datalen; // set the number of bytes to transfer

    cs_write(0); // drive chip select line low

    for (uint8_t idx = 0; idx < datalen; idx++){
        while (!SPI1STATUSbits.TXBE); // wait until transmit buffer empty
        SPI1TXB = send_data[idx]; // add byte to transmit buffer
        while (!PIR2bits.SPI1RXIF); // wait to recive a byte
        PIR2bits.SPI1RXIF = 0; // clear interupt flag
        receive_data[idx] = SPI1RXB; // shift data into recive array
    }

    cs_write(1); // drive chip select line high
    return true;

}