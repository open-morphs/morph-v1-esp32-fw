#pragma once
#include "TitlesTypes.hpp"
#include <array>

namespace Languages {

    struct LanguagePack {

        const char* operator[](const eTitleType type) const {
            return _titles.at(static_cast<size_t>(type));
        }

        explicit LanguagePack(const char** data) {
            std::copy(data, data + static_cast<size_t>(eTitleType::TITLE_MAX), _titles.begin());
        }

    private:
        std::array<const char*, static_cast<size_t>(eTitleType::TITLE_MAX)> _titles; 
    };
};