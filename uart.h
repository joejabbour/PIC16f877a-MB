/* 
 * File:   uart.h
 * Author: joe
 *
 * Created on July 31, 2020, 7:54 PM
 */

#ifndef UART_H
#define	UART_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#define uint unsigned int
#define byte unsigned char    
    
    char uart_init(const long int baudrate)
{
    unsigned int x;
    x=(_XTAL_FREQ - baudrate*64)/(baudrate*64);
    if(x>255)
    {
        x=(_XTAL_FREQ - baudrate*16)/(baudrate*16);
        BRGH=1;
    }
    if(x<256)
    {
        SPBRG=x;
        SYNC=0;
        SPEN=1;
        RX9=0; 
        TRISC7=1;
        TRISC6=0;
        CREN=1;
        TXEN=1;
        return 1;
    }
    return 0;
}

void modbus_write(){
    //take into consideration the time between bytes, probably make it more compact by doing the loop here
}

void uart_write(char data){
    while(!TRMT);
    //TX9D = even_parity(data);
    TXREG=data;
}

char uart_tx_empty(){
    return TRMT;
}

void uart_writetext(char *text)
{
    int i;
    for(i=0;text[i]!='\0';i++)
        uart_write(text[i]);
}


char uart_dataready(){
    return RCIF;
}

char uart_read()
{
     if(OERR) // check for Error 
    {
        CREN = 0; //If error -> Reset 
        CREN = 1; //If error -> Reset 
    }
    while(!RCIF);
    return RCREG;
}

void uart_readtext(char *Output, unsigned int length)
{
    unsigned int i;
    for(int i=0;i<length;i++)
        Output[i]=uart_read();
}

byte even_parity(byte chbyte )
{
    byte parity = 0;
    byte tool = 0b00000001;
    for (int i=0 ; i<=7 ; i++){
        if (tool&&chbyte){
            parity=!parity;
        }
        tool = tool << 1;
        
    }
    return parity;
}                                                

#ifdef	__cplusplus
}
#endif

#endif	/* UART_H */

