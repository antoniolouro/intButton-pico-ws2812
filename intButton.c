/************************************* 
Tarefa 1 da Aula Síncrona de 27/01/2025
Aluno:Antonio Heinrique Figueira Louro
tic370100080
 
 ************************************/


#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "ws2812.pio.h"
#include "pico/time.h"
#include "hardware/clocks.h"


#define NUM_PIXELS 25   // Matriz 5x5
#define WS2812_PIN 7    // Pino de saída dos LEDs
#define BUTTON_A 5    // Pino do botão A=5
#define BUTTON_B 6    // Pino do botão B=6
#define DEBOUNCE_DELAY_MS 150 // Tempo de debounce (150ms)
#define LEDR 13 // Pino do LED vermelho (RGB)

bool atualiza_pixels = false; // Flag para indicar que a matriz de LEDs deve ser atualizada

// Matrizes 5x5 para os números de 0 a 9 (1 = LED aceso, 0 = LED apagado)
// Cada linha contém 25 elementos - os 25 pixels da Matriz 5x5 ws2812b
// Cada linha corresponde a um dígito em formato similar a de displays de 7 segmentos
//A configuração dos 0s e 1s foi feita de forma a representar os números de 0 a 9 na BitDogLab
const uint8_t digits[10][25] = {
    {0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0}, 	// 0
    {0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0}, 	// 1
    {1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 1, 1, 0}, 	// 2
    {1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0},	// 3
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0},  	// 4
    {1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 0}, 	// 5
    {1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0}, 	// 6
    {0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0}, 	// 7
    {0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0}, 	// 8
    {1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 0}  	// 9
};



volatile uint8_t current_digit = 0; // Dígito inicial - o apertar do botão levará ao (0)
/* OBS: ISRs podem modificar variáveis que o programa principal também usa. O compilador pode otimizar e "cachear" essas variáveis
 * em registradores, ignorando mudanças feitas na interrupção. Quando uma variável é marcada como volatile, ela força o processador
 * a sempre buscar o valor atualizado da memória RAM, sem cache. Isso garante que leituras e escritas sejam sempre consistentes. */
 //---------------------

// Função para converter 0s e 1s em valores RGB (fundo = azul e dígito em amarelo)
uint32_t matrix_rgb(uint8_t value) {
    return value ? 0xff000000 : 0x00000000; // acende o verde com 10% do brilho
    
    //é uma expressão condicional (operador ternário), equivalente a um if-else
}
//---------------------

/*Callback único para os dois botões
A função gpio_set_irq_enabled_with_callback() só pode ser chamada uma vez, 
e registra apenas um callback global para todas as interrupções.*/

void button_callback(uint gpio, uint32_t events) {
    gpio_set_irq_enabled(gpio, GPIO_IRQ_EDGE_FALL, false);  // Desativa a interrupção temporariamente
    //Debouncing é computado verificando as diferenças de tempo entre as interrupções com o auxílio da função time_us_64()
    //Caso a diferença seja maior que o tempo de debounce, a interrupção é considerada válida
    static uint64_t last_interrupt_time = 0;
    uint64_t current_time = time_us_64();

    if ((current_time - last_interrupt_time) > (DEBOUNCE_DELAY_MS * 1000)) {
        if (gpio == BUTTON_A) {
            current_digit = (current_digit + 1) % 10;  // Incrementa
        } else if (gpio == BUTTON_B) {
            current_digit = (current_digit == 0) ? 9 : current_digit - 1;  // Decrementa
        }
        atualiza_pixels = true;
        last_interrupt_time = current_time;
    }
     gpio_set_irq_enabled(gpio, GPIO_IRQ_EDGE_FALL, true);  // Reativa a interrupção
    
}



// Função para exibir um número na matriz LED
void show_digit(PIO pio, uint sm, uint8_t digit) {
    for (int i = 0; i < NUM_PIXELS; i++) {
        uint32_t color = matrix_rgb(digits[digit][i]);
        pio_sm_put_blocking(pio, sm, color);
    }
}


int main() {
    stdio_init_all();
	gpio_init(LEDR);						// Inicializa o pino do LED vermelho
	gpio_set_dir(LEDR, GPIO_OUT);			// Configura o pino do LED vermelho como saída
    PIO pio = pio0;							//Ponteiro para o módulo pio0
    uint offset = pio_add_program(pio, &ws2812_program);	// Adiciona o programa ws2812b ao pio0 na posição de memória offset
    uint sm = pio_claim_unused_sm(pio, true);				// Reivindica um state machine livre no pio0
    ws2812_program_init(pio, sm, offset, WS2812_PIN);		 // Configuração da PIO para WS2812B

    // Configuração dos botões A e B com pull-up interno
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    // Ativa a interrupção para o botão A e registra um callback
    // Com pull-up, a interrupção é gerada na borda de descida (quando o botão é pressionado)
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &button_callback);
	
	gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);
    gpio_set_irq_enabled(BUTTON_B, GPIO_IRQ_EDGE_FALL, true); // Ativa a interrupção para o botão B sem registrar um callback
    
    while (1) {
		// Piscar o LEDR 5 vezes por segundo -> indica que o programa principal está em execução
		
		gpio_put(LEDR, 1);	// Acende o LED vermelho
		sleep_ms(100);		// tempo de espera em ON
		gpio_put(LEDR, 0);	// Apaga o LED vermelho
		sleep_ms(100);		// tempo de espera em OFF

		// Testa a flag atualiza_pixels para saber se deve atualizar a matriz de leds ws2812b
        // Esta flag é setada no callback do botão A e B, que introduzem incrementos e decrementos no dígito atual
		if (atualiza_pixels) {	//testa a flag de atualização da matriz
			atualiza_pixels = false;		// limpa a flag
			show_digit(pio, sm, current_digit); // Exibe o número atual na matriz
		}
        
    }
}