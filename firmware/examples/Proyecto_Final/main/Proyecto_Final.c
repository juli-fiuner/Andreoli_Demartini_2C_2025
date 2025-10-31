/*! @mainpage Ejemplo Bluetooth - FFT
 *
 * @section genDesc General Description
 *

 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 02/04/2024 | Document creation		                         |
 *
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "gpio_mcu.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "switch.h"

#include "led.h"

/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD 1000
/*==================[internal data definition]===============================*/

typedef struct{
		gpio_t pin; /// Nro de pin
		io_t dir; /// lo establece como entrada/input (0) o salida/output (1)
	}gpioConf_t;

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/


void app_main(void){
    gpioConf_t gpio5v = {GPIO_19, 1};
    GPIOInit(gpio5v.pin, gpio5v.dir);
    while(1) {
    	GPIOToggle(gpio5v.pin);
		vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
    }
}

/*==================[end of file]============================================*/
