/*
 * File:    cstr.h
 * Author:  Liu HongLiang<lhl_nciae@sina.cn>
 * Brief:   C String.
 *
 * Copyright (c) Liu HongLiang
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

/*
 * History:
 * ================================================================
 * 2018-12-25 Liu HongLiang<lhl_nciae@sina.cn> created.
 *
 */
#include "types.h"

/*Error defines*/
#define CSTR_ERR_BASE   0x1000

#define CSTR_ERR_NOMEM    (CSTR_ERR_BASE + 1)
#define CSTR_ERR_OVERFLOW (CSTR_ERR_BASE + 2)

typedef struct {
    int8*  buff;
    uint32 alen;
    uint32 ulen;
    uint32 mlen;
    uint32 rpos;
}cstr;

int32
cstr_init_heap(cstr *cs, uint32 buff_len);

int32 
cstr_init_stack(cstr *cs, int8* buff, uint32 buff_len);

int32
cstr_init_reset(cstr *cs);

int32
cstr_heap_free(cstr *cs);

int8* 
cstr_get_buff(cstr *cs);

uint32
cstr_get_ulen(cstr *cs);

uint32
cstr_get_alen(cstr *cs);

uint32
cstr_get_mlen(cstr *cs);

int32
cstr_put_int8(cstr *cs, int8 data);

int32
cstr_put_int16(cstr *cs, int16 data);

int32
cstr_put_int32(cstr *cs, int32 data);

int32
cstr_put_blob(cstr *cs, int8 *data, uint32 data_len);


int32
cstr_get_int8(cstr *cs, int8 *data);

int32
cstr_get_uint8(cstr *cs, uint8 *data);

int32
cstr_get_int16(cstr *cs, int16 *data);

int32
cstr_get_uint16(cstr *cs, uint16 *data);

int32
cstr_get_int32(cstr *cs, int32 *data);

int32
cstr_get_uint32(cstr *cs, uint32 *data);

int32
cstr_get_blob(cstr *cs, int8 *data, uint32 data_len);

