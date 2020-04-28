#include "group_monitor.h"

#include <unordered_map>
#include <mutex>
#include <cstdint>
#include <algorithm>
#include <deque>
#include <chrono>

namespace {
    constexpr unsigned group_member_threshold = 500;
    ::std::mutex message_counter_mutex;
    ::std::unordered_map<int64_t, msg_counter_t> group_message_counter;

    using time_point = ::std::chrono::system_clock::time_point;
    ::std::mutex member_counter_mutex;
    using time_deque_t = ::std::deque<time_point>;
    ::std::unordered_map<int64_t, time_deque_t> group_member_counter;

    
    ::std::mutex global_message_counter_mutex;
    time_deque_t global_usage_counter;
    time_deque_t global_burden_counter;

    void pop_front_before(time_deque_t &deq, const time_point &value) {
        while (!deq.empty()) {
            if (deq.front() < value) {
                deq.pop_front();
            } else
                break;
        }
    }

} // namespace

size_t member_increase(int64_t grpid) noexcept {
    auto iter = group_member_counter.find(grpid);
    member_counter_mutex.lock();
    if (iter != group_member_counter.end()) {
        time_point now = ::std::chrono::system_clock::now();
        iter->second.push_back(now);
        time_point six_hour_ago = now - ::std::chrono::hours(6);
        pop_front_before(iter->second, six_hour_ago);
    } else {
        auto &[n_iter, good] = group_member_counter.insert({grpid, time_deque_t()});
        iter = n_iter;
        iter->second.push_back(::std::chrono::system_clock::now());
    }
    member_counter_mutex.unlock();

    auto cnt = iter->second.size();
    if (cnt % 10 == 0) {
        return iter->second.size();
    }
    return 0;
}

void usage_clear() noexcept {
    message_counter_mutex.lock();
    for (auto &r : group_message_counter) {
        r.second = {0,0};
    }
    message_counter_mutex.unlock();
}

void global_usage_increase(bool is_dice_call) noexcept {
    auto now = ::std::chrono::system_clock::now();
    auto an_hour_ago = now - ::std::chrono::hours(1);
    global_message_counter_mutex.lock();
    pop_front_before(global_usage_counter, an_hour_ago);
    if(is_dice_call) global_usage_counter.push_back(now);
    pop_front_before(global_burden_counter, an_hour_ago);
    global_burden_counter.push_back(now);
    global_message_counter_mutex.unlock();
}

::std::pair<size_t, size_t> get_global_usage() noexcept {
    return {global_usage_counter.size(), global_burden_counter.size()};
}

void usage_increase(int64_t grpid, bool is_dice_call) noexcept {
    auto iter = group_message_counter.find(grpid);
    message_counter_mutex.lock();
    if (iter != group_message_counter.end()) {
        iter->second.first += 1;
        if (is_dice_call) iter->second.second += 1;
    } else {
        if (is_dice_call)
            group_message_counter[grpid] = {1, 1};
        else
            group_message_counter[grpid] = {1, 0};
    }
    message_counter_mutex.unlock();
}

void usage_remove(int64_t grpid) noexcept {
    if (group_message_counter.find(grpid) != group_message_counter.end()) {
        message_counter_mutex.lock();
        group_message_counter.erase(grpid);
        message_counter_mutex.unlock();
    }

    if (group_member_counter.find(grpid) != group_member_counter.end()) {
        member_counter_mutex.lock();
        group_member_counter.erase(grpid);
        member_counter_mutex.unlock();
    }
}

::std::vector<::std::pair<int64_t, msg_counter_t>> get_usage_vec() noexcept {
    using counter_type = decltype(group_message_counter);
    using v_type = ::std::pair<int64_t, msg_counter_t>;
    ::std::vector<v_type> sorter;
    sorter.reserve(group_message_counter.size());
    for (auto &r : group_message_counter) {
        sorter.push_back({r.first, r.second});
    }

    ::std::sort(
        sorter.begin(), sorter.end(), [](const counter_type::value_type &lval, const counter_type::value_type &rval) {
            return lval.second.second == rval.second.second ? lval.second.first > rval.second.first
                                                            : lval.second.second < rval.second.second;
        });
    return sorter;
}

::std::vector<::std::pair<int64_t, msg_counter_t>> get_general_usage_vec() noexcept {
    using counter_type = decltype(group_message_counter);
    using v_type = ::std::pair<int64_t, msg_counter_t>;
    ::std::vector<v_type> sorter;
    sorter.reserve(group_message_counter.size());
    for (auto &r : group_message_counter) {
        sorter.push_back({r.first, r.second});
    }

    ::std::sort(
        sorter.begin(), sorter.end(), [](const counter_type::value_type &lval, const counter_type::value_type &rval) {
            return lval.second.first > rval.second.first;
        });
    return sorter;
}

msg_counter_t get_target_usage(int64_t grpid) noexcept {
    msg_counter_t ret = {0, 0};
    auto iter = group_message_counter.find(grpid);
    if (iter != group_message_counter.end()) {
        ret = iter->second;
    }
    return ret;
}