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
u8 REG_INPUT_NREGS=32,REG_HOLDING_NREGS=24;
u8 usRegInputStart=1,usRegHoldingStart=1;


eMBErrorCode
eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{
	eMBErrorCode    eStatus = MB_ENOERR;
	int             iRegIndex;
    uint16_t i=0;

    REG_INPUT_NREGS=(DRYING_CHANNELS_NUM+1)*2;

    for(i=0;i<DRYING_CHANNELS_NUM;i++)
    {
    	((float*)usRegInputBuf)[i] = uks_channels.drying_channel_list[i].temperature;
    }

    ((float*)usRegInputBuf)[DRYING_CHANNELS_NUM] = uks_channels.heater_temperature;

    if( ( usAddress >= REG_INPUT_START )&& ( usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS ) )
    {
        iRegIndex = ( int )( usAddress - usRegInputStart );
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


#define REG_INDEX_HEATER_TEMP_1	14
#define REG_INDEX_HEATER_TEMP_2	16
#define REG_INDEX_P_FACTOR		18
#define REG_INDEX_I_FACTOR		20
#define REG_INDEX_D_FACTOR		22

eMBErrorCode
eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    int             iRegIndex;
	u16 *PRT=(u16*)pucRegBuffer;
	uint16_t i=0;

    if( ( usAddress >= REG_HOLDING_START ) && ( usAddress + usNRegs <= REG_HOLDING_START + REG_HOLDING_NREGS ) )
    {
        iRegIndex = ( int )( usAddress - usRegHoldingStart );
        REG_HOLDING_NREGS=(DRYING_CHANNELS_NUM+5)*2;
        switch ( eMode )
        {
			case MB_REG_READ:
			{

			    for(i=0;i<DRYING_CHANNELS_NUM;i++)
			    {
			    	((float*)usRegHoldingBuf)[i] = uks_channels.uks_params.end_drying_temperature[i];
			    }

			    ((float*)usRegHoldingBuf)[DRYING_CHANNELS_NUM] = uks_channels.uks_params.heater_temperature_1;
			    ((float*)usRegHoldingBuf)[DRYING_CHANNELS_NUM+1] = uks_channels.uks_params.heater_temperature_2;
			    ((float*)usRegHoldingBuf)[DRYING_CHANNELS_NUM+2] = uks_channels.uks_params.p_factor;
			    ((float*)usRegHoldingBuf)[DRYING_CHANNELS_NUM+3] = uks_channels.uks_params.i_factor;
			    ((float*)usRegHoldingBuf)[DRYING_CHANNELS_NUM+4] = uks_channels.uks_params.d_factor;

				while( usNRegs > 0 )
				{
	 				*pucRegBuffer++ = ( unsigned char )( usRegHoldingBuf[iRegIndex] >> 8 );
	                *pucRegBuffer++ = ( unsigned char )( usRegHoldingBuf[iRegIndex] & 0xFF );
					iRegIndex++;

					usNRegs--;
				}
			}
			break;

			case MB_REG_WRITE:
			{
				while( usNRegs > 0 )
				{
					if((iRegIndex>=0)&&(iRegIndex<(DRYING_CHANNELS_NUM*2)))
					{
						//uks_channels.uks_params.end_drying_temperature[(iRegIndex>>1)]=*((float*)pucRegBuffer);

						((uint8_t*)(&(uks_channels.uks_params.end_drying_temperature[(iRegIndex>>1)])))[1]=*pucRegBuffer++;
						((uint8_t*)(&(uks_channels.uks_params.end_drying_temperature[(iRegIndex>>1)])))[0]=*pucRegBuffer++;
						((uint8_t*)(&(uks_channels.uks_params.end_drying_temperature[(iRegIndex>>1)])))[3]=*pucRegBuffer++;
						((uint8_t*)(&(uks_channels.uks_params.end_drying_temperature[(iRegIndex>>1)])))[2]=*pucRegBuffer++;

						iRegIndex+=2;
						usNRegs-=2;
						//pucRegBuffer+=4;
					}
					else
					{
						switch(iRegIndex)
						{
							case REG_INDEX_HEATER_TEMP_1:
							{
								//uks_channels.uks_params.heater_temperature_1=30;//*((float*)pucRegBuffer);
								((uint8_t*)(&uks_channels.uks_params.heater_temperature_1))[1]=*pucRegBuffer++;
								((uint8_t*)(&uks_channels.uks_params.heater_temperature_1))[0]=*pucRegBuffer++;
								((uint8_t*)(&uks_channels.uks_params.heater_temperature_1))[3]=*pucRegBuffer++;
								((uint8_t*)(&uks_channels.uks_params.heater_temperature_1))[2]=*pucRegBuffer++;
								iRegIndex+=2;
								usNRegs-=2;
//								pucRegBuffer+=4;
							}
							break;

							case REG_INDEX_HEATER_TEMP_2:
							{
								//uks_channels.uks_params.heater_temperature_2=*((float*)pucRegBuffer);
								((uint8_t*)(&uks_channels.uks_params.heater_temperature_2))[1]=*pucRegBuffer++;
								((uint8_t*)(&uks_channels.uks_params.heater_temperature_2))[0]=*pucRegBuffer++;
								((uint8_t*)(&uks_channels.uks_params.heater_temperature_2))[3]=*pucRegBuffer++;
								((uint8_t*)(&uks_channels.uks_params.heater_temperature_2))[2]=*pucRegBuffer++;
								iRegIndex+=2;
								usNRegs-=2;
							//	pucRegBuffer+=4;
							}
							break;

							case REG_INDEX_P_FACTOR:
							{
								//uks_channels.uks_params.p_factor=*((float*)pucRegBuffer);
								((uint8_t*)(&uks_channels.uks_params.p_factor))[1]=*pucRegBuffer++;
								((uint8_t*)(&uks_channels.uks_params.p_factor))[0]=*pucRegBuffer++;
								((uint8_t*)(&uks_channels.uks_params.p_factor))[3]=*pucRegBuffer++;
								((uint8_t*)(&uks_channels.uks_params.p_factor))[2]=*pucRegBuffer++;
								iRegIndex+=2;
								usNRegs-=2;
							//	pucRegBuffer+=4;
							}
							break;

							case REG_INDEX_I_FACTOR:
							{
								//uks_channels.uks_params.i_factor=*((float*)pucRegBuffer);
								((uint8_t*)(&uks_channels.uks_params.i_factor))[1]=*pucRegBuffer++;
								((uint8_t*)(&uks_channels.uks_params.i_factor))[0]=*pucRegBuffer++;
								((uint8_t*)(&uks_channels.uks_params.i_factor))[3]=*pucRegBuffer++;
								((uint8_t*)(&uks_channels.uks_params.i_factor))[2]=*pucRegBuffer++;
								iRegIndex+=2;
								usNRegs-=2;
								//pucRegBuffer+=4;
							}
							break;

							case REG_INDEX_D_FACTOR:
							{
								//uks_channels.uks_params.d_factor=*((float*)pucRegBuffer);
								((uint8_t*)(&uks_channels.uks_params.d_factor))[1]=*pucRegBuffer++;
								((uint8_t*)(&uks_channels.uks_params.d_factor))[0]=*pucRegBuffer++;
								((uint8_t*)(&uks_channels.uks_params.d_factor))[3]=*pucRegBuffer++;
								((uint8_t*)(&uks_channels.uks_params.d_factor))[2]=*pucRegBuffer++;
								iRegIndex+=2;
								usNRegs-=2;
								//pucRegBuffer+=4;
							}
							break;

							default:
							{

							}
							break;
						}
					}
				}
			}
			break;
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
