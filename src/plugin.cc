#include "../extern/corncppsdk/sdk/sdk.h"

#undef max
#undef min

#include <cstdint>
#include <string>
#include <sstream>
#include <cmath>
#include <chrono>
#include <unordered_map>
#include <mutex>
#include <functional>
#include <locale>

#include "./constants.h"
#include "./namecheck.h"
#include "./commands.h"
#include "./group_monitor.h"

using time_point = ::std::chrono::system_clock::time_point;
using duration = ::std::chrono::system_clock::duration;
time_point timer_friend;
time_point timer_group;

bool enabled = false;

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


// Cornerstone SDK 的部分 API 尚未经过测试，可能仍存在漏洞
// 欢迎前往 https://github.com/Sc-Softs/CornerstoneSDK 提交Issue和PR，感谢您的贡献

// 请勿在事件处理函数中执行上传文件等耗时操作，此类操作请另开线程执行

::std::string easy_format(int64_t id,const ::std::string& name){
    ::std::string ret = ::std::to_string(id);
    if(name.empty()) return ret;
    return name + "(" + ret + ")";
}

void send_message_to_master(int64_t this_qq,const ::std::string & msg){
    api->SendFriendMessage(this_qq,myid,msg);
}

// 好友消息事件
EventProcess OnPrivateMessage(volatile PrivateMessageData *data)
{
    auto this_qq = data->ThisQQ;
    auto sender_qq = data->SenderQQ;

    if(sender_qq != myid) return EventProcess::Ignore;

    constexpr auto start_with =[](::std::string const& v,::std::string const&w){
        if(w.size()>v.size()) return false;
        for (size_t i = 0; i < w.size(); i++)
        {
            if(v[i]!=w[i]) return false;
        }
        return true;
    };

    ::std::string msg = data->MessageContent;
    for(auto&[callmsg,callee] : command_vec){
        if(start_with(msg,callmsg)){
            callee(this_qq,msg.substr(callmsg.size()),
            [=](::std::string const & s){
                api->SendFriendMessage(this_qq,myid,s);
            });
            break;
        }
    }

    return EventProcess::Ignore;
}

// 群消息事件
EventProcess OnGroupMessage(volatile GroupMessageData *data)
{
    constexpr auto space_then_point = [](::std::string const & src){
        auto pos = ::std::find_if_not(src.begin(),src.end(),
            [](auto c){return ::std::isspace(c);}
        );
        if(pos == src.end()) return false;
        return *pos == '.';
    };

    auto group_id = data->MessageGroupQQ;

    ::std::string src = data->MessageContent;
    bool is_command = space_then_point(src);
    usage_increase(group_id,is_command);
    global_usage_increase(is_command);
    return EventProcess::Ignore;
}

// 插件卸载事件（未知参数）
EventProcess OnUninstall(void *)
{
    delete api; // 清除全局API对象避免内存泄漏
    return EventProcess::Ignore;
}

// 插件设置事件（未知参数），这里可以弹出对话框
EventProcess OnSettings(void *)
{
    return EventProcess::Ignore;
}

// 插件被启用事件（未知参数）
EventProcess OnEnabled(void *)
{        
    timer_friend = std::chrono::system_clock::now();
    auto t_now = std::chrono::system_clock::now();
    timer_group = t_now - invite_interval;
    api->OutputLog("Master id is " + std::to_string(myid));

    return EventProcess::Ignore;
}

// 插件被禁用事件（未知参数）
EventProcess OnDisabled(void *)
{
    return EventProcess::Ignore;
}

namespace FriendVerificationOperator{
    constexpr int32_t accept = 1;
    constexpr int32_t reject = 2;
}

