/** @file osc.c */

#ifndef _BSD_SOURCE
    #define _BSD_SOURCE
#endif // _BSD_SOURCE

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <endian.h>
#include "osc.h"

int32_t osc_unpack_int32(int32_t value)
{
    int32_t h_value = be32toh(value);

return h_value;
}

float osc_unpack_float(float value)
{
    float h_value = 0;
    *(uint32_t *)&h_value = be32toh(*(uint32_t *)&value);

return h_value;
}

size_t osc_message_serialized_length(const struct osc_message* msg)
{
    size_t cur_msg_length = 0;
    unsigned char* uchar_ptr = (unsigned char*)msg->raw_data;
    for(int i = 0, j = 0; i < 4 && j <= 24; i++, j += 8) {
        cur_msg_length |= ((unsigned int)uchar_ptr[i] << j);
    }
   cur_msg_length = osc_unpack_int32((int32_t)cur_msg_length);

return cur_msg_length;
}

size_t osc_bundle_serialized_length(const struct osc_bundle* bundle)
{
    size_t cur_bd_length = 0;
    unsigned char* uchar_ptr = (unsigned char*)bundle->raw_data;
    for(int i = 0, j = 0; i < 4 && j <= 24; i++, j += 8) {
        cur_bd_length |= ((unsigned int)uchar_ptr[i] << j);
    }
   cur_bd_length = osc_unpack_int32(cur_bd_length);

return cur_bd_length;
}

size_t osc_blob_data_size(const osc_blob b)
{
    size_t cur_blob_length = 0;
    unsigned char* uchar_ptr = (unsigned char*)b;
    for(int i = 0, j = 0; i < 4 && j <= 24; i++, j += 8) {
        cur_blob_length |= ((unsigned int)uchar_ptr[i] << j);
    }
    cur_blob_length = osc_unpack_int32((int32_t)cur_blob_length);

return cur_blob_length;
}

/**
 * Actualizes the length bytes of the osc_message instance
 *
 * @param   msg                pointer to the osc_message structure
 * @param   new_msg_length     the new length value
 */
static void actualize_length(struct osc_message* msg, unsigned int new_msg_length)
{
    uint32_t be_value = (uint32_t)htobe32(new_msg_length);
    unsigned char* uchar_ptr = (unsigned char*)msg->raw_data;
    for(int i = 0; i < 4; i++) {
        uchar_ptr[i] = be_value & 0xff;
        be_value >>= 8;
    }
}

/**
 * Actualizes the typetag bytes of the osc_message instance
 *
 * @param   msg     pointer to the osc_message structure
 * @param   tag     tag of the argument being added
 * @return          returns 0 on success or 1 if memory reallocation failed
 */
static int actualize_typetag(struct osc_message* msg, char tag)
{
    unsigned int cur_tg_length = strlen(msg->typetag);
    unsigned char* uchar_ptr = (unsigned char*)msg->raw_data;
    if(((cur_tg_length % 4) % 3 == 0) && ((cur_tg_length % 4) != 0)) {
        unsigned int cur_msg_length = osc_message_serialized_length(msg);
        unsigned int new_mem_size = cur_msg_length + 8;
        unsigned int new_msg_length = cur_msg_length + 4;
        unsigned int addr_length = msg->typetag - msg->address;
        unsigned int arg_size = cur_msg_length - addr_length - cur_tg_length - (4 - (cur_tg_length % 4));
        unsigned char* memory_alloc = (unsigned char*)realloc(uchar_ptr, new_mem_size);
            if(memory_alloc == NULL) {
                return 1;
            }
            else {
              uchar_ptr = memory_alloc;
              msg->raw_data = (void*)uchar_ptr;
              msg->address = (char*)msg->raw_data + sizeof(int32_t);
              msg->typetag = msg->address + strlen(msg->address) + (4 - (strlen(msg->address) % 4));
              char* arg_start = msg->typetag + strlen(msg->typetag) + 1;
              memmove(arg_start + 4, arg_start, arg_size);
              memset(msg->typetag + strlen(msg->typetag) + 1, 0, 4);
              memset(msg->typetag + strlen(msg->typetag), tag, 1);
              actualize_length(msg, new_msg_length);
            }
    }
    else {
       memset(msg->typetag + strlen(msg->typetag), tag, 1);
    }
return 0;
}

