#pragma once
#include <cstdint>
#include <vector>

void usage_clear() noexcept;
void usage_increase(int64_t grpid, bool is_dice_call) noexcept;
void usage_remove(int64_t grpid) noexcept;
size_t member_increase(int64_t grpid) noexcept;

void global_usage_increase(bool is_dice_call) noexcept;
::std::pair<size_t, size_t> get_global_usage() noexcept;

using msg_counter_t = ::std::pair<unsigned, unsigned>;
msg_counter_t get_target_usage(int64_t) noexcept;
::std::vector<::std::pair<int64_t, msg_counter_t>> get_usage_vec() noexcept;
::std::vector<::std::pair<int64_t, msg_counter_t>> get_general_usage_vec() noexcept;