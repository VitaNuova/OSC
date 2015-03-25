/** @file osc.h */

#ifndef OSC_H
#define OSC_H

#include <stdint.h>
#include <stdlib.h>

#define OSC_TT_INT 'i'
#define OSC_TT_STRING 's'
#define OSC_TT_FLOAT 'f'
#define OSC_TT_TIMETAG 't'
#define OSC_TT_BLOB 'b'
#define OSC_TYPETAG(...)  { ',', __VA_ARGS__, '\0'}
#define OSC_TIMETAG_IMMEDIATE(timetag_instance) \
    do { \
    (*timetag_instance).sec = 0; \
    (*timetag_instance).frac = 1; \
    } while (0)
#define OSC_TIMETAG_NULL(timetag_instance) \
    do { \
    (*timetag_instance).sec = 0; \
    (*timetag_instance).frac = 0; \
    } while (0)
#define OSC_MESSAGE_NULL(msg) \
    do { \
    (*msg).address = NULL; \
    (*msg).typetag = NULL; \
    (*msg).raw_data = NULL; \
    } while (0)
#define OSC_BUNDLE_NULL(bnd) \
    do {\
    (*bnd).timetag = NULL; \
    (*bnd).raw_data = NULL; \
    } while (0)
typedef void* osc_blob;

/**
 * Structure representing an osc_message
 * raw_data points to the first byte of the allocated memory block (if any)
 * address points to the first address byte ('\0' if address is not set)
 * typetag points to the first typetag byte (',')
 */
struct osc_message {
    char* address;
    char* typetag;
    void* raw_data;
};

/**
 * Structure representing an osc_bundle
 * raw_data points to the first byte of the allocated memory block (if any)
 * timetag points to the first byte of the osc_bundle timetag ('\0' if not set)
 */
struct osc_bundle {
    struct osc_timetag* timetag;
    void* raw_data;
};

/**
 * Structure representing an osc_timetag
 * sec is a number of seconds
 * frac is a number of fractions of a second
 */
struct osc_timetag {
    uint32_t sec;
    uint32_t frac;
};

/**
 * Union used for representing osc_message arguments of different types and for accessing particular bytes of an argument
 */
union osc_msg_argument {
    const int32_t i;
    const float f;
    const char s;
    const struct osc_timetag t;
    int32_t *b;
};

/**
 * Converts big-endian int32_t numbers into the host endianity
 *
 * @param   value   big-endian integer to convert
 * @return          the same integer in host endianity
 */
int32_t osc_unpack_int32(int32_t value);

/**
 * Converts big-endian floating point numbers into the host endianity
 *
 * @param    value   big-endian floating point number to convert
 * @return           the same number in the host endianity
 */
float osc_unpack_float(float value);

/**
 * Creates a new osc_message instance by allocating to it 12B (basic osc_message size)
 *
 * @param   msg     pointer to the osc_message structure
 * @return          returns 0 on success or 1 if memory allocation failed
 */

int osc_message_new(struct osc_message* msg);

/**
 * Destroys an osc_message instance by freeing the memory to which its raw_data pointer is pointing
 *
 * @param   msg     pointer to the osc_message structure
 */
void osc_message_destroy(struct osc_message* msg);

/**
 * Finds the length of the osc_message (excluding the first 4B containing the length value)
 *
 * @param    msg    pointer to the osc_message structure
 * @return          osc_message length in the host endianity
 */
size_t osc_message_serialized_length(const struct osc_message* msg);

/**
 * Sets the address bytes of the osc_message instance
 *
 * @param    msg        pointer to the osc_message structure
 * @param    address    pointer to the string to be used as address
 * @return              returns 0 on success or 1 if memory reallocation failed
 */
int osc_message_set_address (struct osc_message* msg, const char* address);

/**
 * Adds the argument of struct osc_timetag type to the osc_message instance
 *
 * @param    msg        pointer to the osc_message structure
 * @param    tag        struct osc_timetag variable to be added to the osc_message as a new argument
 * @return              returns 0 on success or 1 if memory reallocation failed
 */
int osc_message_add_timetag(struct osc_message* msg, struct osc_timetag tag);

/**
 * Adds a string as an argument to the osc_message instance
 *
 * @param    msg        pointer to the osc_message structure
 * @param    data       pointer to the string to be added as an argument
 * @return              returns 0 on success or 1 if memory reallocation failed
 */
