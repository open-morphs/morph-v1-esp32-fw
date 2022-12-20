#pragma once

#include <string>

namespace HttpUtils {
    bool getValueFromForm(const std::string &form, const std::string &key, std::string &out);
};