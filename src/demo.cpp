#include <iostream>
#include <set>
#include <sstream>

#include <cqcppsdk/cqcppsdk.h>

#include <chrono>
#include <cmath>
#include <cstdint>
#include <sstream>
#include <unordered_map>
#include <mutex>
#include <functional>

#include "./warnme.h"
#include "./namecheck.h"
#include "./commands.h"
#include "./group_monitor.h"

using time_point = ::std::chrono::system_clock::time_point;
using duration = ::std::chrono::system_clock::duration;
time_point timer_friend;
time_point timer_group;

bool enabled = false;

using namespace cq;
using Message = cq::message::Message;
using MessageSegment = cq::message::MessageSegment;

constexpr ::std::chrono::seconds invite_interval(1800);
constexpr int64_t WAIT_MAX_HOLD = 5;
constexpr ::std::chrono::seconds one_sec(1);


auto get_available_invite_count(time_point now, time_point past) {
    return ::std::min(((now - past) / invite_interval), WAIT_MAX_HOLD);
};
auto get_time_point_of_target_count(time_point now, int64_t target_count) {
    return now - target_count * invite_interval;
};

constexpr unsigned group_member_threshold = 500;

::std::mutex message_counter_mutex;
::std::unordered_map<int64_t, uint64_t> group_message_counter;

template<class T>
struct c_array_info;
template<class A, size_t N>
struct c_array_info<A[N]> {
    using type = A;
    static constexpr size_t length = N;
};

template<class T,class E>
void catch_wrapper(const E &event,T runnable) noexcept {
    try {
        runnable();
    } catch (const ApiError &err) {
        warn_me_api_err(err, event);
    } catch (const ::std::exception &except) {
        warn_me_std_err(except, event);
    }
}

bool start_with(const ::std::string &src, const ::std::string &compare) {
    if (src.size() < src.size()) return false;
    for (size_t i = 0; i < compare.size(); i++) {
        if (src[i] != compare[i]) return false;
    }
    return true;
}

template <class T>
bool command_resolver(const ::std::string &src, const ::std::string &cmd, T callable) {
    if (start_with(src, cmd)) {
        callable(src.substr(cmd.size()));
        return true;
    }
    return false;
}

bool space_then_point(const ::std::string &src) {
    auto t = src.find_first_not_of(" ");
    if (t != ::std::string::npos && t < src.size()) return src[t] == '.';
    return false;
}

CQ_INIT {
    on_enable([] {
        enabled = true;
        timer_friend = std::chrono::system_clock::now();
        auto t_now = std::chrono::system_clock::now();
        timer_group = t_now - invite_interval;
        cq::logging::info("invite_controller", "Master id is " + std::to_string(myid));
    });

    on_friend_request([](const FriendRequestEvent &event) {
        catch_wrapper(event,[&event]() {
            auto uinfo = get_stranger_info(event.user_id);
            bool approved = false;
            do {
                if (!check_frd_info(uinfo, event.comment)) {
                    break;
                }
                set_friend_request(event.flag, RequestEvent::Operation::APPROVE);
                approved = true;
            } while (false);
            warn_me(event, uinfo, approved);
        });
    });

    on_group_message([](const GroupMessageEvent &event) { 
        bool is_command = space_then_point(event.message);
        usage_increase(event.group_id, is_command);
        global_usage_increase(is_command);
    });

    on_message([](const MessageEvent &event) {
        if (event.user_id != myid) return;
        catch_wrapper(event, [&event]() {
            for (auto &[cmd, callee] : command_vec) {
                if (command_resolver(event.message, cmd, callee)) return;
            }
        });
    });

    on_group_member_increase([](const GroupMemberIncreaseEvent &event) {
        catch_wrapper(event, [&event]() {
            cq::Group ginfo;

            try {
                ginfo = get_group_info(event.group_id);
            } catch (const cq::ApiError &) {
                ginfo.member_count = 0;
            }

            auto incre = member_increase(event.group_id);
            if (incre > 0) {
                warn_me_group_increase(event, ginfo, incre);
            }

            if (!check_group_info(ginfo) || (ginfo.member_count > group_member_threshold && incre > 10)) {
                set_group_leave(event.group_id, false);
                usage_remove(event.group_id);

                warn_me_group_leave(event, ginfo);
            }
        });
    });

    on_group_request([](const GroupRequestEvent &event) {
        catch_wrapper(event, [&event]() {
            if (event.sub_type != GroupRequestEvent::SubType::INVITE) return;
            auto r = get_stranger_info(event.user_id);
            bool approved = false;

            do {
                time_point now = std::chrono::system_clock::now();
                auto available_invite = get_available_invite_count(now, timer_group);
                if (available_invite > 0) {
                    set_group_request(event.flag, event.sub_type, RequestEvent::Operation::APPROVE);
                    timer_group = get_time_point_of_target_count(now, available_invite - 1);
                    approved = true;
                } else {
                    std::ostringstream osstream(std::ostringstream::ate);
                    osstream << u8"加群控制器 by dynilath\n";
                    osstream << u8"等待" << invite_interval / one_sec << "秒后，该机器人将开放1次邀请加群";
                    send_private_message(event.user_id, osstream.str());
                    set_group_request(event.flag, event.sub_type, RequestEvent::Operation::REJECT);
                }
            } while (false);
            warn_me_group_invite(event, r, event.group_id);
        });
    });
}