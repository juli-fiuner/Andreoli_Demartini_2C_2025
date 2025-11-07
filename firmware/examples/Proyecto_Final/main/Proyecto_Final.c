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
#include "uart_mcu.h"


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

/** @def uint8_t control_OnOff
 * @brief si es 0 --> apagado; si es 1 --> encendido. Controlado por SWITCH_1 y la lógica de cotrol de derrames.
 */
uint8_t control_OnOff=0;

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
* @brief Tarea que envía mensajes por UART, los posibles mensajes son: "apagado" 
(al comienzo del programa, cuando se apaga manualmente o cuando detecta que se retira el recipiente), "cargando...", "fin de carga".
* @return void
*/
static void msjUART_task(void);

/*==================[external functions definition]==========================*/

void leer_tecla1(){
	control_OnOff=!control_OnOff;
}



static void msjUART_task(void *pvParameter){
	while(true){
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if(control_OnOff){ /* = 1--> msj de cargando...,*/
				UartSendString(UART_PC,"Cargando...");
			if(1){ /*ver si usar otra variable de control o directamente hacer por comparación de la medida*/
				UartSendString(UART_PC,-"Fin de la carga");
				control_OnOff=!control_OnOff;
				vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS); /** cuanto tiempo? */
			}
		} else if (!control_OnOff){
			UartSendString(UART_PC, "Apagado");
		}




	}
}





void app_main(void){


/*Para los mensajes de estado de la UART */
	SwitchesInit();
	SwitchActivInt(SWITCH_1, leer_tecla1, NULL); /* para el encendido/apagado */
	serial_config_t my_uart={
		.port=UART_PC,
		.baud_rate = 9600,
		.func_p= NULL, /* cambiar si decidimos que el encendido se haga por uart */
		.param_p=NULL	
	};
	UartInit(&my_uart);

	
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
