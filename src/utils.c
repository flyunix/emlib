
/*
 *   Copyright (C) 2020 ZPD Ltd. All rights reserved.
 *   
 *   FileName:utils.c
 *
 *   Author:LiuHongliang
 *
 *   Create:2020年04月26日
 *
 *   Brief:
 *
 */
#include "emlib.h"

static uint8 bcd2ascii[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

static uint8 ascii2bcd1[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
static uint8 ascii2bcd2[6]  = {0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
static uint8 ascii2bcd3[6]  = {0X0a, 0X0b, 0X0c, 0X0d, 0X0e, 0X0f};

/*
 * @breif: Convert Ascii code To BCD Code 
 *
 * @func: asc2bcd
 *
 * @param:
 *
 * @return:
 *
 */
    uint32 
asc2bcd(uint8 *bcd, const int8 *asc, int32 len)
{
    uint8 c = 0;
    uint8 index = 0;
    uint8 i = 0; 

    len >>= 1;    

    for(; i < len; i++) {
        //first BCD
        if(isupper(*asc)){
            index = *asc - 'A'; 
            c  = ascii2bcd2[index] << 4;
        } else if(isdigit(*asc)) {
            index = *asc - '0';
            c  = ascii2bcd1[index] << 4;
        }else if(islower(*asc)) {
            index = *asc - 'a'; 
            c  = ascii2bcd3[index] << 4;
        }
        asc++;

        //second BCD
        if(isupper(*asc)) {
            index = *asc - 'A'; 
            c  |= ascii2bcd2[index];
        } else if(isdigit(*asc)) {
            index = *asc - '0';
            c  |= ascii2bcd1[index];
        }else if(islower(*asc)) {
            index = *asc - 'a'; 
            c  |= ascii2bcd3[index];
        }
        asc++;

        *bcd++ = c;
    }

    return 0;
}

/*
 * @breif: Convert BCD Code To Ascii Code
 *
 * @func:  bcd2asc
 *
 * @param:
 *
 * @return:
 *
 */
    int8* 
bcd2asc (int8 *asc, const uint8 *bcd, uint32 len)
{
    uint8 c = 0;
    uint8 i;  
    int8* ptr = asc;

    for(i = 0; i < len; i++) {
        //first BCD
        c = *bcd >> 4;
        *ptr++ = bcd2ascii[c];

        //second
        c = *bcd & 0x0f;
        *ptr++ = bcd2ascii[c]; 
        bcd++;
    }

    return asc;
}

#define PRESET_VALUE 0xFFFF
#define POLYNOMIAL  0x8408

/*
 * @breif: CRC16
 *
 * @func:  crc16
 *
 * @param:
 *
 * @return:
 *
 */
uint32 crc16(int8 const  *pucY, uint8 ucX)
{
    uint8  ucI,ucJ;
    uint16 uiCrcValue = PRESET_VALUE;

    for(ucI = 0; ucI < ucX; ucI++)
    {
        uiCrcValue = uiCrcValue ^ *(pucY + ucI);
        for(ucJ = 0; ucJ < 8; ucJ++)
        {
            if(uiCrcValue & 0x0001)
            {
                uiCrcValue = (uiCrcValue >> 1) ^ POLYNOMIAL;
            }
            else
            {
                uiCrcValue = (uiCrcValue >> 1);
            }
        }
    }
    return uiCrcValue;
}

