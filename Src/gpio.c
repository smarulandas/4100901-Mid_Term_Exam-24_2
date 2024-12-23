#include "gpio.h"
#include "rcc.h"
#include "systick.h"

// Definiciones de registros y direcciones de periféricos
#define EXTI_BASE 0x40010400 // Dirección base del controlador de interrupciones externas (EXTI)
#define EXTI ((EXTI_t *)EXTI_BASE) // Definición para acceder a los registros del EXTI

// Definición de la dirección base para el controlador de interrupciones externas (EXTI)
#define EXTI15_10_IRQn 40
#define NVIC_ISER1 ((uint32_t *)(0xE000E104)) // NVIC Interrupt Set-Enable Register (Registro para habilitar interrupciones en el NVIC)

#define SYSCFG_BASE 0x40010000 // Dirección base del controlador SYSCFG
#define SYSCFG ((SYSCFG_t *)SYSCFG_BASE) // Definición para acceder a los registros del SYSCFG

// Direcciones base para los puertos GPIOA y GPIOC
#define GPIOA ((GPIO_t *)0x48000000) // Base address of GPIOA (Dirección base del puerto GPIOA)
#define GPIOC ((GPIO_t *)0x48000800) // Base address of GPIOC (Dirección base del puerto GPIOC)

// Definiciones de pines específicos
#define LED_HEARTBEAT_PIN 5 // Pin 5 of GPIOA (Definición del pin 5 en GPIOA (LED de latido))
#define BUTTON_CONTROL_PIN 13 // Pin 13 of GPIOC (Definición del pin 13 en GPIOC (botón de control)

// Configuración del GPIO para USART
void configurar_gpio_usart(void)
{
    // Habilitar el reloj para GPIOA
    *RCC_AHB2ENR |= (1 << 0);

    // Configurar PA2 como función alternativa para TX
    GPIOA->MODER &= ~(3U << (2 * 2)); // Borra los bits de modo para PA2
    GPIOA->MODER |= (2U << (2 * 2));  // Establece el modo de función alternativa para PA2

    // Configurar PA3 como función alternativa para RX
    GPIOA->MODER &= ~(3U << (3 * 2)); // Borra los bits de modo para PA3
    GPIOA->MODER |= (2U << (3 * 2));  // Establece el modo de función alternativa para PA3

    // Configurar la función alternativa AF7 para PA2 y PA3
    GPIOA->AFR[0] &= ~(0xF << (4 * 2)); // Limpia los bits AFR para PA2
    GPIOA->AFR[0] |= (7U << (4 * 2));   // Establece AFR a AF7 para PA2
    GPIOA->AFR[0] &= ~(0xF << (4 * 3)); // Limpia los bits AFR para PA3
    GPIOA->AFR[0] |= (7U << (4 * 3));   // Establece AFR a AF7 para PA3

    // Configurar PA2 y PA3 con velocidad muy alta
    GPIOA->OSPEEDR |= (3U << (2 * 2)); // Velocidad alta para PA2
    GPIOA->OSPEEDR |= (3U << (3 * 2)); // Velocidad alta para PA3

    // Configurar PA2 y PA3 sin resistencias pull-up ni pull-down
    GPIOA->PUPDR &= ~(3U << (2 * 2)); // Sin resistencias pull-up ni pull-down para PA2
    GPIOA->PUPDR &= ~(3U << (3 * 2)); // Sin resistencias pull-up ni pull-down para PA3
}

// Configuración general de GPIO
void configurar_gpio(void)
{
   // Habilitar el reloj para GPIOA y GPIOC
   *RCC_AHB2ENR |= (1 << 0) | (1 << 2);

    // Configurar PA5 como salida para el LED
    GPIOA->MODER &= ~(3U << (LED_HEARTBEAT_PIN * 2)); // Borra los bits de modo para PA5
    GPIOA->MODER |= (1U << (LED_HEARTBEAT_PIN * 2));  // Configura como salida para PA5

    // Configurar PC13 como entrada para el botón
    GPIOC->MODER &= ~(3U << (BUTTON_CONTROL_PIN * 2)); // Borra los bits de modo para PC13

    // Habilitar el reloj para SYSCFG
    *RCC_APB2ENR |= (1 << 0); // Habilita SYSCFG

    // Configurar SYSCFG EXTICR para mapear EXTI13 a PC13
    SYSCFG->EXTICR[3] &= ~(0xF << 4); // Limpia los bits para EXTI13
    SYSCFG->EXTICR[3] |= (0x2 << 4);  // Asigna EXTI13 al puerto C

    // Configurar EXTI13 para activarse en el flanco de bajada
    EXTI->FTSR1 |= (1 << BUTTON_CONTROL_PIN);  // Habilita disparo por flanco de bajada
    EXTI->RTSR1 &= ~(1 << BUTTON_CONTROL_PIN); // Deshabilita disparo por flanco de subida

    // Desenmascarar EXTI13 para habilitar la interrupción
    EXTI->IMR1 |= (1 << BUTTON_CONTROL_PIN);

    // Habilitar la interrupción EXTI15_10 en el NVIC
    *NVIC_ISER1 |= (1 << (EXTI15_10_IRQn - 32));
}

// Control del estado del LED de la puerta
void cambiar_estado_led_puerta(uint8_t estado) {
    if (estado) {
        GPIOA->ODR |= (1 << 4); // Enciende el LED de estado de puerta
    } else {
        GPIOA->ODR &= ~(1 << 4); // Apaga el LED de estado de puerta
    }
}

// Alterna el estado del LED de latido
void alternar_led_latido(void) {
    GPIOA->ODR ^= (1 << 5);
}

volatile uint8_t boton_presionado = 0; // Variable para indicar si el botón fue presionado
uint8_t obtener_evento_boton(void)
{
    uint8_t evento = boton_presionado;
    boton_presionado = 0;
    return evento;
}

uint32_t tiempo_boton = 0;
void detectar_presion_boton(void)
{
    if (systick_GetTick() - tiempo_boton < 50) {
        return; // Ignora rebotes menores a 50 ms
    } else if (systick_GetTick() - tiempo_boton > 500) {
        boton_presionado = 1; // Presión simple
    } else {
        boton_presionado = 2; // Presión doble
    }

    tiempo_boton = systick_GetTick();
}

// Manejo de la interrupción EXTI para el botón
void EXTI15_10_IRQHandler(void)
{
    if (EXTI->PR1 & (1 << BUTTON_CONTROL_PIN)) {
        EXTI->PR1 = (1 << BUTTON_CONTROL_PIN); // Borra el bit pendiente
        detectar_presion_boton();
    }
}