EventProcess NonGroupEvent(EventData data){
    switch (data.EventType)
    {
    // 好友事件_被好友删除
    case EventType::Friend_Removed:
        break;
    // 好友事件_签名变更
    case EventType::Friend_SignatureChanged:
        break;
    // 好友事件_昵称改变
    case EventType::Friend_NameChanged:
        break;
    // 好友事件_某人撤回事件
    case EventType::Friend_UserUndid:
        break;
    // 好友事件_有新好友
    case EventType::Friend_NewFriend:
        break;
    // 好友事件_好友请求
    case EventType::Friend_FriendRequest:{
        bool approved = false;
        ::std::string msg = data.MessageContent;
        if(check_frd_info(msg)){
            api->ProcessFriendVerificationEvent(
                data.ThisQQ,
                data.TriggerQQ,
                data.MessageSeq,
                static_cast<int32_t>(data.EventType)
            );
            approved = true;
        }
        
        ::std::ostringstream osstr(std::ostringstream::ate);
        osstr << "收到好友请求";
        osstr << "\n用户: " << easy_format(data.OperateQQ, data.OperateQQName);
        if (!msg.empty()) osstr << "\n请求文本: " << msg;
        osstr << "\n处理: ";
        if (approved)
            osstr << "通过";
        else
            osstr << "拒绝";
        send_message_to_master(data.ThisQQ,osstr.str());
        break;
    }
    // 好友事件_对方同意了您的好友请求
    case EventType::Friend_FriendRequestAccepted:
        break;
    // 好友事件_对方拒绝了您的好友请求
    case EventType::Friend_FriendRequestRefused:
        break;
    // 好友事件_资料卡点赞
    case EventType::Friend_InformationLiked:
        break;
    // 好友事件_签名点赞
    case EventType::Friend_SignatureLiked:
        break;
    // 好友事件_签名回复
    case EventType::Friend_SignatureReplied:
        break;
    // 好友事件_个性标签点赞
    case EventType::Friend_TagLiked:
        break;
    // 好友事件_随心贴回复
    case EventType::Friend_StickerLiked:
        break;
    // 好友事件_随心贴增添
    case EventType::Friend_StickerAdded:
        break;
    // 空间事件_与我相关
    case EventType::QZone_Related:
        break;
    // 框架事件_登录成功
    case EventType::This_SignInSuccess:
        break;
    // 其他事件
    default:
        break;
    }
    return EventProcess::Ignore;
}

namespace GroupVerificationOperator{
    constexpr int32_t accept = 11;
    constexpr int32_t reject = 12;
    constexpr int32_t ignore = 14;
}

::std::string format_nickname(int64_t qqid){
    ::std::string nick = api->GetNameFromCache(qqid);
    return easy_format(qqid,nick);
}

::std::string get_format_groupname(int64_t groupid){
    ::std::string nick = api->GetGroupNameFromCache(groupid);
    return easy_format(groupid,nick);
}

