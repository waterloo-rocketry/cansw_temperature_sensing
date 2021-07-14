#include "temperature.h"


bool init_temp_ref_diodes(void){
    return true;
}

void cs_write_1(uint8_t state){
    if(!state)
        LATC4 = 0;
    else
        LATC4 = 1;
}

void cs_write_2(uint8_t state){
    if(!state)
        LATC6 = 0;
    else
        LATC6 = 1;
}

void cs_write_3(uint8_t state){
    if(!state)
        LATB0 = 0;
    else
        LATB0 = 1;
}


bool config_channel(uint8_t channel_num, uint32_t config_word, 
                    void (*cs_write)(uint8_t)){
    
    if(channel_num > 20 || channel_num == 0) return false; //invalid num
    
    uint8_t send_data[7];
    uint8_t receive_data[7]; //unused
    uint16_t address = 0x200 + (channel_num - 1)*4; //find the memory address
    
    send_data[0] = 0x02; //write instruction
    send_data[1] = address >> 8;
    send_data[2] = address & 0xff;
    send_data[3] = config_word >> 24;
    send_data[4] = (config_word >> 16) & 0xff;
    send_data[5] = (config_word >> 8) & 0xff;
    send_data[6] = config_word & 0xff;
    
    return spi_transfer(send_data, receive_data, 7, cs_write);
    
}

bool start_conversion(uint8_t channel_num, void (*cs_write)(uint8_t)){
    
    if(channel_num > 20 || channel_num == 0) return false; //invalid num
    
    uint8_t send_data[4];
    uint8_t receive_data[4]; //unused
    
    send_data[0] = 0x02; //write instruction
    send_data[1] = 0x00; //command register is address 0
    send_data[2] = 0x00;
    send_data[3] = 0x80 | channel_num; //set start bit and channel num
    
    return spi_transfer(send_data, receive_data, 4, cs_write);
}

uint32_t get_conversion(uint8_t channel_num, void (*cs_write)(uint8_t)){
    
    if(channel_num > 20 || channel_num == 0) return false; //invalid num
    
    uint8_t send_data[7];
    uint8_t receive_data[7];
    uint16_t address = 0x10 + (channel_num - 1)*4; //find the memory address
    
    send_data[0] = 0x03; //read instruction
    send_data[1] = address >> 8;
    send_data[2] = address & 0xff;
    send_data[3] = 0;
    send_data[4] = 0;
    send_data[5] = 0;
    send_data[6] = 0;
    
    uint32_t result = 0;
    if(spi_transfer(send_data, receive_data, 7, cs_write)){
        result = receive_data[3] << 24;
        result |= receive_data[4] << 16;
        result |= receive_data[5] << 8;
        result |= receive_data[6];  
    }
    else{
        result = 0x00100000; // return -8191 and 1023/1024 degrees
    }
    
    return result;
    
}

uint8_t get_status(void (*cs_write)(uint8_t)){
    uint8_t send_data[4];
    uint8_t receive_data[4];
    
    send_data[0] = 0x03; //read instruction
    send_data[1] = 0x00; //command register is address 0
    send_data[2] = 0x00;
    send_data[3] = 0x00; //unused
    
    spi_transfer(send_data, receive_data, 4, cs_write);
            
    return receive_data[3];
}