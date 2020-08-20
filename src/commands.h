#pragma once

#include <string>
#include <functional>
#include <vector>

using sender_t = ::std::function<void(::std::string const&)>;
using callee_t = ::std::function<void(int64_t,::std::string,sender_t)>;

using cmd_vec_type = ::std::vector<::std::pair<::std::string, callee_t>>;
extern cmd_vec_type command_vec;