int osc_message_new(struct osc_message* msg)
{
    unsigned char* mem_alloc = (unsigned char*)realloc(NULL, 12);
    //check if memory allocation was successful
    if(mem_alloc == NULL) {
        return 1;
    }
    else {
        unsigned char* uchar_ptr = mem_alloc;
        msg->raw_data = (void*)uchar_ptr;
        memset(uchar_ptr, 0, 3);
        memset(uchar_ptr + 3, 8, 1);
        memset(uchar_ptr + 4, '\0', 4);
        memset(uchar_ptr + 8, ',' ,1);
        memset(uchar_ptr + 9, '\0', 3);
        msg->address = (char*)msg->raw_data + sizeof(int32_t);
        msg->typetag = msg->address + strlen(msg->address) + (4 - (strlen(msg->address) % 4));
    }
return 0;
}

void osc_message_destroy(struct osc_message* msg)
{
    free(msg->raw_data);
    msg->raw_data = NULL;
    msg->address = NULL;
    msg->typetag = NULL;

}

osc_blob osc_blob_new(size_t length)
{
    unsigned int add_bytes = 0;
    if(length % 4 != 0) {
        add_bytes = 4 - (length % 4);
    }
    osc_blob blob = (void*)realloc(NULL, (length + 4 + add_bytes) * sizeof(char));
    if(blob == NULL) {
        return blob;
    }
    uint32_t be_length = (uint32_t)htobe32(length);
    unsigned char* uchar_ptr = (unsigned char*)blob;
    for(int i = 0; i < 4; i++) {
        uchar_ptr[i] = be_length & 0xff;
        be_length >>= 8;
    }
    memset(uchar_ptr + 4, 0, length + add_bytes);

return blob;
}

void osc_blob_destroy(osc_blob b)
{
    free(b);
}

void* osc_blob_data_ptr(const osc_blob b)
{
    unsigned char* data_space = (unsigned char*)b + 4;
    void* void_ptr_data = (void*)data_space;

return void_ptr_data;
}

int osc_message_set_address (struct osc_message* msg, const char* address)
{
    unsigned int cur_addr_space_size = msg->typetag - msg->address;
    unsigned int new_addr_space_size = strlen(address) + (4 - (strlen(address) % 4));
    if(cur_addr_space_size == new_addr_space_size) {
        strcpy(msg->address, address);
        return 0;
    }

    else {
        unsigned char* uchar_ptr = (unsigned char*)msg->raw_data;
        unsigned int add_mem_size = new_addr_space_size - cur_addr_space_size;
        unsigned int cur_msg_length = osc_message_serialized_length(msg);
        unsigned int new_mem_size = cur_msg_length + 4 + add_mem_size;
        unsigned int num_bytes_to_copy = cur_msg_length - cur_addr_space_size;
        unsigned int new_msg_length = cur_msg_length + add_mem_size;
        unsigned char* memory_alloc = (unsigned char*)realloc(uchar_ptr, new_mem_size);
        if(memory_alloc == NULL) {
                return 1;
        }
        else {
             uchar_ptr = memory_alloc;
             msg->raw_data = (void*)uchar_ptr;
             msg->address = (char*)msg->raw_data + sizeof(int32_t);
             msg->typetag = msg->address + strlen(msg->address) + (4 - (strlen(msg->address) % 4));
             memmove(msg->typetag + add_mem_size, msg->typetag, num_bytes_to_copy);
             strcpy(msg->address, address);
             memset(msg->address + strlen(address), 0, (4 - (strlen(address) % 4)));
        }
        actualize_length(msg, new_msg_length);
        msg->typetag += add_mem_size;
    }
return 0;
}

/**
 * Adds an argument of the desired type to the osc_message instance
 *
 * @param   msg         pointer to the osc_message structure
 * @param   tag         tag of the argument being added
 * @param   argument    data of the argument to be added
 * @return              returns 0 on success or 1 if memory reallocation failed
 */
