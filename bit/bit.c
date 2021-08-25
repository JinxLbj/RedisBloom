#include <stdio.h>
#include "src/cms.h"
#include "bit/bit.h"

#define BIT_NUM_PRE_BYTE 8
#define MASK (u_int8_t)1

static u_int64_t MAX_PRE_BIT[65] = {0,1,3,7,15,
                                   31,63,127,255,511,
                                   1023,2047,4095,8191,16383,
                                   32767,65535,131071,262143,524287,
                                   1048575,2097151,4194303,8388607,16777215,
                                   33554431,67108863,134217727,268435455,536870911,
                                   1073741823,2147483647,4294967295,8589934591,17179869183,
                                   34359738367,68719476735,137438953471,274877906943,549755813887,
                                   1099511627775,2199023255551,4398046511103,8796093022207,17592186044415,
                                   35184372088831,70368744177663,140737488355327,281474976710655,562949953421311,
                                   1125899906842623,2251799813685247,4503599627370495,9007199254740991,18014398509481983,
                                   36028797018963967,72057594037927935,144115188075855871,288230376151711743,576460752303423487,
                                   1152921504606846975,2305843009213693951,4611686018427387903,9223372036854775807,18446744073709551615};

u_int8_t get_min_need_bit(u_int64_t max) {
    for (int i = 0; i < 65; i++) {
        if (max <= MAX_PRE_BIT[i]) {
            return i;
        }
    }
    return 64;
}

u_int64_t get_bit_num(const CMSketch *cms, uint64_t index) {
    return get_num(cms->array, index * cms->num_use_bit, cms->num_use_bit);
}

void set_bit_num(const CMSketch *cms, uint64_t index, uint64_t num) {
    return set_num(cms->array, index * cms->num_use_bit, cms->num_use_bit, num);
}

u_int64_t get_num(const u_int8_t *arr, u_int64_t start_bit, u_int8_t len) {
    if (len > 64) {
        return 0;
    }
    u_int8_t offset = start_bit % BIT_NUM_PRE_BYTE;
    u_int64_t byte = start_bit / BIT_NUM_PRE_BYTE;
    u_int64_t result_num = 0;
    for (int i = 0; i < len; i++) {
        //get target byte
        u_int8_t var1 = *(arr + byte);
        //make bit to lowest location
        var1 >>= (u_int8_t) (BIT_NUM_PRE_BYTE - 1 - offset);
        //use 00000001
        u_int8_t var2 = var1 & MASK;
        //num = num^2 + (0 | 1)
        result_num <<= (u_int8_t) 1;
        result_num += var2;
        offset += 1;
        //reset
        if (offset >= BIT_NUM_PRE_BYTE) {
            offset = 0;
            byte += 1;
        }
    }
    return result_num;
}

void set_num(u_int8_t *arr, u_int64_t start_bit, u_int8_t len, u_int64_t num) {
    u_int8_t min_bit = get_min_need_bit(num);
    if (min_bit > len) {
        return;
    }
    u_int64_t end_bit = start_bit + len - 1;
    //get last bit location
    u_int8_t offset = end_bit / BIT_NUM_PRE_BYTE;
    u_int64_t byte = end_bit % BIT_NUM_PRE_BYTE;
    for (int i = 0; i < len; i++) {
        //get target byte
        u_int8_t var1 = *(arr + byte);
        //use mask to update bit
        if ((num & MASK) == 0) {
            *(arr + byte) =
                    var1 & (u_int8_t) ~((u_int8_t) ((u_int8_t) 1 << (u_int8_t) (BIT_NUM_PRE_BYTE - 1 - offset)));
        } else {
            *(arr + byte) = var1 | (u_int8_t) ((u_int8_t) 1 << (u_int8_t) (BIT_NUM_PRE_BYTE - 1 - offset));
        }
        offset -= 1;
        //offset is an unsigned value, if < 0, it will overflow and reset
        if (offset >= BIT_NUM_PRE_BYTE) {
            offset = BIT_NUM_PRE_BYTE - 1;
            byte -= 1;
        }
        num >>= (u_int8_t) 1;
    }

}