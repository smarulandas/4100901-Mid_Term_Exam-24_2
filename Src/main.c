// Access Control System Implementation

#include "gpio.h"
#include "systick.h"
#include "uart.h"

#define TEMP_UNLOCK_DURATION 5000 // Duration in ms for temporary unlock

typedef enum {
    LOCKED,
    TEMP_UNLOCK,
    PERM_UNLOCK
} DoorState_t;

DoorState_t current_state = LOCKED;
uint32_t unlock_timer = 0;

// Máquina de estados para gestionar las transiciones entre estados de la puerta
void run_state_machine(void) {
    switch (current_state) {
        case LOCKED:
             // No se realiza ninguna acción periódica cuando está bloqueado
            break;
        case TEMP_UNLOCK:
        // Si el tiempo transcurrido supera la duración configurada, volver al estado bloqueado
            if (systick_GetTick() - unlock_timer >= TEMP_UNLOCK_DURATION) {
                gpio_set_door_led_state(0); // Apagar el LED que indica el estado de la puerta
                current_state = LOCKED; // Cambiar al estado bloqueado
            }
            break;
        case PERM_UNLOCK:
            // No se realiza ninguna acción periódica en el estado de desbloqueo permanente
            break;
    }
}

void handle_event(uint8_t event) {
     if (event == 1) { // Evento: pulsación única del botón
        usart2_send_string("Single Button Press\r\n"); // Notificar evento por UART
        gpio_set_door_led_state(1); // Encender el LED indicador de estado
        current_state = TEMP_UNLOCK; // Cambiar al estado de desbloqueo temporal
        unlock_timer = systick_GetTick(); // Iniciar temporizador para el desbloqueo temporal
    } else if (event == 2) { // Evento: doble pulsación del botón
        usart2_send_string("Double Button Press\r\n"); // Notificar evento por UART
        gpio_set_door_led_state(1); // Encender el LED indicador de estado
        current_state = PERM_UNLOCK; // Cambiar al estado de desbloqueo permanente
    } else if (event == 'O') { // Evento: comando UART para abrir
        usart2_send_string("Open Command Received\r\n"); // Notificar comando recibido
        gpio_set_door_led_state(1); // Encender el LED indicador de estado
        current_state = TEMP_UNLOCK; // Cambiar al estado de desbloqueo temporal
        unlock_timer = systick_GetTick(); // Iniciar temporizador para el desbloqueo temporal
    } else if (event == 'C') { // Evento: comando UART para cerrar
        usart2_send_string("Close Command Received\r\n"); // Notificar comando recibido
        gpio_set_door_led_state(0); // Apagar el LED indicador de estado
        current_state = LOCKED; // Cambiar al estado bloqueado
    }
}

// Función principal donde se inicializa el sistema y se controla el flujo del programa
int main(void) {
    configure_systick_and_start(); // Configurar e iniciar el temporizador de sistema
    configure_gpio();              // Configurar los pines GPIO necesarios
    usart2_init();                 // Inicializar el módulo UART

    usart2_send_string("System Initialized\r\n");  // Enviar mensaje de inicialización
    
    uint32_t heartbeat_tick = 0; // Variable para controlar el parpadeo del LED indicador
    while (1) { 
        // Alternar el estado del LED cada 500 ms para indicar actividad del sistema
        if (systick_GetTick() - heartbeat_tick >= 500) {
            heartbeat_tick = systick_GetTick();
            gpio_toggle_heartbeat_led(); // Cambiar estado del LED de actividad
        }

          uint8_t boton_presionado = button_driver_get_event();
        if (boton_presionado != 0) { // Si se detectó un evento
            usart2_send_string("Button Pressed\r\n"); // Informar que el botón fue presionado
            handle_event(boton_presionado); // Procesar el evento detectado
            boton_presionado = 0; // Restablecer el estado del botón
        }

        // Verificar si se recibió algún comando UART
        uint8_t rx_byte = usart2_get_command();
        if (rx_byte != 0) { // Si se recibió un comando válido
            usart2_send_string("Command Received\r\n"); // Informar que se recibió un comando
            handle_event(rx_byte); // Procesar el comando recibido
        }
        // Ejecutar la lógica de la máquina de estados
        run_state_machine();
    }
}