#pragma once

#include <string>
#include <functional>
#include <vector>

using cmd_vec_type = ::std::vector<::std::pair<::std::string, ::std::function<void(::std::string)>>>;
extern cmd_vec_type command_vec;