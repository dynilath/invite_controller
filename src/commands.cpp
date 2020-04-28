#include "commands.h"

#include <cstdint>
#include <cqcppsdk/cqcppsdk.h>

#include "constants.h"
#include "group_monitor.h"

namespace {
    constexpr char grp_info[] = "group info";
    auto grp_info_call = [](::std::string msg) {
        int64_t grpid = ::std::stoll(msg);

        auto [usage_burden, total_burden] = get_target_usage(grpid);

        ::std::ostringstream osstr(std::ostringstream::ate);
        try {
            auto grpinfo = cq::get_group_info(grpid, true);
            osstr << "群：" << grpinfo.group_name << "(" << grpinfo.group_id << ")";
            osstr << "\n群成员数：" << grpinfo.member_count;
            osstr << "\n总负载/指令使用：" << usage_burden << "/" << total_burden;

        } catch (const cq::ApiError&) {
            osstr << "错误：不在群" << grpid << "中，或者群不存在";
        }

        cq::send_private_message(myid, osstr.str());
    };

    constexpr char grp_exit[] = "group exit";
    auto grp_exit_call = [](::std::string msg) {
        int64_t grpid = ::std::stoll(msg);
        ::std::ostringstream osstr(std::ostringstream::ate);

        cq::Group grpinfo;
        bool no_grp_info = false;

        try {
            grpinfo = cq::get_group_info(grpid, true);
        } catch (const cq::ApiError&) {
            no_grp_info = true;
        }

        try {
            cq::set_group_leave(grpid, false);
            usage_remove(grpid);
            if (no_grp_info)
                osstr << "退出群：" << grpid << "";
            else
                osstr << "退出群：" << grpinfo.group_name << "(" << grpinfo.group_id << ")";
        } catch (const cq::ApiError&) {
            osstr << "错误：不在群" << grpid << "中";
        }

        cq::send_private_message(myid, osstr.str());
    };

    constexpr char user_info[] = "user info";
    auto user_info_call = [](::std::string msg) {
        int64_t userid = ::std::stoll(msg);
        auto usrinfo = cq::get_stranger_info(userid, true);

        ::std::ostringstream osstr(std::ostringstream::ate);
        osstr << "用户：" << usrinfo.nickname << "(" << usrinfo.user_id << ")\n";
        cq::send_private_message(myid, osstr.str());
    };

    constexpr char top_clear[] = "top clear";
    auto top_clear_call = [](::std::string msg) {
        usage_clear();
        cq::send_private_message(myid, "已清理消息记录");
    };


    auto top_waht__call = [](const decltype(get_general_usage_vec()) &vec, size_t usage, size_t burden) -> std::string {
        ::std::ostringstream osstr(std::ostringstream::ate);
        osstr << "1小时内消息总数/指令数: " << burden << "/" << usage;
        osstr << "\n群消息统计如下：";
        for (size_t i = 0; i < vec.size() && i < 15; i++) {
            auto [grpid, cnt] = vec[i];
            try {
                auto grpinfo = cq::get_group_info(grpid);
                osstr << "\n"
                      << i << ": 群：" << grpinfo.group_name << "(" << grpinfo.group_id << ") : " << cnt.first << "/"
                      << cnt.second;
            } catch (const cq::ApiError &) {
                osstr << "\n" << i << ": 群：(" << grpid << ") : " << cnt.first << "/" << cnt.second;
            }
        }
        return osstr.str();
    };


    constexpr char top_general[] = "top general";
    auto top_general_call = [](::std::string msg) {
        auto sorter = get_general_usage_vec();
        auto [usage_burden, total_burden] = get_global_usage();

        auto send = top_waht__call(sorter, usage_burden, total_burden);
        ::std::ostringstream osstr(std::ostringstream::ate);
        cq::send_private_message(myid, send);
    };

    constexpr char top_info[] = "top";
    auto top_info_call = [](::std::string msg) {
        auto sorter = get_usage_vec();
        auto [usage_burden, total_burden] = get_global_usage();

        auto send = top_waht__call(sorter, usage_burden, total_burden);
        ::std::ostringstream osstr(std::ostringstream::ate);
        cq::send_private_message(myid, send);
    };
}

cmd_vec_type command_vec = {{grp_info, grp_info_call},
                            {grp_exit, grp_exit_call},
                            {user_info, user_info_call},
                            {top_clear, top_clear_call},
                            {top_general, top_general_call},
                            {top_info, top_info_call}};
