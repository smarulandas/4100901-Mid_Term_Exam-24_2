// uart.h
#ifndef UART_H
#define UART_H

#include <stdint.h>

// Base addresses for USART peripherals
#define USART2_BASE  0x40004400UL

// Define USART_TypeDef structure to match hardware registers
typedef struct {
   volatile uint32_t CR1;    // Registro de control 1 (configuración principal)
    volatile uint32_t CR2;    // Registro de control 2 (configuración adicional)
    volatile uint32_t CR3;    // Registro de control 3 (control avanzado)
    volatile uint32_t BRR;    // Registro de velocidad en baudios (baud rate)
    volatile uint32_t GTPR;   // Registro de tiempo de guarda y preescalador
    volatile uint32_t RTOR;   // Registro de tiempo de espera del receptor
    volatile uint32_t RQR;    // Registro de solicitud (request)
    volatile uint32_t ISR;    // Registro de estado e interrupciones
    volatile uint32_t ICR;    // Registro para limpiar banderas de interrupción
    volatile uint32_t RDR;    // Registro para recibir datos
    volatile uint32_t TDR;    // Registro para transmitir datos
} USART_TypeDef;

// Asigna la dirección base al puntero USART2 como estructura USART_TypeDef
#define USART2 ((USART_TypeDef *) USART2_BASE)

// Definiciones de bits en el registro USART_CR1 (Control Register 1)
#define USART_CR1_UE      (1U << 0)    // Habilita el módulo USART
#define USART_CR1_M       (1U << 12)   // Longitud de palabra (0 = 8 bits, 1 = 9 bits)
#define USART_CR1_PCE     (1U << 10)   // Habilita el control de paridad
#define USART_CR1_OVER8   (1U << 15)   // Selección del modo de sobremuestreo
#define USART_CR1_TE      (1U << 3)    // Habilita el transmisor
#define USART_CR1_RE      (1U << 2)    // Habilita el receptor
#define USART_CR1_RXNEIE  (1U << 5)    // Habilita la interrupción por recepción de datos

// Definiciones de bits en el registro USART_CR2 (Control Register 2)
#define USART_CR2_STOP    (3U << 12)   // STOP bits (00 = 1 Stop bit)

// Definiciones de bits en el registro USART_ISR (Interrupt and Status Register)
#define USART_ISR_RXNE    (1U << 5)    // Indica que hay datos recibidos disponibles
#define USART_ISR_TXE     (1U << 7)    // Indica que el registro de transmisión está vacío
#define USART_ISR_TEACK   (1U << 21)   // Confirma la habilitación del transmisor
#define USART_ISR_REACK   (1U << 22)   // Confirma la habilitación del receptor

// Valor calculado para configurar la velocidad en baudios a 9600 con un reloj de 4 MHz
#define BAUD_9600_4MHZ   (0x1A1)       // Calculated value for 9600 baud rate with 4 MHz APB clock

// Enumeración para definir los comandos recibidos por UART
typedef enum {
    CMD_NONE = 0,
    CMD_OPEN = 'O', // Comando para abrir (carácter 'O')
    CMD_CLOSE = 'C' // Comando para cerrar (carácter 'C')
} command_t;

// Declaraciones de funciones públicas disponibles en el módulo UART
void usart2_init(void);                   // Inicializa el periférico USART2
void usart2_send_string(const char *str); // Envía una cadena de caracteres por USART
// Función para obtener el último comando recibido (no bloqueante)

// Obtiene el último comando recibido a través de USART2 (sin bloquear la ejecución)
command_t usart2_get_command(void);

#endif // UART_H