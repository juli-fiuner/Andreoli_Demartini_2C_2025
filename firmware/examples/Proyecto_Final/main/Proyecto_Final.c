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
 * @author Demartini Paula (paula.demartini@ingenieria.uner.edu.ar)
 * @author Andreoli Aguilar Julieta (julieta.andreoli@ingenieria.uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "switch.h"
#include "led.h"
#include "gpio_mcu.h"
#include "hc_sr04.h"

/*==================[macros and definitions]=================================*/

#define CONFIG_BLINK_PERIOD 1000

/*==================[internal data definition]===============================*/

typedef struct{
		gpio_t pin; /// Nro de pin
		io_t dir; /// lo establece como entrada/input (0) o salida/output (1)
	}gpioConf_t;

/** @def TaskHandle_t controlDeDerrames_task_handle
 *  @brief Handle de la tarea que controla la apertura/cerrado de la válvula
 */
TaskHandle_t controlDeDerrames_task_handle = NULL; 

/** @def TaskHandle_t mideDistancia_task_handle
 *  @brief Handle de la tarea que mide la distancia a través del sensor
 */
TaskHandle_t mideDistancia_task_handle = NULL; 

/** @def TaskHandle_t UART_task_handle
 *  @brief Handle de la tarea que envía mensajes del estado por UART
 */
TaskHandle_t UART_task_handle = NULL; 

/*==================[internal functions declaration]=========================*/

/** @fn static void controlDeDerrames_task (void)
* @brief Tarea que previene derrames por retirada abrupta del recipiente o rebalse
* @return void
*/
static void controlDeDerrames_task (void);

/** @fn static void mideDistancia_task(void)
* @brief Tarea que enciende los leds según la distancia medida 
* @return void
*/
static void mideDistancia_task(void);


/** @fn static void mideDistancia_Task(void)
* @brief Tarea que enciende los leds según la distancia medida 
* @return void
*/
static void msjUART_task(void);

/*==================[external functions definition]==========================*/




void app_main(void){


	
    gpioConf_t gpio5v = {GPIO_19, 1};
    GPIOInit(gpio5v.pin, gpio5v.dir);

	
    while(1) {
/* Prueba en clase 31/10
    	GPIOToggle(gpio5v.pin);
		vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
*/
    }
}

/*==================[end of file]============================================*/
