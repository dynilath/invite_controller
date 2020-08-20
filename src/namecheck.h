#pragma once

const ::std::vector bad_grp_name = {u8"红包",
                                    u8"互赞",
                                    u8"名片赞",
                                    u8"聊天",
                                    u8"邀人",
                                    u8"免费",
                                    u8"拉人",
                                    u8"表白",
                                    u8"交友",
                                    u8"拉一个人",
                                    u8"拉一人"};

const ::std::vector bad_frd_proposal = {u8"扩列", u8"互暖", u8"互z", u8"拉我", u8"互赞"};

auto check_frd_info(const ::std::string& proposal) {
    for (auto &s : bad_frd_proposal) {
        if (proposal.find(s) != ::std::string::npos) {
            return false;
        }
    }
    return true;
};

auto check_group_info(const ::std::string &group_name) {
    for (auto &s : bad_grp_name) {
        if (group_name.find(s) != ::std::string::npos) {
            return false;
        }
    }
    return true;
};