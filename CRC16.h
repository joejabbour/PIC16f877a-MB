/* 
 * File:   mb.h
 * Author: joe
 *
 * Created on October 23, 2020, 9:37 PM
 */

#ifndef MB_H
#define	MB_H


#define uint unsigned int

uint CRC16 (byte PDU[],char count ) //current index incremented or count without CRC
{

    uint reg_crc = 0xFFFF;
    
    for (int i=0 ; i < (count-2) ;i++)
    {
        reg_crc ^= PDU[i];
        for (int j=0 ; j<8 ;j++)
        {
            if((reg_crc&0x01)==1)
            {
                reg_crc= (uint) ((reg_crc>>1)^0xA001);
            }
            else
            {
                reg_crc= (uint) (reg_crc>>1);
            }
            
            
        }
    }

    return reg_crc;
}









#ifdef	__cplusplus
extern "C" {
#endif




#ifdef	__cplusplus
}
#endif

#endif	/* MB_H */

