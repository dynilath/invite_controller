#pragma once

#include <sstream>
#include <cqcppsdk/cqcppsdk.h>

constexpr int64_t myid = 532468158;

void warn_me(const cq::FriendRequestEvent &event, const cq::User &who, bool approved) noexcept {
    try {
        ::std::ostringstream osstr(std::ostringstream::ate);
        osstr << "收到好友请求";
        osstr << "\n用户: " << who.nickname << "(" << who.user_id << ")";
        if (!event.comment.empty()) osstr << "\n请求文本: " << event.comment << "";
        osstr << "\n处理: ";
        if (approved)
            osstr << "通过";
        else
            osstr << "拒绝";
        cq::send_private_message(myid, osstr.str());
    } catch (...) {
        cq::logging::warning("controller", "发生错误");
    }
}

void warn_me_group_invite(const cq::GroupRequestEvent &event, const cq::User &who, int64_t grpid) noexcept {
    try {
        ::std::ostringstream osstr(std::ostringstream::ate);
        osstr << "收到加群邀请";
        osstr << "\n用户：" << who.nickname << "(" << who.user_id << ")";
        osstr << "\n群：" << grpid;
        cq::send_private_message(myid, osstr.str());
    } catch (...) {
        cq::logging::warning("controller", "发生错误");
    }
}

void warn_me_group_leave(const cq::GroupMemberIncreaseEvent &event, const cq::Group &where) noexcept {
    try {
        ::std::ostringstream osstr(std::ostringstream::ate);
        osstr << "退出群：" << where.group_name << "(" << where.group_id << ")";
        cq::send_private_message(myid, osstr.str());
    } catch (...) {
        cq::logging::warning("controller", "发生错误");
    }
}

void warn_me_group_increase(const cq::GroupMemberIncreaseEvent &event, const cq::Group &where, size_t increment) noexcept {
    try {
        ::std::ostringstream osstr(std::ostringstream::ate);
        osstr << "发现群成员数多且在增加：" << where.group_name << "(" << where.group_id << ")";
        osstr << "\n6小时内增量：" << increment;
        cq::send_private_message(myid, osstr.str());
    } catch (...) {
        cq::logging::warning("controller", "发生错误");
    }
}

template <typename T, typename = void>
struct has_group_id : ::std::false_type {};
template <typename T>
struct has_group_id<T, ::std::void_t<decltype(T::group_id)>> : ::std::true_type {};

template <typename T, typename = void>
struct has_user_id : ::std::false_type {};
template <typename T>
struct has_user_id<T, ::std::void_t<decltype(T::user_id)>> : ::std::true_type {};

template <class T, typename ::std::enable_if<has_group_id<T>::value && has_user_id<T>::value, int>::type = 0>
void resolve_event(::std::ostream &stream, const T &evt) {
    stream << "\ngroup_id : " << evt.group_id;
    stream << "\nuser_id : " << evt.user_id;
}

template <class T, typename ::std::enable_if<has_group_id<T>::value && !has_user_id<T>::value, int>::type = 0>
void resolve_event(::std::ostream &stream, const T &evt) {
    stream << "\ngroup_id : " << evt.group_id;
}

template <class T, typename ::std::enable_if<!has_group_id<T>::value && has_user_id<T>::value, int>::type = 0>
void resolve_event(::std::ostream &stream, const T &evt) {
    stream << "\nuser_id : " << evt.user_id;
}

template <class T>
void warn_me_api_err(const cq::ApiError &err, const T &evt) noexcept {
    try {
        ::std::ostringstream osstr(std::ostringstream::ate);
        osstr << "控制器发生ApiError : " << err.what();
        osstr << "\n信息类型: " << typeid(T).name();
        resolve_event(osstr, evt);
        cq::send_private_message(myid, osstr.str());
    } catch (...) {
        cq::logging::warning("controller", "发生错误");
    }
}

template <class T>
void warn_me_std_err(const ::std::exception &err, const T &) noexcept {
    try {
        ::std::ostringstream osstr(std::ostringstream::ate);
        osstr << "控制器发生::std::exception : " << err.what();
        osstr << "\n信息类型: " << typeid(T).name();
        cq::send_private_message(myid, osstr.str());
    } catch (...) {
        cq::logging::warning("controller", "发生错误");
    }
}
