#include "temperature.h"



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

bool config_temp_ref_diodes(void (*cs_write)(uint8_t)){
    bool success = true;
    success &= config_channel(6, DIODE_CONFIG_WORD, cs_write);
    success &= config_channel(10, DIODE_CONFIG_WORD, cs_write);
    success &= config_channel(16, DIODE_CONFIG_WORD, cs_write);
    success &= config_channel(20, DIODE_CONFIG_WORD, cs_write);
    
    return success;
}

bool config_tc(uint8_t channel_num, void (*cs_write)(uint8_t)){
    uint8_t cold_junc_ch = 0;
    if(channel_num > 0 && channel_num < 6) cold_junc_ch = 6;
    else if(channel_num > 6 && channel_num < 10) cold_junc_ch = 10;
    else if(channel_num > 10 && channel_num < 16) cold_junc_ch = 16;
    else if(channel_num > 16 && channel_num < 20) cold_junc_ch = 20;
    else{
        //If it wasn't in one of those ranges, then it isn't valid
        return false;
    }
    // add the cold junction channel to the config word
    uint32_t config_word = TC_CONFIG_WORD | ((cold_junc_ch & 0x1F) << 22);
    return config_channel(channel_num, config_word, cs_write);
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
        result = ((uint32_t)receive_data[3] << 24) & 0xFF000000;
        result |= ((uint32_t)receive_data[4] << 16) & 0xFF0000;
        result |= ((uint32_t)receive_data[5] << 8) & 0xFF00;
        result |= (uint32_t)receive_data[6] & 0xFF;  
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