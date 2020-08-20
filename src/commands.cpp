#include "commands.h"

#include "../extern/corncppsdk/sdk/sdk.h"

#include <cstdint>
#include <sstream>

#include "constants.h"
#include "group_monitor.h"

namespace {
    constexpr char grp_info[] = "group info";
    auto grp_info_call = [](int64_t this_qq,::std::string msg, sender_t sender) {
        int64_t grpid = ::std::stoll(msg);

        auto [usage_burden, total_burden] = get_target_usage(grpid);

        ::std::ostringstream osstr(std::ostringstream::ate);

        // GroupCardInformation info;
        // auto good = api->GetGroupInformation(this_qq,grpid,info);

        // if(good){
        //     osstr << "群：" << info.GroupName << "(" << grpid << ")";
        //     osstr << "\n总负载/指令使用：" << usage_burden << "/" << total_burden;
        // }
        // else{
        //     osstr << "错误：不在群" << grpid << "中，或者群不存在";
        // }

        osstr << "群：" << "(" << grpid << ")";
        osstr << "\n总负载/指令使用：" << usage_burden << "/" << total_burden;

        sender(osstr.str());
    };

    constexpr char grp_exit[] = "group exit";
    auto grp_exit_call = [](int64_t this_qq,::std::string msg, sender_t sender) {
        int64_t grpid = ::std::stoll(msg);
        ::std::ostringstream osstr(std::ostringstream::ate);
        
        // GroupCardInformation info;
        // auto good_grp_info = api->GetGroupInformation(this_qq,grpid,info);

        auto good = api->QuitGroup(this_qq,grpid);

        if(good){
            usage_remove(grpid);
            // if (good_grp_info)
            //     osstr << "退出群：" << info.GroupName << "(" << grpid << ")";
            // else
                osstr << "退出群：" << grpid << "";
        }
        else{
            osstr << "错误：不在群" << grpid << "中，或者群不存在";
        }
        sender(osstr.str());
    };

    constexpr char user_info[] = "user info";
    auto user_info_call = [](int64_t this_qq,::std::string msg, sender_t sender) {
        // int64_t userid = ::std::stoll(msg);

        // FriendInformation info;
        // auto good = api->GetFriendInformation(this_qq,userid,info);

        // ::std::ostringstream osstr(std::ostringstream::ate);
        // if(good){
        //     osstr << "用户：" << info.Name << "(" << info.QQNumber << ")\n";
        // }
        // else{
        //     osstr << "错误：无法找到" << userid << "的信息";
        // }
        // sender(osstr.str());
    };

    constexpr char top_clear[] = "top clear";
    auto top_clear_call = [](int64_t this_qq,::std::string msg, sender_t sender) {
        usage_clear();
        sender("已清理消息记录");
    };


    auto top_waht__call = [](int64_t this_qq,const decltype(get_general_usage_vec()) &vec, size_t usage, size_t burden) -> std::string {
        ::std::ostringstream osstr(std::ostringstream::ate);
        osstr << "1小时内消息总数/指令数: " << burden << "/" << usage;
        osstr << "\n群消息统计如下：";
        for (size_t i = 0; i < vec.size() && i < 15; i++) {
            auto [grpid, cnt] = vec[i];
            //GroupCardInformation info;
            //auto good = api->GetGroupInformation(this_qq, grpid, info);
            // if(good){
            //     osstr << "\n"
            //           << i << ": 群：" << info.GroupName << "(" << grpid << ") : " << cnt.first << "/"
            //           << cnt.second;
            // }
            // else
            {
                osstr << "\n" << i << ": 群：(" << grpid << ") : " << cnt.first << "/" << cnt.second;
            }
        }
        return osstr.str();
    };


    constexpr char top_general[] = "top general";
    auto top_general_call = [](int64_t this_qq,::std::string msg, sender_t sender) {
        auto sorter = get_general_usage_vec();
        auto [usage_burden, total_burden] = get_global_usage();
        auto send = top_waht__call(this_qq,sorter, usage_burden, total_burden);
        sender(send);
    };

    constexpr char top_info[] = "top";
    auto top_info_call = [](int64_t this_qq,::std::string msg, sender_t sender) {
        auto sorter = get_usage_vec();
        auto [usage_burden, total_burden] = get_global_usage();
        auto send = top_waht__call(this_qq, sorter, usage_burden, total_burden);
        sender(send);
    };

    constexpr char repeat_msg[] = "repeat";
    auto repeat_msg_call = [](int64_t this_qq,::std::string msg, sender_t sender) {
        auto start = ::std::find_if(msg.begin(),msg.end(),[](auto c){return !::std::isspace(c);});
        ::std::string s(start,msg.end());
        if(!s.empty())
            sender(msg);
    };

    constexpr char repeat_raw_msg[] = "repeat raw";
    auto repeat_raw_msg_call = [](int64_t this_qq,::std::string msg, sender_t sender) {
        
        auto start = ::std::find_if(msg.begin(),msg.end(),[](auto c){return !::std::isspace(c);});
        auto new_start = start;

        ::std::ostringstream oss;

        while(start != msg.end()){
            auto new_start = ::std::find_if(start, msg.end() ,[](auto c){
                return c>='[' && c <= ']';
            });
            oss << ::std::string(start,new_start);
            if(new_start == msg.end()) break;

            switch (*new_start)
            {
            case '[':
                oss << "\\u005b";
                break;
            case '\\':
                oss << "\\u005c";
                break;
            case ']':
                oss << "\\u005d";
                break;
            default:
                break;
            }

            start = new_start + 1;
        }
        auto s = oss.str();
        if(!s.empty())
            sender(oss.str());
    };
}

cmd_vec_type command_vec = {{grp_info, grp_info_call},
                            {grp_exit, grp_exit_call},
                            {user_info, user_info_call},
                            {top_clear, top_clear_call},
                            {top_general, top_general_call},
                            {top_info, top_info_call},
                            {repeat_raw_msg,repeat_raw_msg_call},
                            {repeat_msg,repeat_msg_call}};
