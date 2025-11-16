/*! @mainpage Proyecto Final de Andreoli Aguilar y Demartini
 *
 * @section genDesc General Description
 * 
 * Proyecto final de electrónica programable que consiste en un sistema automático de control de derrames para dispensadores de agua
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

/** @def CONFIG_BLINK_PERIOD
* @brief Configuración del periodo del vTaskDelay
*/
#define CONFIG_BLINK_PERIOD 1000

/** @def timerPeriod_us
* @brief Periodo del timer en [us]
*/
#define timerPeriod_us 50000 

/** @def distanciaGround_cm
* @brief Distancia a la mesa desde la electroválvula en [cm]
*/
#define distanciaGround_cm 30

/** @def distanciaLimite_cm
* @brief Distancia al nivel de agua máximo en [cm]
*/
#define distanciaLimite_cm 10

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
 * si es 0 --> llegó al nivel deseado; si es 1 --> retirada abrupta; si es 3 --> sistema habilitado a cargar. 
 */
uint8_t control_estado=3;

/** @def uint8_t distancia
 * @brief Variable que guarda la distancia medida
 */
uint16_t distancia;

/*==================[internal functions declaration]=========================*/

/** @fn static void controlDeDerrames_task (void)
* @brief Tarea que previene derrames por retirada abrupta del recipiente o rebalse. También indica cualitativamente con los LEDs el progreso del llenado.
* @return void
*/
static void controlDeDerrames_task (void *pvParameter);


/** @fn static void mideDistancia_Task(void)
* @brief Tarea que envía mensajes por UART, los posibles mensajes son: "apagado" 
(idle o cuando se apaga manualmente), "fin de carga" y "has retirado el recipiente"

control_OnOff = 1, variable control 3 --> cargando, no se envían mensajes

control_OnOff = 0, variable control 0 --> fin de carga

control_OnOff = 0, variable control 1 --> has retirado el recipiente

control_OnOff = 0, apagado

* @return void
*/
static void msjUART_task(void *pvParameter);

/*==================[external functions definition]==========================*/


static void msjUART_task(void *pvParameter){
	while(true){
		if(control_OnOff){ 
			//UartSendString(UART_PC,"Cargando\r\n");				
		}else if (!control_OnOff){
			if (control_estado==3){
			UartSendString(UART_PC, "Apagado\r\n");
			}else if(control_estado==0){ 
				UartSendString(UART_PC,"Fin de carga \r\n");
				control_estado=3;
			}else if(control_estado==1){
				UartSendString(UART_PC, "Has retirado el recipiente\r\n");
				control_estado=3;
		}

		vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
	
		}
	}
}


static void controlDeDerrames_task (void *pvParameter) {

    while (1) {
        
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		if (control_OnOff) {

			distancia=HcSr04ReadDistanceInCentimeters(); 
			if (distancia<distanciaLimite_cm){
                LedOn(LED_1);
                LedOn(LED_2);
                LedOn(LED_3);
				GPIOOn(gpio5v.pin);
				control_estado=0;
				control_OnOff=0;



            } else if (distancia<20){
                LedOff(LED_1);
                LedOn(LED_2);
                LedOn(LED_3);
				GPIOOff(gpio5v.pin);


            }else if(distancia<distanciaGround_cm){
                LedOff(LED_1);
                LedOff(LED_2);
                LedOn(LED_3);
				GPIOOff(gpio5v.pin);


            }else{
                LedsOffAll();
				GPIOOn(gpio5v.pin);	//GPIO19 en cero
				control_estado=1;
				control_OnOff=0;
                
            }
		
	
	

	}else if(!control_OnOff){
				GPIOOn(gpio5v.pin);	//GPIO19 en cero
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

	serial_config_t my_uart={
		.port=UART_PC,
		.baud_rate = 19200,
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

	xTaskCreate(&controlDeDerrames_task, "", 8192, NULL, 5, &controlDeDerrames_task_handle);
	xTaskCreate(&msjUART_task, "", 8192, NULL, 4, &UART_task_handle);

	TimerStart(timer_controlDeDerrames.timer);


}
