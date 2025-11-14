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

#define CONFIG_BLINK_PERIOD 100

/** @def timerPeriod_us
* @brief Periodo del timer en [us]
*/
#define timerPeriod_us 100000 

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
    gpioConf_t gpio5v = {GPIO_19, 1};

/** @def TaskHandle_t controlDeDerrames_task_handle
 *  @brief Handle de la tarea que controla la apertura/cerrado de la válvula
 */
TaskHandle_t controlDeDerrames_task_handle = NULL; 

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



/*==================[internal functions declaration]=========================*/

/** @fn static void controlDeDerrames_task (void)
* @brief Tarea que previene derrames por retirada abrupta del recipiente o rebalse
* @return void
*/

/*==================[external functions definition]==========================*/

static void controlDeDerrames_task (void *pvParameter) {

    while (1) {
        
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		if (control_OnOff) {
			UartSendString(UART_PC,"ctrl");

			distancia=HcSr04ReadDistanceInCentimeters(); 
			if (distancia<10){
                LedsOffAll();
				GPIOOff(gpio5v.pin);

            } else if (distancia<20){
                LedOn(LED_1);
                LedOff(LED_2);
                LedOff(LED_3);
				GPIOOn(gpio5v.pin);


            }else if(distancia<30){
                LedOn(LED_1);
                LedOn(LED_2);
                LedOff(LED_3);
				GPIOOn(gpio5v.pin);


            }else{
                LedOn(LED_1);
                LedOn(LED_2);
                LedOn(LED_3);
				GPIOOff(gpio5v.pin);	//GPIO19 en cero
                
            }		
	

	}

    }
}


void leer_tecla1(){
	control_OnOff=!control_OnOff;
}

void notifyControl(void* param){ 
    vTaskNotifyGiveFromISR(controlDeDerrames_task_handle, pdFALSE);  
}


void app_main(void){

    SwitchesInit();
	SwitchActivInt(SWITCH_1, leer_tecla1, NULL); /* para el encendido/apagado */

    HcSr04Init(GPIO_3, GPIO_2);
	LedsInit();
    GPIOInit(gpio5v.pin, gpio5v.dir);

	xTaskCreate(&controlDeDerrames_task, "", 4096, NULL, 5, &controlDeDerrames_task_handle);


	serial_config_t my_uart={
		.port=UART_PC,
		.baud_rate = 9600,
		.func_p= NULL, /* cambiar si decidimos que el encendido se haga por uart */
		.param_p=NULL	
	};
	UartInit(&my_uart);

    timer_config_t timer_controlDeDerrames = { 

		.timer = TIMER_A,
		.period = timerPeriod_us,
		.func_p = notifyControl,
		.param_p = NULL,

	};
	TimerInit(&timer_controlDeDerrames);

	TimerStart(timer_controlDeDerrames.timer);


}