int osc_message_add_string(struct osc_message* msg, const char* data);

/**
 * Adds a floating point number as an argument to the osc_message instance
 *
 * @param    msg        pointer to the osc_message structure
 * @param    data       floating point number to be added as an argument
 * @return              returns 0 on success or 1 if memory reallocation failed
 */
int osc_message_add_float(struct osc_message* msg, float data);

/**
 * Adds a 4B integer  as an argument to the osc_message instance
 *
 * @param    msg        pointer to the osc_message structure
 * @param    data       4B integer to be added as an argument
 * @return              returns 0 on success or 1 if memory reallocation failed
 */
int osc_message_add_int32(struct osc_message* msg, int32_t data);

/**
 * Find the number of arguments in the osc_message instance
 *
 * @param   msg         pointer to the osc_message structure
 * @return              the number of arguments
 */
size_t osc_message_argc(const struct osc_message* msg);

/**
 * Finds an argument with the given index in the osc_message instance
 *
 * @param  msg          pointer to the osc_message structure
 * @param  arg_index    index of the desired argument
 * @return              pointer to the argument with the given index or NULL if it doesn't exist
 */
const union osc_msg_argument* osc_message_arg(const struct osc_message* msg, size_t arg_index);

/**
 * Creates a new osc_bundle instance by allocating to it 16B (basic bundle size)
 *
 * @param   bnd         pointer to the osc_bundle structure
 * @return              returns 0 on success or 1 if memory allocation failed
 */
int osc_bundle_new(struct osc_bundle * bnd);

/**
 * Sets the timtetag bytes of the osc_bundle instance
 *
 * @param  bundle       pointer to the osc_bundle structure
 * @param  timetag      value to be used as osc_bundle timetag
 */
void osc_bundle_set_timetag(struct osc_bundle * bundle, struct osc_timetag timetag);

/**
 * Destroys the osc_bundle instance by freeing is memory
 *
 * @param    bn         pointer to the osc_bundle structure
 */
void osc_bundle_destroy(struct osc_bundle * bn);

/**
 * Adds an osc_message instance to the osc_bundle
 *
 * @param  bundle       pointer to the osc_bundle structure
 * @param  msg          pointer to the osc_message instance to be added
 * @return              returns 0 on success or 1 if memory reallocation failed
 */
int osc_bundle_add_message(struct osc_bundle * bundle, const struct osc_message * msg);

/**
 * Finds the osc_message instance immediately following the given instance
 *
 * @param  bundle       pointer to the osc_bundle structure
 * @param  prev         the preceding osc_message instance
 * @return              the next osc_message after the given one or an empty osc_message if the given one is the last in the bundle
 */
struct osc_message osc_bundle_next_message(const struct osc_bundle * bundle, struct osc_message prev);

/**
 * Finds the length of the osc_bundle instance (excluding the first 4B bytes containing the length)
 *
 * @param  bundle       pointer to the osc_bundle structure
 * @return              the length of the osc_bundle
 */
size_t osc_bundle_serialized_length(const struct osc_bundle * bundle);

/**
 * Finds the length of the osc_blob instance (excluding the first 4B bytes containing the length and the alignment bytes)
 *
 * @param  b            osc_blob instance
 * @return              the length of the osc_blob
 */
size_t osc_blob_data_size(const osc_blob b);

/**
 * Finds the first byte of the osc_blob data block
 *
 * @param  b            osc_blob instance
 * @return              pointer to the first byte of the osc_blob data
 */
void * osc_blob_data_ptr(const osc_blob b);

/**
 * Creates an osc_blob instance of the given length
 *
 * @param   length      the desired length of the osc_blob data block
 * @return              osc_blob instance with the desired data block length
 */
osc_blob osc_blob_new(size_t length);

/**
 * Destroys the given osc_blob instance by freeing its memory
 * @param  b            the osc_blob instance to destroy
 */
void osc_blob_destroy(osc_blob b);

/**
 * Adds osc_blob instance as an argument to the osc_message instance
 *
 * @param    msg        pointer to the osc_message structure
 * @param    b          osc_blob instance to be added as an argument
 * @return              returns 0 on success or 1 if memory reallocation failed
 */
int osc_message_add_blob(struct osc_message * msg, const osc_blob b);

#endif //OSC_H
