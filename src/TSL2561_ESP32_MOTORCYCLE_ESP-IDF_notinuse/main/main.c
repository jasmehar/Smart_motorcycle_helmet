#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "sdkconfig.h"

#include "LightSensorInterface.h"
#include "TSL2561_defs.h"
#include "TSL2561.h"    

static const char *TAG_RED = "MOTORCYCLE BRAKE LIGHTS";
static const char *TAG_LEFT = "MOTORCYCLE lEFT TURN LIGHTS";
static const char *TAG_RIGHT = "MOTORCYCLE RIGHT TURN LIGHTS";

#define I2C_PORT            I2C_NUM_0
#define I2C_MASTER_SCL_IO   38               
#define I2C_MASTER_SDA_IO   37
#define I2C_INTERRUPT_IO    7

#define LED_BUILT_IN_IO     48

#define LIGHT_MAX_TH 0xFFFF
#define LIGHT_MIN_TH 0x20

// static void onLightOutOfBounds(uint16_t val) { //if needed for interupts 
//     if(LIGHT_MIN_TH) {
//         gpio_set_level(LED_BUILT_IN_IO, true);
//         printf(TAG, "It's getting too dark!");
//     }
// }

const TSL2561_Config_t tslConfig_red = {
    .address = TSL2561_ADDR_FLOAT,
    .port = I2C_PORT,
    .io = {
        .scl = I2C_MASTER_SCL_IO,
        .sda = I2C_MASTER_SDA_IO,
        // .isr = I2C_INTERRUPT_IO
    }
};

const TSL2561_Config_t tslConfig_left = {
    .address = TSL2561_ADDR_LOW,
    .port = I2C_PORT,
    .io = {
        .scl = I2C_MASTER_SCL_IO,
        .sda = I2C_MASTER_SDA_IO,
        // .isr = I2C_INTERRUPT_IO
    }
};

const TSL2561_Config_t tslConfig_right = {
    .address = TSL2561_ADDR_HIGH,
    .port = I2C_PORT,
    .io = {
        .scl = I2C_MASTER_SCL_IO,
        .sda = I2C_MASTER_SDA_IO,
        // .isr = I2C_INTERRUPT_IO
    }
};


void app_main(void){


static TSL2561 brake_light;
static TSL2561 left_turn;
static TSL2561 right_turn;
static LightSensorInterface* brake_light_sensor;
static LightSensorInterface* left_turn_light_sensor;
static LightSensorInterface* right_turn_light_sensor;


TSL2561_new(&brake_light, tslConfig_red);
TSL2561_new(&left_turn, tslConfig_left);
TSL2561_new(&right_turn, tslConfig_right);

brake_light_sensor = TSL2561_viewAsLightSensorInterface(&brake_light);
left_turn_light_sensor = TSL2561_viewAsLightSensorInterface(&left_turn);
right_turn_light_sensor = TSL2561_viewAsLightSensorInterface(&right_turn);

    LightSensorInterface_init(brake_light_sensor);
    LightSensorInterface_init(left_turn_light_sensor);
    LightSensorInterface_init(right_turn_light_sensor);
    // LightSensorInterface_enableThresholdInterrupts(brake_light_sensor, LIGHT_MIN_TH, LIGHT_MAX_TH, onLightOutOfBounds);

    uint16_t irradiance_red;
    uint16_t irradiance_left;
    uint16_t irradiance_right;        
    while(1) {
        LightSensorInterface_readIrradianceLevel(brake_light_sensor , &irradiance_red);
        LightSensorInterface_readIrradianceLevel(left_turn_light_sensor , &irradiance_left);
        LightSensorInterface_readIrradianceLevel(right_turn_light_sensor , &irradiance_right);       
        // if(irradiance_red > LIGHT_MIN_TH) {
        //     gpio_set_level(LED_BUILT_IN_IO, false);
        // }            
        ESP_LOGI(TAG_RED, "Read: %x", irradiance_red);
        ESP_LOGI(TAG_LEFT, "Read: %x", irradiance_left);
        ESP_LOGI(TAG_RIGHT, "Read: %x", irradiance_right);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
