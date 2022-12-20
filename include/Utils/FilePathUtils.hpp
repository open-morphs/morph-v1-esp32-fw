#pragma once

#include <string>

namespace FilePathUtils {
    inline std::string trimFilePath(const std::string &fullpath) {

        size_t lastSlashIndex = fullpath.find_last_of('/');
        return lastSlashIndex == std::string::npos ? "" : fullpath.substr(lastSlashIndex + 1);
    }
};