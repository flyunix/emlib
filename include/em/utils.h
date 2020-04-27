
/*
 *   Copyright (C) 2020 ZPD Ltd. All rights reserved.
 *   
 *   FileName:utils.h
 *
 *   Author:LiuHongliang
 *
 *   Create:2020年04月26日
 *
 *   Brief:
 *
 */
#ifndef _UTILS_H_
#define _UTILS_H_

#include "em/types.h"

    uint32 
asc2bcd(uint8 *bcd, const int8 *asc, int32 len);

    int8* 
bcd2asc (int8 *asc, const uint8 *bcd, uint32 len);

uint32 crc16(int8 const  *pucY, uint8 ucX);

#endif //UTILS_H_
