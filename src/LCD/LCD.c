//================================================================================ //
// Sub-rotinas para o trabalho com um LCD 16x2 com via de dados de 4 bits //
// Controlador HD44780 - Pino R/W aterrado //
// A via de dados do LCD deve ser ligado aos 4 bits mais significativos ou //
// aos 4 bits menos significativos do PORT do uC //
//================================================================================ //
#include "LCD.h"
//-----------------------------------------------------------------------------------
// Sub-rotina para enviar caracteres e comandos ao LCD com via de dados de 4 bits
//-----------------------------------------------------------------------------------
// c é o dado e cd indica se é instrução ou caractere (0 ou 1)
void cmd_LCD(unsigned char c, char cd){
    if (cd == 0) // instrução
        clr_bit(CONTR_LCD, RS);
    else                        // caractere
        set_bit(CONTR_LCD, RS); // primeiro nibble de dados - 4 MSB
    #if (nibble_dados)              // compila o código para os pinos de dados do LCD nos 4 MSB do PORT
        DADOS_LCD = (DADOS_LCD & 0x0F) | (0xF0 & c);
    #else // compila o código para os pinos de dados do LCD nos 4 LSB do PORT
        DADOS_LCD = (DADOS_LCD & 0xF0) | (c >> 4);
    #endif
        pulso_enable();
    // segundo nibble de dados - 4 LSB
    #if (nibble_dados) // compila o código para os pinos de dados do LCD nos 4 MSB do PORT
        DADOS_LCD = (DADOS_LCD & 0x0F) | (0xF0 & (c << 4));
    #else // compila o código para os pinos de dados do LCD nos 4 LSB do PORT
        DADOS_LCD = (DADOS_LCD & 0xF0) | (0x0F & c);
    #endif
        pulso_enable();
        if ((cd == 0) && (c < 4)) // se for instrução de retorno ou limpeza espera LCD estar pronto
            _delay_ms(2);
}

//-----------------------------------------------------------------------------------
// Sub-rotina para inicialização do LCD com via de dados de 4 bits
//-----------------------------------------------------------------------------------
// sequência ditada pelo fabricando do circuito integrado HD44780
void inic_LCD_4bits() {     // o LCD será só escrito. Então, R/W é sempre zero.
    clr_bit(CONTR_LCD, RS); // RS em zero indicando que o dado para o LCD será uma instrução
    clr_bit(CONTR_LCD, E);  // pino de habilitação em zero
    _delay_ms(20);          /*tempo para estabilizar a tensão do LCD, após VCC ultrapassar 4.5 V (na prática pode ser maior).*/
// interface de 8 bits
    #if (nibble_dados)
        DADOS_LCD = (DADOS_LCD & 0x0F) | 0x30;
    #else
        DADOS_LCD = (DADOS_LCD & 0xF0) | 0x03;
    #endif

    pulso_enable(); // habilitação respeitando os tempos de resposta do LCD
    _delay_ms(5);
    pulso_enable();
    _delay_us(200);
    pulso_enable(); // até aqui ainda é uma interface de 8 bits.
// interface de 4 bits, deve ser enviado duas vezes (a outra está abaixo)
    #if (nibble_dados)
        DADOS_LCD = (DADOS_LCD & 0x0F) | 0x20;
    #else
        DADOS_LCD = (DADOS_LCD & 0xF0) | 0x02;
    #endif
        pulso_enable();
        cmd_LCD(0x28, 0); // interface de 4 bits 2 linhas (aqui se habilita as 2 linhas)
        // são enviados os 2 nibbles (0x2 e 0x8)
        cmd_LCD(0x08, 0); // desliga o display
        cmd_LCD(0x01, 0); // limpa todo o display
        cmd_LCD(0x0C, 0); // mensagem aparente cursor inativo não piscando
        cmd_LCD(0x80, 0); // inicializa cursor na primeira posição a esquerda - 1a linha
}

//------------------------------------------------------------------------------------
// Sub-rotina de escrita no LCD - dados armazenados na RAM
//------------------------------------------------------------------------------------
void escreve_LCD(char *c){
    for (; *c != 0; c++)
        cmd_LCD(*c, 1);
}

//------------------------------------------------------------------------------------
// Sub-rotina de escrita no LCD - dados armazenados na FLASH
//------------------------------------------------------------------------------------
void escreve_LCD_Flash(const char *c){
    for (; pgm_read_byte(&(*c)) != 0; c++)
        cmd_LCD(pgm_read_byte(&(*c)), 1);
}

//------------------------------------------------------------------------------------
// Conversão de um número em seus digitos individuais – função auxiliar
//-----------------------------------------------------------------------------------
void ident_num(unsigned int valor, unsigned char *disp){
    unsigned char n;
    for (n = 0; n < tam_vetor; n++)
        disp[n] = 0 + conv_ascii; // limpa vetor para armazenagem dos digitos
    do
    {
        *disp = (valor % 10) + conv_ascii; // pega o resto da divisão por 10
        valor /= 10;                       // pega o inteiro da divisão por 10
        disp++;
    } while (valor != 0);
}
//-----------------------------------------------------------------------------------