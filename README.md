# intButton-pico-ws2812
Tarefa 1- Aula Síncrona 27/01/2025 - Embarcatech-CEPEDI
Em termos gerais este projeto está dividido em:
1)Programa principal (main()):
  executa um loop infinto, fazendo um led piscar numa frequência de 5 Hz (Ligado:100ms e desligado:100ms);
  verifica a necessidade de atualizar o display (matriz 5x5 de leds ws2812) e em caso positivo, chama a devida função.
  Isto é feito testando uma flag manipulada na ISR dos botões.
2)Interrupção de GPIO:
  A função gpio_set_irq_enabled_with_callback() só pode ser chamada uma vez e registra apenas um callback global para todas as interrupções.
  Como há dois botões (A e B), a mesma ISR será usada para ambos.
  Conteúdo da ISR:
  a)Desativa temporariamente a interrupção no "GPIO do botão pressionado" para eventos de borda de descida.
    Neste caso é útil para evitar que o botão gere interrupções repetidas antes de terminar o debounce.
  b)Deabaucing - É computado verificando a distância temporal entre as interrupções, com o auxílio da função time_us_64().
    Caso a distância seja maior que o tempo de debounce especificado (150ms), a interrupção é considerada válida e a ISR é processada até o fim.
    Este método não bloqueia a CPU, como é o caso da função sleep_ms(), que não é recomendada para ISRs.
  c)Testa qual botão foi pressionado e incrementa ou decrementa o dígito atual.
  d)Seta uma variável (flag), que informa a produção de um novo dígito (será testada pela main() para atualizar o display).
  e)Reativa as interrupções.
3)Funções para a atualização do display (matriz de leds ws2812b) utilizando o módulo PIO.
  O módulo PIO é um coprocessador programável que pode executar tarefas paralelas sem ocupar a CPU. 
  No controle da matriz ws2812, a CPU prepara os dados do dígito e os envia ao PIO, que gera os sinais de temporização precisos para acender os LEDs corretamente. 
  Isso libera a CPU para a lógica do programa, enquanto o PIO trabalha diretamente no envio dos dados para a matriz de leds.
  As funções em C-SDK informam a sequência de acendimento (a matriz do dígito configurada para o esquema elétrico da matriz de leds na BitDogLab) e a cor/intensidade,que o LED deve ser aceso.
## Vídeo Explicativo
Confira o vídeo no link abaixo:
[![Assista no YouTube](https://img.youtube.com/vi/DOgGAeboZPo/maxresdefault.jpg)](https://youtu.be/DOgGAeboZPo?si=bDhn3wvPkQE3Qnap)