static int add_argument(struct osc_message* msg, char tag, union osc_msg_argument* argument)
{
    unsigned char* uchar_ptr = (unsigned char*)msg->raw_data;
    char* p_new_data = NULL;
    const char* bytes = &argument->s;
    unsigned int cur_msg_length = osc_message_serialized_length(msg);
    unsigned int new_mem_size = 0;
    unsigned int byte_count = 0;
    unsigned int blob_add_bytes;
    switch(tag) {
        case 'i': new_mem_size = cur_msg_length + 4 + sizeof(int32_t);
                  byte_count = sizeof(int32_t); break;
        case 'f': new_mem_size = cur_msg_length + 4 + sizeof(float);
                  byte_count = sizeof(float); break;
        case 's': new_mem_size = cur_msg_length + 4 + strlen(bytes) + (4 - (strlen(bytes) % 4));
                  byte_count = strlen(bytes) + 1; break;
        case 't': new_mem_size = cur_msg_length + 4 + sizeof(struct osc_timetag);
                  byte_count = sizeof(struct osc_timetag); break;
        case 'b': byte_count = osc_blob_data_size((osc_blob)bytes);
                  if(byte_count % 4 != 0) {
                      blob_add_bytes = 4 - (byte_count % 4);
                  }
                  byte_count = byte_count + 4 + blob_add_bytes;
                  new_mem_size = cur_msg_length + 4 + byte_count; break;
    }
    unsigned int new_msg_length = new_mem_size - 4;
    unsigned char* memory_alloc = (unsigned char*)realloc(uchar_ptr, new_mem_size);
    if(memory_alloc == NULL) {
       return 1;
    }
    else {
        uchar_ptr = memory_alloc;
        msg->raw_data = (void*)uchar_ptr;
        msg->address = (char*)msg->raw_data + sizeof(int32_t);
            msg->typetag = msg->address + strlen(msg->address) + (4 - (strlen(msg->address) % 4));
        p_new_data = (char*)uchar_ptr + 4 + cur_msg_length;
        if(tag == 's') {
            memset(p_new_data, 0, strlen(bytes) + (4 - (strlen(bytes) % 4)));
        }
        for(unsigned int i = 0; i < byte_count; i++) {
            p_new_data[i] = bytes[i];
        }
        actualize_length(msg, new_msg_length);
        switch(tag) {
            case 'i': actualize_typetag(msg, OSC_TT_INT); break;
            case 'f': actualize_typetag(msg, OSC_TT_FLOAT); break;
            case 's': actualize_typetag(msg, OSC_TT_STRING); break;
            case 't': actualize_typetag(msg, OSC_TT_TIMETAG); break;
            case 'b': actualize_typetag(msg, OSC_TT_BLOB); break;
        }
    }

return 0;
}

int osc_message_add_timetag(struct osc_message* msg, struct osc_timetag tag)
{
   struct osc_timetag be_tag;
   be_tag.sec = htobe32(tag.sec);
   be_tag.frac = htobe32(tag.frac);
   union osc_msg_argument* argument = (union osc_msg_argument*)&be_tag;
   int return_value = add_argument(msg, OSC_TT_TIMETAG, argument);
   if(return_value == 1) {
       return 1;
   }

return 0;
}
int osc_message_add_string(struct osc_message* msg, const char* data)
{
    union osc_msg_argument* argument = (union osc_msg_argument*)data;
    int return_value = add_argument(msg, OSC_TT_STRING, argument);
    if(return_value == 1) {
        return 1;
    }

return 0;
}

int osc_message_add_float(struct osc_message* msg, float data)
{
    uint32_t be_value = htobe32(*(uint32_t*)(&data));
    union osc_msg_argument* argument = (union osc_msg_argument*)&be_value;
    int return_value = add_argument(msg, OSC_TT_FLOAT, argument);
    if(return_value == 1) {
        return 1;
    }

return 0;
}

int osc_message_add_blob(struct osc_message* msg, const osc_blob b)
{
    union osc_msg_argument* argument = (union osc_msg_argument*)b;
    int return_value = add_argument(msg, OSC_TT_BLOB, argument);
    if(return_value == 1) {
        return 1;
    }

return 0;
}

int osc_message_add_int32(struct osc_message* msg, int32_t data)
{
    int32_t be_value = htobe32(data);
    union osc_msg_argument* argument = (union osc_msg_argument*)&be_value;
    int return_value = add_argument(msg, OSC_TT_INT, argument);
    if(return_value == 1) {
        return 1;
    }
return 0;
}

size_t osc_message_argc(const struct osc_message* msg)
{
    size_t argc = strlen(msg->typetag) - 1;

return argc;
}

const union osc_msg_argument* osc_message_arg(const struct osc_message* msg, size_t arg_index)
{
    char* p_arguments = msg->typetag + strlen(msg->typetag) + (4 - (strlen(msg->typetag) % 4));
    unsigned int blob_size = 0;
    unsigned int add_bytes = 0;
    if(msg->typetag[arg_index + 1] == '\0') {
        return NULL;
    }
    for(unsigned int i = 1; i < arg_index + 1; i++) {
        switch(msg->typetag[i]) {
            case OSC_TT_INT:     p_arguments += 4; break;
            case OSC_TT_FLOAT:   p_arguments += 4; break;
            case OSC_TT_STRING:  p_arguments += strlen(p_arguments) + (4 - (strlen(p_arguments) % 4)); break;
            case OSC_TT_TIMETAG: p_arguments += 8; break;
            case OSC_TT_BLOB:    blob_size = osc_blob_data_size((osc_blob*)p_arguments);
                                 if(blob_size % 4 != 0) {
                                 add_bytes = 4 - (blob_size % 4);
                                 }
                                 p_arguments += 4 + blob_size + add_bytes; break;
        }
    }
return (const union osc_msg_argument*)p_arguments;
}

