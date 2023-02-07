
    
/*
 * 
 * File:   newmain.c
 * Author: joe
 *
 * Created on June 17, 2020, 9:30 PM
 */
#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF        // Watchdog Timer Enable bit (WDT enabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

#define _XTAL_FREQ 20000000

#define byte unsigned char
#define uint unsigned int

#include <xc.h>
#include "uart.h"
#include "CRC16.h"

#define ADDRESS 1
#define DIs 16
#define Cls 16
#define IRs 2
#define HRs 3
#define byte unsigned char
#define uint unsigned int


const byte address = ADDRESS;
byte PDU[10] = {0};
byte count;
byte status;
byte checksum [2] = {0};
uint CRC = 0;
byte DiscreteInputs [DIs] = {0};
byte Coils [Cls] = {0};
uint InputRegisters [IRs] = {0};
uint HoldingRegisters [HRs] = {0};



void writePDU(char count)
{
    for (int i=0; i<count;i++)
    uart_write (PDU[i]); 
}

void sendexception(byte functioncode, byte errorcode)
{
    PDU[0] = address;
    PDU[1] = functioncode | 0x80;
    PDU[2] = errorcode;
    CRC=CRC16 (PDU,3);
    PDU[3] = (byte) (CRC & 0xFF);
    PDU[4] = (byte) ((CRC>>8)&0xFF);
    writePDU(5);//send pdu
}

byte getPDU (void)
{
    byte i = 0;
    
    byte nineth=RCSTAbits.RX9D;
    PDU[i]=RCREG;
    
        while(i<=6)
        {
            TMR0 = 134;
            while(1)
            {
                if(uart_dataready())
                {
                    i++;
                    break;
                }
                else if (INTCONbits.TMR0IF)
                {
                    INTCONbits.TMR0IF=0;    
                    return i;
                }
            }

            nineth = RCSTAbits.RX9D;
            PDU [i] =RCREG;
            if (nineth != even_parity(PDU[i]))
            {
             status = 5;    
             return 0;   
            }
            
            
        }
            
}

