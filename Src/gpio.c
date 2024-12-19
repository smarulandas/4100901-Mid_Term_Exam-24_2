#include "gpio.h"
#include "rcc.h"

#include "systick.h"

#define EXTI_BASE 0x40010400
#define EXTI ((EXTI_t *)EXTI_BASE)

#define EXTI15_10_IRQn 40
#define NVIC_ISER1 ((uint32_t *)(0xE000E104)) // NVIC Interrupt Set-Enable Register


#define SYSCFG_BASE 0x40010000
#define SYSCFG ((SYSCFG_t *)SYSCFG_BASE)


#define GPIOA ((GPIO_t *)0x48000000) // Base address of GPIOA
#define GPIOC ((GPIO_t *)0x48000800) // Base address of GPIOC

#define LED_PIN 5 // Pin 5 of GPIOA
#define BUTTON_PIN 13 // Pin 13 of GPIOC
#define RING_LED_PIN 6      // Pin para el LED del timbre



void configure_gpio_for_usart(void)
{
    // Enable GPIOA clock
    *RCC_AHB2ENR |= (1 << 0);

    // Configure PA2 (TX) as alternate function
    GPIOA->MODER &= ~(3U << (2 * 2)); // Clear mode bits for PA2
    GPIOA->MODER |= (2U << (2 * 2));  // Set alternate function mode for PA2

    // Configure PA3 (RX) as alternate function
    GPIOA->MODER &= ~(3U << (3 * 2)); // Clear mode bits for PA3
    GPIOA->MODER |= (2U << (3 * 2));  // Set alternate function mode for PA3

    // Set alternate function to AF7 for PA2 and PA3
    GPIOA->AFR[0] &= ~(0xF << (4 * 2)); // Clear AFR bits for PA2
    GPIOA->AFR[0] |= (7U << (4 * 2));   // Set AFR to AF7 for PA2
    GPIOA->AFR[0] &= ~(0xF << (4 * 3)); // Clear AFR bits for PA3
    GPIOA->AFR[0] |= (7U << (4 * 3));   // Set AFR to AF7 for PA3

    // Configure PA2 and PA3 as very high speed
    GPIOA->OSPEEDR |= (3U << (2 * 2)); // Very high speed for PA2
    GPIOA->OSPEEDR |= (3U << (3 * 2)); // Very high speed for PA3

    // Configure PA2 and PA3 as no pull-up, no pull-down
    GPIOA->PUPDR &= ~(3U << (2 * 2)); // No pull-up, no pull-down for PA2
    GPIOA->PUPDR &= ~(3U << (3 * 2)); // No pull-up, no pull-down for PA3
}


void configure_gpio(void)
{
    //Codigo mio

    // Habilitar relojes para GPIOA y GPIOC
    *RCC_AHB2ENR |= (1 << 0);  // GPIOA clock
    *RCC_AHB2ENR |= (1 << 2);  // GPIOC clock
    
    // Configurar PA4 (LED de estado de puerta) como salida
    GPIOA->MODER &= ~(3U << (4 * 2));  // Limpiar bits de modo
    GPIOA->MODER |= (1U << (4 * 2));   // Configurar como salida
    GPIOA->OTYPER &= ~(1 << 4);        // Push-pull
    GPIOA->OSPEEDR |= (3U << (4 * 2)); // Very high speed
    GPIOA->PUPDR &= ~(3U << (4 * 2));  // No pull-up/pull-down
    
    // Configurar PA5 (LED de heartbeat) como salida
    GPIOA->MODER &= ~(3U << (5 * 2));  // Limpiar bits de modo
    GPIOA->MODER |= (1U << (5 * 2));   // Configurar como salida
    GPIOA->OTYPER &= ~(1 << 5);        // Push-pull
    GPIOA->OSPEEDR |= (3U << (5 * 2)); // Very high speed
    GPIOA->PUPDR &= ~(3U << (5 * 2));  // No pull-up/pull-down
    
    // Configurar PC13 (Botón) como entrada con interrupción
    GPIOC->MODER &= ~(3U << (13 * 2)); // Modo entrada (00)
    GPIOC->PUPDR |= (1U << (13 * 2));  // Pull-up
    
    // Habilitar SYSCFG clock (necesario para EXTI)
    *RCC_APB2ENR |= (1 << 0);
    
    // Conectar EXTI13 a PC13
    SYSCFG->EXTICR[3] &= ~(0xF << 4);  // Limpiar bits EXTI13
    SYSCFG->EXTICR[3] |= (2 << 4);     // PC13 como fuente
    
    // Configurar EXTI para flanco descendente
    EXTI->FTSR1 |= (1 << BUTTON_PIN);  // Habilitar trigger por flanco descendente
    EXTI->RTSR1 &= ~(1 << BUTTON_PIN); // Deshabilitar trigger por flanco ascendente
    EXTI->IMR1 |= (1 << BUTTON_PIN);   // Habilitar interrupción
    EXTI->PR1 = (1 << BUTTON_PIN);     // Limpiar cualquier interrupción pendiente

    
    // Habilitar interrupción EXTI en NVIC
    *NVIC_ISER1 |= (1 << (EXTI15_10_IRQn - 32));

    GPIOA->MODER &= ~(3U << (RING_LED_PIN * 2));  // Limpiar bits de modo
    GPIOA->MODER |= (1U << (RING_LED_PIN * 2));   // Configurar como salida
    GPIOA->OTYPER &= ~(1 << RING_LED_PIN);        // Push-pull
    GPIOA->OSPEEDR |= (3U << (RING_LED_PIN * 2)); // Very high speed
    GPIOA->PUPDR &= ~(3U << (RING_LED_PIN * 2));  // No pull-up/pull-down

    //Hasta aqui, por si se daña saber que hice
}

    void gpio_set_bell_event(uint8_t state) {
    if (state) {
        usart2_send_string("BELL_RING\r\n"); // Notificar el evento del timbre
    }
}

// Emula el comprtamiento de la puerta
void gpio_set_door_led_state(uint8_t state) {
    if (state) {
        GPIOA->ODR |= (1 << 4); // encender LED estado puerta
    } else {
        GPIOA->ODR &= ~(1 << 4); // apagar LED estado puerta
    }
    if (state) {
        GPIOA->ODR |= (1 << RING_LED_PIN); // Encender LED del timbre
    } else {
        GPIOA->ODR &= ~(1 << RING_LED_PIN); // Apagar LED del timbre
    }
}

void gpio_toggle_heartbeat_led(void) {
    GPIOA->ODR ^= (1 << 5);
}

volatile uint8_t button_pressed = 0; // Flag to indicate button press
uint8_t button_driver_get_event(void)
{
    return button_pressed;
}

uint32_t b1_tick = 0;
void detect_button_press(void)
{
    if (systick_GetTick() - b1_tick < 50) {
        return; // Ignore bounces of less than 50 ms
    } else if (systick_GetTick() - b1_tick > 500) {
        button_pressed = 1; // single press
    } else {
        button_pressed = 2; // double press
    }

    b1_tick = systick_GetTick();
}

void EXTI15_10_IRQHandler(void)
{
    if (EXTI->PR1 & (1 << BUTTON_PIN)) {
        EXTI->PR1 = (1 << BUTTON_PIN); // Clear pending bit
        detect_button_press();
    }
}
