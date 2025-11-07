/*! @mainpage Ejemplo Bluetooth - FFT
 *
 * @section genDesc General Description
 *

 *
 * @section changelog Changelog
 *
 * |    HC-SR04     |   EDU-ESP   	|
 * |:--------------:|:--------------|
 * | 	ECHO 	 	| 	GPIO_3		|
 * | 	TRIGGER	 	| 	GPIO_2		|
 * | 	+5V 	 	| 	+5V 		|
 * | 	GND 	 	| 	GND     	|
 * 
 * |     Relé       |   EDU-ESP   	|
 * |:--------------:|:--------------|
 * | 	TRIGGER (S) | 	GPIO_19		|
 * |    	+ 	 	| 	+5V 		|
 * | 		-   	| 	GND     	|
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
#include "timer_mcu.h"


/*==================[macros and definitions]=================================*/

#define CONFIG_BLINK_PERIOD 1000

/** @def timerPeriod_us
* @brief Periodo del timer en [us]
*/
#define timerPeriod_us 1000000 

/** @def distanciaGround_cm
* @brief Distancia a la mesa desde la electroválvula en [cm]
*/
#define distanciaGround_cm 30 

/** @def distanciaLimite_cm
* @brief Distancia al nivel de agua máximo en [cm]
*/
#define distanciaLimite_cm 6 

/*==================[internal data definition]===============================*/

typedef struct{
		gpio_t pin; /// Nro de pin
		io_t dir; /// lo establece como entrada/input (0) o salida/output (1)
	} gpioConf_t;


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

/** @def uint8_t control_estado
 * @brief Variable que informa a la UART el estado de carga una vez encendido el sistema: 
 * si es 0 --> llegó al nivel deseado; si es 1 --> retirada abrupta. 
 */
uint8_t control_estado=3;

/** @def uint8_t distancia
 * @brief Variable que guarda la distancia medida
 */
uint16_t distancia;

/** @def void notifyDistancia(void* param)
 * @brief Función que notifica a mideDistancia_task
 */
//void notifyDistancia(void* param);

/** @def void notifyControl(void* param)
 * @brief Función que notifica a controlDeDerrames_task
 */
//void notifyControl(void* param);

/** @def vector_LEDS
* @brief Vector de LEDS (estructuras led_t)
*/
led_t vector_LEDS[3]={LED_1, LED_2, LED_3}; //vector de led_t



gpioConf_t gpio5v = {GPIO_19, 1};

/*==================[internal functions declaration]=========================*/

/** @fn static void controlDeDerrames_task (void)
* @brief Tarea que previene derrames por retirada abrupta del recipiente o rebalse
* @return void
*/
static void controlDeDerrames_task (void *pvParameter);

/** @fn static void mideDistancia_task(void)
* @brief Tarea que enciende los leds según la distancia medida 
* @return void
*/
static void mideDistancia_task(void *pvParameter);


/** @fn static void mideDistancia_Task(void)
* @brief Tarea que envía mensajes por UART, los posibles mensajes son: "apagado" 
(al comienzo del programa, cuando se apaga manualmente o cuando detecta que se retira el recipiente), "cargando...", "fin de carga".
* @return void
*/
static void msjUART_task(void *pvParameter);

/*==================[external functions definition]==========================*/

void leer_tecla1(){
	control_OnOff=!control_OnOff;
}


void notifyDistancia(void* param){ 
    vTaskNotifyGiveFromISR(mideDistancia_task_handle, pdFALSE);  
}

void notifyControl(void* param){ 
    vTaskNotifyGiveFromISR(controlDeDerrames_task_handle, pdFALSE);  
}

/*

static void msjUART_task(void *pvParameter){
	while(true){
		if(control_OnOff){ 
				UartSendString(UART_PC,"Cargando...");
			if(!control_estado){ 
				UartSendString(UART_PC,"Fin de la carga");
				control_estado=3;
			}else if(control_estado){
				UartSendString(UART_PC, "Apagado");
				control_estado=3;
			}
		} else if (!control_OnOff){
			UartSendString(UART_PC, "Apagado");
		}

	vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
	}
}

*/

static void mideDistancia_task(void *pvParameter){

	while (1) {


		if (control_OnOff) {

			distancia=HcSr04ReadDistanceInCentimeters(); 
		
			if (distancia<=10) {

				for (int i=0 ; i<3 ; i++) {
					LedOff(vector_LEDS[i]);
				}

			} else if ((10<distancia) && (distancia<=20)) {

				for (int i=1 ; i<3 ; i++) {
					LedOff(vector_LEDS[i]);
				}
				LedOn(vector_LEDS[0]);

			} else if ((20<distancia) && (distancia<=30)) {

				for (int i=0 ; i<2 ; i++) {
					LedOn(vector_LEDS[i]);
				}
				LedOff(vector_LEDS[2]);

			} else { 

				for (int i=0 ; i<3 ; i++) {
					LedOn(vector_LEDS[i]);
				}
			}

		}
	
		vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);

	}

}


/*
static void controlDeDerrames_task (void *pvParameter) {

    while (1) {
        
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if (control_OnOff) {

			if (distancia==distanciaLimite_cm || distancia<distanciaLimite_cm) {

				GPIOOff(gpio5v.pin);
				control_estado=0;

			} else if (distancia>distanciaLimite_cm && distancia<distanciaGround_cm) {

				GPIOOn(gpio5v.pin);
            
			} else if (distancia==distanciaGround_cm) {

				GPIOOff(gpio5v.pin);
				control_estado=1;

			}

		} else if (!control_OnOff) {

				GPIOOff(gpio5v.pin);	//GPIO19 en cero

		}


    }

}
	*/

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


	HcSr04Init(GPIO_3, GPIO_2);
	LedsInit();
    GPIOInit(gpio5v.pin, gpio5v.dir);

/*	xTaskCreate(&msjUART_task, "", 4096, NULL, 5, &UART_task_handle);
*/
	xTaskCreate(&mideDistancia_task, "", 4096, NULL, 5, &mideDistancia_task_handle);
/*	xTaskCreate(&controlDeDerrames_task, "", 4096, NULL, 5, &controlDeDerrames_task_handle);
*/


	timer_config_t timer_controlDeDerrames = { 

		.timer = TIMER_A,
		.period = timerPeriod_us,
		.func_p = notifyControl,
		.param_p = NULL,

	};

	timer_config_t timer_mideDistancia = { 

		.timer = TIMER_B,
		.period = timerPeriod_us,
		.func_p = notifyDistancia,
		.param_p = NULL,

	};

	TimerStart(timer_controlDeDerrames.timer);
	TimerStart(timer_mideDistancia.timer);


	
    while(1) {
/* Prueba en clase 31/10
    	GPIOToggle(gpio5v.pin);
		vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
*/
    }
}

/*==================[end of file]============================================*/
