#include <stdint.h>
#include <string.h>
#include "mcc_generated_files/mcc.h"

#include "canlib/can_common.h"
#include "canlib/pic18f26k83/pic18f26k83_can.h"
#include "canlib/util/timing_util.h"
#include "canlib/util/can_tx_buffer.h"
#include "canlib/pic18f26k83/pic18f26k83_timer.h"

#include "setup.h"
#include "spi.h"
#include "temperature.h"

#define _XTAL_FREQ 12000000 //12MHz
#define MAX_LOOP_TIME_DIFF_ms 500

// Temp sensor channel mapping arrays
// 21 bytes for worst case 20 channels + null terminator
// use type char so I can cheat and use strlen()
char board_1_map[21] = {7};
char board_2_map[21] = {7};
char board_3_map[21] = {7};

// Memory pool for CAN transmit buffer
uint8_t tx_pool[100];
uint32_t result = 0;

static void can_msg_handler(const can_msg_t *msg);
static void send_status_ok(void);

int main(int argc, char** argv) {
    // initialize pins, interrupts
    pin_init();
    osc_init();
    spi_init();

    // now that the clock is setup
    // give the temp chip time to boot
    __delay_ms(500);

    // enable global interrupts
    INTCON0bits.GIE = 1;

    // initialize timer
    timer0_init();

    /***********set up CAN**********/
    // Set up CAN TX
    TRISC0 = 0;
    RC0PPS = 0x33;
    // Set up CAN RX
    TRISC1 = 1;
    ANSELC1 = 0;
    CANRXPPS = 0x11;
    // set up CAN module
    can_timing_t can_setup;
    can_generate_timing_params(_XTAL_FREQ, &can_setup);

    can_init(&can_setup, can_msg_handler);

    // set up CAN tx buffer
    txb_init(tx_pool, sizeof(tx_pool), can_send, can_send_rdy);

    /***********Setup Temp Sensor Channels***********/
    config_temp_ref_diodes(cs_write_1); // for board 1
    config_temp_ref_diodes(cs_write_2); // for board 2
    config_temp_ref_diodes(cs_write_3); // for board 3
    for(uint8_t idx = 0; idx < strlen(board_1_map); idx++){
        config_tc(board_1_map[idx], cs_write_1);
    }
    for(uint8_t idx = 0; idx < strlen(board_2_map); idx++){
        config_tc(board_2_map[idx], cs_write_2);
    }
    for(uint8_t idx = 0; idx < strlen(board_3_map); idx++){
        config_tc(board_3_map[idx], cs_write_3);
    }

    // indices for tracking which channel to measure
    uint8_t idx_1 = 0, idx_2 = 0, idx_3 = 0;
    // start the first conversion on each board
    start_conversion(board_1_map[idx_1], cs_write_1);
    start_conversion(board_2_map[idx_2], cs_write_2);
    start_conversion(board_3_map[idx_3], cs_write_3);

    uint32_t last_millis = millis();
    uint32_t last_measure_millis = millis();

    // main loop
    while (true) {

        // handle temp measurements
        // should add support for error bits
        if(CONVERSION_COMPLETE_1){
            uint32_t result_1 = get_conversion(board_1_map[idx_1], cs_write_1);
            can_msg_t temp_1_msg;
            // canlib will throw out the MSB and only send the result (the 3 LSB)
            build_temp_data_msg(millis(), 10 + idx_1, result_1, &temp_1_msg);
            txb_enqueue(&temp_1_msg);
            idx_1++;
            if(idx_1 >= strlen(board_1_map)) idx_1 = 0;
            start_conversion(board_1_map[idx_1], cs_write_1);
        }
        if(CONVERSION_COMPLETE_2){
            uint32_t result_2 = get_conversion(board_2_map[idx_2], cs_write_2);
            can_msg_t temp_2_msg;
            // canlib will throw out the MSB and only send the result (the 3 LSB)
            build_temp_data_msg(millis(), 20 + idx_2, result_2, &temp_2_msg);
            txb_enqueue(&temp_2_msg);
            idx_2++;
            if(idx_2 >= strlen(board_2_map)) idx_2 = 0;
            start_conversion(board_2_map[idx_2], cs_write_2);
        }
        if(CONVERSION_COMPLETE_3){
            uint32_t result_3 = get_conversion(board_3_map[idx_3], cs_write_3);
            can_msg_t temp_3_msg;
            // canlib will throw out the MSB and only send the result (the 3 LSB)
            build_temp_data_msg(millis(), 30 + idx_3, result_3, &temp_3_msg);
            txb_enqueue(&temp_3_msg);
            idx_3++;
            if(idx_3 >= strlen(board_3_map)) idx_3 = 0;
            start_conversion(board_3_map[idx_3], cs_write_3);
        }

        // general stuff
        if(millis() > last_millis + MAX_LOOP_TIME_DIFF_ms) {
            last_millis = millis();

            // heartbeat LED
            BLUE_LED = !BLUE_LED;

            send_status_ok();
        }

        // send queued messages
        txb_heartbeat();

    }

    return (EXIT_SUCCESS);
}

static void can_msg_handler(const can_msg_t *msg) {
    uint16_t msg_type = get_message_type(msg);

    switch (msg_type) {

        case MSG_RESET_CMD:
            RESET();
        case MSG_LEDS_ON:
            RED_LED = 1;
            BLUE_LED = 1;
            WHITE_LED = 1;
            break;

        case MSG_LEDS_OFF:
            RED_LED = 0;
            BLUE_LED = 0;
            WHITE_LED = 0;
            break;

        default:
            // this is where we go for all the messages we don't care about
            break;
    }
}

// top level ISR
static void __interrupt() interrupt_handler() {

    // Timer0 has overflowed - update millis() function
    // This happens approximately every 500us
    if (PIE3bits.TMR0IE == 1 && PIR3bits.TMR0IF == 1) {
        timer0_handle_interrupt();
        PIR3bits.TMR0IF = 0;
    }

    // handle CAN interrupts
    if (PIR5) {
        can_handle_interrupt();
    }
}

// Send a CAN message with nominal status
static void send_status_ok(void) {
    can_msg_t board_stat_msg;
    build_board_stat_msg(millis(), E_NOMINAL, NULL, 0, &board_stat_msg);

    // send it off at low priority
    txb_enqueue(&board_stat_msg);
}
