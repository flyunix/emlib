/*
 * File:    cstr.c
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
#include <stdlib.h>
#include <string.h>

#include "cstr.h"
#include "assert.h"

int32
cstr_init_heap(cstr *cs, uint32 buff_len)
{
    return_val_if_fail(cs && (buff_len > 0), EMBED_INVALID_PARAMS);
    
    cs->buff = calloc(1, buff_len);

    if(cs->buff == NULL)  {
        EM_LOG(EM_LOG_ERROR, "calloc failed.");
        return CSTR_ERR_NOMEM;  
    }

    cs->alen = buff_len;
    cs->ulen = 0;
    cs->mlen = buff_len;
    cs->rpos = 0;

    return EMBED_SUCC;
}

int32
cstr_heap_free(cstr *cs)
{
    return_val_if_fail(cs, EMBED_INVALID_PARAMS);

    if(cs->buff) {
        free(cs->buff);
        cs->buff = NULL;
    }

    return EMBED_SUCC;
}

int32 
cstr_init_stack(cstr *cs, int8* buff, uint32 buff_len)
{
    return_val_if_fail(cs && buff && (buff_len > 0), EMBED_INVALID_PARAMS); 

    cs->buff = buff;
    memset(cs->buff, 0, buff_len);

    cs->alen = buff_len;
    cs->ulen = 0;
    cs->mlen = buff_len;
    cs->rpos = 0;

    return EMBED_SUCC;
}

int32
cstr_init_reset(cstr *cs)
{
    return_val_if_fail(cs, EMBED_INVALID_PARAMS);

    memset(cs->buff, 0, cs->alen);

    cs->ulen = 0;
    cs->mlen = 0;
    cs->rpos = 0;

    return EMBED_SUCC;
}

int8* 
cstr_get_buff(cstr *cs)
{
    return_val_if_fail(cs, NULL);

    return cs->buff;
}

uint32
cstr_get_ulen(cstr *cs)
{
    return_val_if_fail(cs, EMBED_INVALID_PARAMS);

    return cs->ulen;
}

uint32
cstr_get_alen(cstr *cs)
{
    return_val_if_fail(cs, EMBED_INVALID_PARAMS);

    return cs->alen;
}

uint32
cstr_get_mlen(cstr *cs)
{
    return_val_if_fail(cs, EMBED_INVALID_PARAMS);

    return cs->mlen;
}

static int32 
_cstr_put_data(cstr *cs, int8 *data, uint32 data_len)
{
    if(cs->mlen < data_len) {
        return CSTR_ERR_NOMEM;  
    }
    
    memcpy(cs->buff + cs->ulen, data, data_len);

    cs->ulen += data_len;
    cs->mlen = cs->alen - cs->ulen;

    return EMBED_SUCC;
}

static int32
_cstr_get_data(cstr *cs, int8* data, uint32 data_len)
{
    if(cs->ulen - cs->rpos < data_len) {
        return CSTR_ERR_OVERFLOW;
    }   

    memcpy(data, cs->buff + cs->rpos, data_len);   

    cs->rpos += data_len;

    return EMBED_SUCC;
}

int32
cstr_put_int8(cstr *cs, int8 data)
{
    return_val_if_fail(cs, EMBED_INVALID_PARAMS);
    
    return _cstr_put_data(cs, (int8*)&data, sizeof(data));
}

int32
cstr_put_int16(cstr *cs, int16 data)
{
    return_val_if_fail(cs, EMBED_INVALID_PARAMS);

    return _cstr_put_data(cs, (int8*)&data, sizeof(data));
}

int32
cstr_put_int32(cstr *cs, int32 data)
{
    return_val_if_fail(cs, EMBED_INVALID_PARAMS);

    return _cstr_put_data(cs, (int8*)&data, sizeof(data));
}

int32
cstr_put_blob(cstr *cs, int8 *data, uint32 data_len)
{
    return_val_if_fail(cs, EMBED_INVALID_PARAMS);
    return _cstr_put_data(cs, data, data_len);
}

int32
cstr_put_str(cstr *cs, int8* str, uint32 str_len)
{
    return_val_if_fail(cs && str, EMBED_INVALID_PARAMS);

    if(cs->mlen < str_len ) {
        return CSTR_ERR_NOMEM;
    }

    strncpy(cs->buff + cs->ulen, str, str_len);
    cs->ulen += str_len;

    return EMBED_SUCC;
}

int32
cstr_get_int8(cstr *cs, int8 *data)
{
    return_val_if_fail(cs, EMBED_INVALID_PARAMS);
    return _cstr_get_data(cs, data, sizeof(int8));
}

int32
cstr_get_uint8(cstr *cs, uint8 *data)
{
    return_val_if_fail(cs, EMBED_INVALID_PARAMS);
    return _cstr_get_data(cs, (int8*)data, sizeof(uint8));
}

int32
cstr_get_int16(cstr *cs, int16 *data)
{
    return_val_if_fail(cs, EMBED_INVALID_PARAMS);

    return _cstr_get_data(cs, (int8*)data, sizeof(int16));
}

int32
cstr_get_uint16(cstr *cs, uint16 *data)
{
    return_val_if_fail(cs, EMBED_INVALID_PARAMS);

    return _cstr_get_data(cs, (int8*)data, sizeof(uint16));
}

int32
cstr_get_int32(cstr *cs, int32 *data)
{
    return_val_if_fail(cs, EMBED_INVALID_PARAMS);

    return _cstr_get_data(cs, (int8*)data, sizeof(int32));
}

int32
cstr_get_uint32(cstr *cs, uint32 *data)
{
    return_val_if_fail(cs, EMBED_INVALID_PARAMS);

    return _cstr_get_data(cs, (int8*)data, sizeof(uint32));
}

int32
cstr_get_blob(cstr *cs, int8 *data, uint32 data_len)
{
    return_val_if_fail(cs && data, EMBED_INVALID_PARAMS);

    return _cstr_get_data(cs, data, data_len);
}

int32
cstr_get_str(cstr *cs, int8 *str, uint32 str_len)
{
    return_val_if_fail(cs && str, EMBED_INVALID_PARAMS);   

    if((cs->ulen - cs->rpos) < str_len) {
        return CSTR_ERR_OVERFLOW;
    }

    strncpy(str, cs->buff + cs->rpos, str_len);

    cs->rpos += str_len;

    return EMBED_SUCC;
}
