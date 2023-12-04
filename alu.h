#ifndef COMPARISONS_H
#define COMPARISONS_H

#include <cstdint>

bool alu_less_than();
bool alu_greater_or_equal_than();
bool alu_equal_than();
bool alu_less_or_equal_than();
bool alu_not_equal_than();
bool alu_greater_than();
uint8_t alu_get_state();
uint32_t alu_get_a();
uint32_t alu_get_b();
void alu_set_a(uint32_t a);
void alu_set_b(uint32_t b);
uint32_t alu_get_sum();
void alu_set_carry_in(bool carry);
void alu_set_flags(uint8_t flags);
constexpr uint32_t alu_greater_than_bit = 1;
constexpr uint32_t alu_equals_bit = 2;
constexpr uint32_t alu_less_than_bit = 4;
constexpr uint32_t alu_carry_in_bit = 8;
constexpr uint32_t alu_flags_bits = alu_greater_than_bit | alu_equals_bit | alu_less_than_bit;

#endif
