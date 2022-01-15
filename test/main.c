#include "uart.h"
#include "twi.h"
#include "MPU9250.h"
#include "BMP280.h"

#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>

typedef struct USART_RXC_cb_ctx_t {
    uint8_t rx;
} USART_RXC_cb_ctx;

/**
 * USART RX interrupt callback handle
 * @param[inout] ctx user data for interrupt callback
 * When ISR occurs USART_RXC_cb will be called with ctx as parameter
 * UART RX data (UDR) should be saved in this function
 */
static void USART_RXC_cb_handle(void* ctx) {
    USART_RXC_cb_ctx* t_ctx = (USART_RXC_cb_ctx*)ctx;

    t_ctx->rx = UDR;
}

int main(void) {

    // UART INIT
    //-------------------------------
    const uint16_t baud_rate = 38400;

    uint8_t sts = 0;
    struct USART_RXC_cb_ctx_t USART_RXC_ctx = {};

    regiter_USART_RXC_cb(USART_RXC_cb_handle, &USART_RXC_ctx);

    USART_init(baud_rate);

    // TWI init
    //-------------------------------
    TWI_init(TWI_PS_1, 2);

    // MPU9250 init
    //-------------------------------
    register_MPU_cb(TWI_write_reg, TWI_read_reg_burst);

    MPU9250_calib();

    sts = MPU9250_init();
    if (sts) return sts;

    printf("MPU9250 init done\n");

    MPU9250_data mpu_data = {};

    // BMP280 init
    //-------------------------------
    register_BMP_cb(TWI_write_reg, TWI_read_reg_burst);

    sts = BMP280_init();
    if (sts) return sts;

    printf("BMP280 init done\n");

    BMP280_final bmp_data = {};

    while(1) {
        
        MPU9250_get_data(&mpu_data);
        BMP280_get_data(&bmp_data);

        // printf("%05d\n", mpu_data.tmp);
        printf("%05d, %05d, %05d ", mpu_data.acc[0], mpu_data.acc[1], mpu_data.acc[2]);
        printf("%d, %d\n", (int16_t)(bmp_data.temp/256/100), (int16_t)(bmp_data.baro/256/100));
        // printf("%05d, %05d, %05d\n", mpu_data.gyro[0], mpu_data.gyro[1], mpu_data.gyro[2]);
        // printf("%05d, %05d, %05d\n", mpu_data.mag[0] - (min_mag[0] + max_mag[0]) / 2, mpu_data.mag[1] -  (min_mag[1] + max_mag[1]) / 2, mpu_data.mag[2] - (min_mag[2] + max_mag[2]) / 2);

        _delay_ms(100);
    }

    return sts;
}
