//
// Created by 云动九天 on 2021-08-18.
//

#ifndef DYNAMIC_TYPE_BIT_H
#define DYNAMIC_TYPE_BIT_H

u_int8_t get_min_need_bit(u_int64_t max);

u_int64_t get_bit_num(const CMSketch *cms, uint64_t index);

void set_bit_num(const CMSketch *cms, uint64_t index, uint64_t num);

u_int64_t get_num(const u_int8_t *arr, u_int64_t start_bit, unsigned char len);

void set_num(u_int8_t *arr, u_int64_t start_bit, u_int8_t len, u_int64_t num);

#endif //DYNAMIC_TYPE_BIT_H
