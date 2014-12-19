#include "ModBus/include/mb.h" 
#include "uks.h"

extern struct uks uks_channels;

void ENTER_CRITICAL_SECTION(void)
{

}

void EXIT_CRITICAL_SECTION(void)
{

}


static volatile u16 usRegInputBuf[64];
u16 *usRegHoldingBuf=usRegInputBuf;

u8 REG_INPUT_START=1,REG_HOLDING_START=1;
u8 REG_INPUT_NREGS=32,REG_HOLDING_NREGS=8;
u8 usRegInputStart=0,usRegHoldingStart=0;


    static float temp=0;
eMBErrorCode
eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{
	eMBErrorCode    eStatus = MB_ENOERR;
	int             iRegIndex;
    uint16_t i=0;

    REG_INPUT_NREGS=(DRYING_CHANNELS_NUM+1)*2;


    temp=temp+0.01;
    for(i=0;i<DRYING_CHANNELS_NUM;i++)
    {
//    	usRegInputBuf[i+1]   =((uint16_t*)&uks_channels.drying_channel_list[i>>1].temperature)[0];
//    	usRegInputBuf[i] =((uint16_t*)&uks_channels.drying_channel_list[i>>1].temperature)[1];

    	((float*)usRegInputBuf)[i] = uks_channels.drying_channel_list[i].temperature;
//    		    float tmp2=temp+i;
//    		    ((float*)usRegInputBuf)[i]=tmp2;
//    	    	usRegInputBuf[i+1]   =((uint16_t*)&tmp2)[0];
//    	    	usRegInputBuf[i] =((uint16_t*)&tmp2)[1];
//    			usRegInputBuf[i]  =0x1111*i;
//    			usRegInputBuf[i+1]=0x1111*i;
    }

//
    ((float*)usRegInputBuf)[DRYING_CHANNELS_NUM] = uks_channels.heater_temperature;
//    ((float*)usRegInputBuf)[DRYING_CHANNELS_NUM+1] +=3.21;//uks_channels.heater_temperature;
//    ((float*)usRegInputBuf)[DRYING_CHANNELS_NUM+2] = 12.3456789;//uks_channels.heater_temperature;

    if( ( usAddress >= REG_INPUT_START )&& ( usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS ) )
    {
        iRegIndex = 0;//( int )( usAddress - usRegInputStart );
        while( usNRegs > 0 )
        {
            *pucRegBuffer++ =
                ( unsigned char )( usRegInputBuf[iRegIndex] >> 8 );
            *pucRegBuffer++ =
                ( unsigned char )( usRegInputBuf[iRegIndex] & 0xFF );
            iRegIndex++;
            usNRegs--;
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }

    return eStatus;

}


eMBErrorCode
eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    int             iRegIndex;
	u16 *PRT=(u16*)pucRegBuffer;

    if( ( usAddress >= REG_HOLDING_START ) && ( usAddress + usNRegs <= REG_HOLDING_START + REG_HOLDING_NREGS ) )
    {
        iRegIndex = ( int )( usAddress - usRegHoldingStart );
        switch ( eMode )
        {
        case MB_REG_READ:
            while( usNRegs > 0 )
            {
                *PRT++ = __REV16(usRegHoldingBuf[iRegIndex++]);

// 				*pucRegBuffer++ = ( unsigned char )( usRegHoldingBuf[iRegIndex] >> 8 );
//              *pucRegBuffer++ = ( unsigned char )( usRegHoldingBuf[iRegIndex] & 0xFF );
//				iRegIndex++;
                usNRegs--;
            }
            break;

        case MB_REG_WRITE:
            while( usNRegs > 0 )
            {
                usRegHoldingBuf[iRegIndex++] = __REV16(*PRT++);

//				usRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
//              usRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
//              iRegIndex++;
                usNRegs--;
            }
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }
    return eStatus;
}



eMBErrorCode
eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils, eMBRegisterMode eMode )
{
    ( void )pucRegBuffer;
    ( void )usAddress;
    ( void )usNCoils;
    ( void )eMode;
    return MB_ENOREG;
}


eMBErrorCode
eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete )
{
    ( void )pucRegBuffer;
    ( void )usAddress;
    ( void )usNDiscrete;
    return MB_ENOREG;
}
