#pragma once
#include "singletonTemplate.hpp"
#include "Languages.hpp"
#include "LanguagePack.hpp"
#include "FontPack.hpp"

namespace Languages {

    struct LanguagePackCollection : Singleton<LanguagePackCollection> {
        friend class Singleton;
        const LanguagePack& currentLanguagePack() const;
        const FontPack& currentFontPack() const;
        eLanguage currentLanguage() const {
            return _currentLanguage;
        }
        void setLanguage(const eLanguage language) {
            _currentLanguage = language;
        }
        private:
            LanguagePackCollection();
            eLanguage _currentLanguage;
            LanguagePack _rusPack;
            LanguagePack _engPack;
            LanguagePack _itaPack;
            LanguagePack _gerPack;
            LanguagePack _japPack;
            FontPack _fonts;
            FontPack _japaneseFonts;
    };  
};
#define LANGUAGES       Languages::LanguagePackCollection::instance()
#define LANGUAGE_PACK   LANGUAGES.currentLanguagePack()
#define FONT_PACK       LANGUAGES.currentFontPack()