byte processPDU (byte count) // where should i send the exceptions ???
{
    if (PDU[0] != address)
        return 2;       //if invalid address, break from function and return 2;
    
    byte index=2;
    
    CRC = CRC16(PDU,count);
    checksum [1] = (byte) ((CRC>>8)&0xFF);
    checksum [0] = (byte) (CRC & 0xFF); 
    if (!(PDU[count]==checksum[1] && PDU[count-1]==checksum[0]))
        return 3;       //if wrong CRC , break from function and return 3
    
    
    switch (PDU[1]) // else send exception 01
    {
        case 0x01:   //read coils  
        {
            uint reg = PDU[2]<<8 | PDU[3];
            reg--;
            
            uint coilcount = PDU[4]<<8 | PDU[5];
            
            
            if (reg < 0 || reg > ((Cls-1)-coilcount) )
            {
                sendexception(PDU[1],0x02);
                return -2; //exception 02
            }    
            if (coilcount > Cls || coilcount < 1)
            {
                sendexception(PDU[1],0x03);
                return -3; //exception 03
            }
            
            for (int i=2; i<10; i++)
            {
                PDU[i]=0x00;
            }
            
            byte coilbytescount = coilcount / 8;
            byte coilsleft = coilcount % 8;
            
            if (coilcount%8)
                PDU[index]=coilbytescount+1;
            else
                PDU[index]=coilbytescount;
            index++;
                  
      
            byte buff ;
            
            for (int i=0 ; i < coilbytescount ; i++) 
            {
                buff = 0x00;
                for (int j=0 ; j<8 ; j++ ) 
                {
                    buff = buff>>1;
                  if (Coils[reg+(i*8)+j])
                      buff |= 0xA0;
                }
                PDU[index++]=buff;
            }
            
            if(coilsleft)
            {
                for(int i=0;i<coilsleft;i++)
                {
                    buff = 0x00;
                    buff=buff>>1;
                    if(Coils[reg+(coilbytescount*8)+i])
                        buff|=0xA0;
                }
            PDU[index++]=buff;
            }
          
            CRC=CRC16 (PDU,index);
            PDU[index++] = (byte) (CRC & 0xFF);
            PDU[index++] = (byte) ((CRC>>8)&0xFF);
        //index is now total number of bytes
            writePDU(index);
        } //case 01
        break;
        
        case 0x02://read discrete inputs
        {
            uint reg = PDU[2]<<8 | PDU[3];          
            reg--;
            
            uint coilcount = PDU[4]<<8 | PDU[5];
            
            
            if (reg < 0 || reg > ((DIs-1)-coilcount) )
            {
                sendexception(PDU[1],0x02);
                return -2; //exception 02
            }
            if (coilcount > DIs || coilcount < 1)
            {
                sendexception(PDU[1],0x03);
                return -3; //exception 03
            }
            for (int i=2; i<10; i++)
            {
                PDU[i]=0x00;
            }
            
            byte coilbytescount = coilcount / 8;
            byte coilsleft = coilcount % 8;
            
            if (coilcount%8)
                PDU[index]=coilbytescount+1;
            else
                PDU[index]=coilbytescount;
            index++;
                  
      
            byte buff ;
            
            for (int i=0 ; i < coilbytescount ; i++) 
            {
                buff = 0x00;
                for (int j=0 ; j<8 ; j++ ) 
                {
                    buff = buff>>1;
                  if (DiscreteInputs[reg+(i*8)+j])
                      buff |= 0xA0;
                }
                PDU[index++]=buff;
            }
            
            if(coilsleft)
            {
                for(int i=0;i<coilsleft;i++)
                {
                    buff = 0x00;
                    buff=buff>>1;
                    if(DiscreteInputs[reg+(coilbytescount*8)+i])
                        buff|=0xA0;
                }
            PDU[index++]=buff; 
            }
            
            CRC=CRC16 (PDU,index);
            PDU[index++] = (byte) (CRC & 0xFF);
            PDU[index++] = (byte) ((CRC>>8)&0xFF);
        
            writePDU(index);
        }
        break;

        
        case 0x03: // read holding registers
        {
            uint regadd = PDU[2]<<8 | PDU[3];            
            regadd--;
            
            uint regcount = PDU[4]<<8 | PDU[5];
            
            
            if (regadd < 0 || regadd > ((HRs-1)-regcount) )
            {
                sendexception(PDU[1],0x02);
                return -2; //exception 02
            }
            if (HRs > 3 || regcount < 1)
            {
                sendexception(PDU[1],0x03);
                return -3; //exception 03
            }
            for (int i=2; i<10; i++)
            {
                PDU[i]=0x00;
            }
            
            PDU[index++] = 2*regcount;
            
            for (int i=regadd; i<(regadd+regcount);i++)
            {
                PDU[index++]=HoldingRegisters[i]>>8;
                PDU[index++]=HoldingRegisters[i];
                
            }
            
            CRC=CRC16 (PDU,index);
            PDU[index++] = (byte) (CRC & 0xFF);
            PDU[index++] = (byte) ((CRC>>8)&0xFF);
            
            writePDU(index);
        }
        break;
        
        case 0x04: // Read input registers
        {
            uint regadd = PDU[2]<<8 | PDU[3];            
            regadd--;
            
            uint regcount = PDU[4]<<8 | PDU[5];
            
            
            if (regadd < 0 || regadd > ((IRs-1)-regcount) )
            {
                sendexception(PDU[1],0x02);
                return -2; //exception 02
            }
            if (HRs > 3 || regcount < 1)
            {
                sendexception(PDU[1],0x03);
                return -3; //exception 03
            }
            for (int i=2; i<10; i++)
            {
                PDU[i]=0x00;
            }
            
            PDU[index++] = 2*regcount;
            
            for (int i=regadd; i<(regadd+regcount);i++)
            {
                PDU[index++]=InputRegisters[i]>>8;
                PDU[index++]=InputRegisters[i];
            }
            
            CRC=CRC16 (PDU,index);
            PDU[index++] = (byte) (CRC & 0xFF);
            PDU[index++] = (byte) ((CRC>>8)&0xFF);
            writePDU(index);
            
        }
        break;
        
        case 0x05: //write single coil
        {
            uint regadd = PDU[2]<<8 | PDU[3];            
            regadd--;
            uint valueint = PDU[4]<<8 | PDU[5];
            
            if(valueint != (0xFF00) && valueint != (0xFFFF))
            {
                sendexception(PDU[1],0x03);
                return -3 ;//exception 0x03
            }
            if(regadd < 0 || regadd > Cls )
            {
                sendexception(PDU[1],0x02);
                return -2 ;//exception 0x02
            }
            if (valueint==0xFFFF)
                Coils[regadd] = 0xFF ;
            else if (valueint==0xFF00)
                Coils[regadd] = 0x00 ;
            
            index=6;
            
            CRC=CRC16 (PDU,index);
            PDU[index++] = (byte) (CRC & 0xFF);
            PDU[index++] = (byte) ((CRC>>8)&0xFF);
            writePDU(index);
        }
        break;
        
        case 0x06: //write single holding register
        {
            uint regadd = PDU[2]<<8 | PDU[3];            
            regadd--;
            uint valueint = PDU[4]<<8 | PDU[5];
            
            if(valueint < 0x0000 || valueint > 0xFFFF)
            {
                sendexception(PDU[1],0x03);
                return -3; // exception 0x03
            }
            if (regadd < 0||regadd > HRs )
            {
                sendexception(PDU[1],0x02);
                return -2; // exception 0x02
            }
            HoldingRegisters[regadd]=valueint;
            index = 6;
            
            CRC=CRC16 (PDU,index);
            PDU[index++] = (byte) (CRC & 0xFF);
            PDU[index++] = (byte) ((CRC>>8)&0xFF);
            writePDU(index);
        }
        break;
        
        default:
        {
        sendexception(PDU[1],0x01);
        //send exception 01
        }
        
       } //switch case
               return 0;    
 }//whole function
  
    

void __interrupt() myISR()
{
        
        
    if (RCIF==1)
    {
        
        count = getPDU();
        if (count >= 6)
            status= processPDU(count);        
        else 
            status=1; //less than 7 bytes received
        
    }
    
}

void main(void) 
{

    TRISD=0x00;
    uart_init(9600);
    
    PIE1bits.RCIE=1;
    OPTION_REGbits.T0CS = 0;
    OPTION_REGbits.T0SE = 0; 
    OPTION_REGbits.PSA = 0; 
    OPTION_REGbits.PS2 = 1; 
    OPTION_REGbits.PS1 = 0;
    OPTION_REGbits.PS0 = 1;
    
    
    
    while(1)
    {
     
        PORTD = Coils[0];
     
    }
  
    
    
    
    
    return ;
}
