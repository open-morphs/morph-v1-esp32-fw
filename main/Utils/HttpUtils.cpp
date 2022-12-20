#include <Utils/HttpUtils.hpp>
#include <iostream>

bool HttpUtils::getValueFromForm(const std::string &form, const std::string &key, std::string &out) {
    const size_t keyPosistion{form.find(key.c_str(), 0)};
    // printf("keyPosistion: %u\n", keyPosistion);
    if (std::string::npos == keyPosistion or (form.length() - keyPosistion) <= key.length()) {
        return false;
    }

    const size_t valuePosition{keyPosistion + key.length() + 1};
    // printf("valuePosition: %u\n", valuePosition);
    if (form.at(valuePosition - 1) != '=') {
        return false;
    }

    const size_t ampersandPosition{form.find('&', valuePosition)};
    // printf("ampersandPosition: %u\n", ampersandPosition);
    out = form.substr(valuePosition, std::string::npos == ampersandPosition ?
                                     (form.length() - valuePosition - 1) : (ampersandPosition - valuePosition));

    return true;
}