int osc_bundle_new(struct osc_bundle* bnd)
{
    char* mem_alloc = (char*)realloc(NULL, 20 * sizeof(char));
    if(mem_alloc == NULL) {
        return 1;
    }
    else {
        bnd->raw_data = (void*)mem_alloc;
        char* timetag = (char*)bnd->raw_data + 12;
        bnd->timetag = (struct osc_timetag*)timetag;
        struct osc_timetag tag;
        OSC_TIMETAG_IMMEDIATE(&tag);
        osc_bundle_set_timetag(bnd, tag);
        char* char_ptr = (char*)bnd->raw_data;
        memset(char_ptr, 0, 3);
        memset(char_ptr + 3, 16, 1);
        strcpy(char_ptr + 4, "#bundle");
  }
return 0;
}
void osc_bundle_destroy(struct osc_bundle* bn)
{
    free(bn->raw_data);
    bn->raw_data = NULL;
    bn->timetag = NULL;
}
void osc_bundle_set_timetag(struct osc_bundle* bundle, struct osc_timetag timetag)
{
    timetag.sec = (int32_t)htobe32(timetag.sec);
    timetag.frac = (int32_t)htobe32(timetag.frac);
    char* bnd_tag = (char*)bundle->raw_data + 12;
    union osc_msg_argument* tag = (union osc_msg_argument*)&timetag;
    const char* bytes = &tag->s;
    for(unsigned int i = 0; i < 8; i++) {
        bnd_tag[i] = bytes[i];
    }
}

int osc_bundle_add_message(struct osc_bundle* bundle, const struct osc_message* msg)
{
    unsigned int cur_bd_length = osc_bundle_serialized_length(bundle);
    unsigned int cur_msg_length = osc_message_serialized_length(msg);
    unsigned char* uchar_ptr_bd = (unsigned char*)bundle->raw_data;
    unsigned char* uchar_ptr_msg = (unsigned char*)msg->raw_data;
    unsigned int new_mem_size = cur_bd_length + cur_msg_length + 8;
    unsigned int new_bd_length = new_mem_size - 4;
    unsigned char* memory_alloc = (unsigned char*)realloc(uchar_ptr_bd, new_mem_size);
    if(memory_alloc == NULL) {
            return 1;
    }
    else {
        uchar_ptr_bd = memory_alloc;
        bundle->raw_data = (void*)uchar_ptr_bd;
        char* timetag = (char*)bundle->raw_data + 12;
        bundle->timetag = (struct osc_timetag*)timetag;
        unsigned char* p_new_data = uchar_ptr_bd + 4 + cur_bd_length;
        memmove(p_new_data,uchar_ptr_msg, cur_msg_length + 4);
        uint32_t be_value = (uint32_t)htobe32(new_bd_length);
        for(int i = 0; i < 4; i++) {
            uchar_ptr_bd[i] = (unsigned char)be_value & 0xff;
            be_value >>= 8;
        }
    }
return 0;
}
struct osc_message osc_bundle_next_message(const struct osc_bundle * bundle, struct osc_message prev)
{
    struct osc_message next_msg;
    OSC_MESSAGE_NULL(&next_msg);
    unsigned int cur_bd_length = osc_bundle_serialized_length(bundle);
    unsigned char* next_msg_uchar = (unsigned char*)bundle->raw_data + 20;
    unsigned char* uchar_prev = (unsigned char*)prev.raw_data;
    unsigned char* first_byte_after = (unsigned char*)bundle->raw_data + 4 + cur_bd_length;
    if(next_msg_uchar == first_byte_after) {
           return next_msg;
    }
    if(prev.raw_data == NULL) {
           next_msg.raw_data = (void*)next_msg_uchar;
           next_msg.address = (char*)next_msg.raw_data + sizeof(int32_t);
           next_msg.typetag = next_msg.address + strlen(next_msg.address) + (4 - (strlen(next_msg.address) % 4));
           return next_msg;
    }
    else {
        unsigned int prev_msg_length = osc_message_serialized_length(&prev);
        if(uchar_prev + prev_msg_length + 4 < first_byte_after) {
            next_msg_uchar = uchar_prev + prev_msg_length + 4;
            next_msg.raw_data = (void*)next_msg_uchar;
            next_msg.address = (char*)next_msg_uchar + 4;
            next_msg.typetag = next_msg.address + strlen(next_msg.address) + (4 - (strlen(next_msg.address) % 4));
        }
        else {
            return next_msg;
        }
   }
return next_msg;
}
