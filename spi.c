#include "spi.h"


void spi_init(void){
    
    //Configure Periferal Pin Select registers
     
    // SCK is B1
    RB1PPS = 0b011110;
    // set as output
    TRISB1 = 0;
    
    // SDO is C5
    RC5PPS = 0b011111;
    
    // SDI is C7
    SPI1SDIPPS = 0b010111;
    
    
    
    // SPI Clock Select 
    //Fbaud = Fclock / (2*(BAUD+1)
    SPI1CLK = 0; //Fclock = FOSC = 12MHz
    
    //BAUD = Fclock/(2*Fbaud) - 1
    //want Fbaud = 1MHz, 12/(2*1) - 1 = 5
    SPI1BAUD = 0x05;
            
    SPI1CON0bits.MST = 1; // set mode to master
    SPI1CON0bits.BMODE = 1; // sets bit mode to constant width

    //Default settings in SPI1CON1 are good, no changes needed
    
    SPI1CON2bits.TXR = 1; //require transmit for transfer
    SPI1CON2bits.RXR = 1; //require receive for transfer
    
    //Enable SPI 
    SPI1CON0bits.EN = 1; // enable SPI
    
}

bool spi_transfer(uint8_t* send_data, uint8_t* receive_data, 
                  uint8_t datalen, void *cs_write(uint8_t)){
    
    if(!send_data || !receive_data){
        return false; // passed a null pointer
    }
    if(SPI1CON2bits.BUSY){
        return false; // transfer already in progress
    }
    
    SPI1STATUSbits.CLRBF = 1; //ensure buffers are clear
    SPI1TCNTH = datalen; // set the number of bytes to transfer
    
    cs_write(0); //drive chip select line low
    
    //while there are more bytes to transfer or we are busy
    while(SPI1TCNTH > 0 || SPI1CON2bits.BUSY){
        if(SPI1RXB != 0){ // if there is something in the receive buffer
            *receive_data = SPI1RXB; //read a byte if available
            receive_data++; // increment to the next byte
        }
        if(SPI1STATUSbits.TXBE){ // if the TX buffer is empty
            SPI1TXB = *send_data; // add a byte
            send_data++; // increment to the next byte
        }
    }
    
    cs_write(1); //drive chip select line high
    return true;
    
}