EventProcess GroupEvent(EventData data){
    switch (data.EventType)
    {
    // 群事件_我被邀请加入群
    case EventType::Group_Invited:{
        bool approved = false;
        auto now = ::std::chrono::system_clock::now();
        auto available_invite = get_available_invite_count(now,timer_group);

        auto process_group_veri = [&](auto oper){
            api->ProcessGroupVerificationEvent(
                data.ThisQQ,
                data.SourceGroupQQ,
                data.OperateQQ,
                data.MessageSeq,
                oper,
                static_cast<int32_t>(data.EventType)
            );
        };

        if(available_invite > 0){
            process_group_veri(GroupVerificationOperator::accept);
            timer_group = get_time_point_of_target_count(now, available_invite - 1);
            approved = true;
        }else{
            process_group_veri(GroupVerificationOperator::reject);
        }

        {
            ::std::ostringstream osstr(std::ostringstream::ate);
            osstr << "收到加群邀请";
            osstr << "\n用户：" << format_nickname(data.OperateQQ);
            osstr << "\n群：" << data.SourceGroupQQ;
            osstr << "\n处理：";
            if(approved) osstr << "通过";
            else osstr << "拒绝";
            send_message_to_master(data.ThisQQ,osstr.str());
        }

        break;
    }
    // 群事件_某人加入了群
    case EventType::Group_MemberJoined:{
        auto incre = member_increase(data.SourceGroupQQ);
        ::std::string group_name = data.SourceGroupName;

        if(incre > 0){
            ::std::string group_name = data.SourceGroupName;
            ::std::ostringstream osstr(std::ostringstream::ate);
            osstr << "发现群成员数多且在增加：" << easy_format(data.SourceGroupQQ,group_name);
            osstr << "\n6小时内增量：" << incre;
            send_message_to_master(data.ThisQQ,osstr.str());
        }

        if(!check_group_info(group_name)){
            api->QuitGroup(data.ThisQQ,data.SourceGroupQQ);
            ::std::ostringstream osstr(std::ostringstream::ate);
            osstr << "退出群：" << easy_format(data.SourceGroupQQ,group_name);
            send_message_to_master(data.ThisQQ,osstr.str());
        }

        break;
    }
    // 群事件_某人申请加群
    case EventType::Group_MemberVerifying:
        break;
    // 群事件_群被解散
    case EventType::Group_GroupDissolved:
        break;
    // 群事件_某人退出了群
    case EventType::Group_MemberQuit:
        break;
    // 群事件_某人被踢出群
    case EventType::Group_MemberKicked:
        break;
    // 群事件_某人被禁言
    case EventType::Group_MemberShutUp:
        break;
    // 群事件_某人撤回事件
    case EventType::Group_MemberUndid:
        break;
    // 群事件_某人被取消管理
    case EventType::Group_AdministratorTook:
        break;
    // 群事件_某人被赋予管理
    case EventType::Group_AdministratorGave:
        break;
    // 群事件_开启全员禁言
    case EventType::Group_EnableAllShutUp:
        break;
    // 群事件_关闭全员禁言
    case EventType::Group_DisableAllShutUp:
        break;
    // 群事件_开启匿名聊天
    case EventType::Group_EnableAnonymous:
        break;
    // 群事件_关闭匿名聊天
    case EventType::Group_DisableAnonymous:
        break;
    // 群事件_开启坦白说
    case EventType::Group_EnableChatFrankly:
        break;
    // 群事件_关闭坦白说
    case EventType::Group_DisableChatFrankly:
        break;
    // 群事件_允许群临时会话
    case EventType::Group_AllowGroupTemporary:
        break;
    // 群事件_禁止群临时会话
    case EventType::Group_ForbidGroupTemporary:
        break;
    // 群事件_允许发起新的群聊
    case EventType::Group_AllowCreateGroup:
        break;
    // 群事件_禁止发起新的群聊
    case EventType::Group_ForbidCreateGroup:
        break;
    // 群事件_允许上传群文件
    case EventType::Group_AllowUploadFile:
        break;
    // 群事件_禁止上传群文件
    case EventType::Group_ForbidUploadFile:
        break;
    // 群事件_允许上传相册
    case EventType::Group_AllowUploadPicture:
        break;
    // 群事件_禁止上传相册
    case EventType::Group_ForbidUploadPicture:
        break;
    // 群事件_某人被邀请入群
    case EventType::Group_MemberInvited:
        break;
    // 群事件_展示成员群头衔
    case EventType::Group_ShowMemberTitle:
        break;
    // 群事件_隐藏成员群头衔
    case EventType::Group_HideMemberTitle:
        break;
    // 群事件_某人被解除禁言
    case EventType::Group_MemberNotShutUp:
        break;
    // 其他事件
    default:
        break;
    }
    return EventProcess::Ignore;
}

// 事件消息
EventProcess OnEvent(volatile EventData *data)
{
    if (data->SourceGroupQQ == 0) // 非群事件
    {
        return NonGroupEvent(*const_cast<EventData*>(data));
    }
    else // 群事件
    {
        return GroupEvent(*const_cast<EventData*>(data));
    }
    return EventProcess::Ignore;
}
