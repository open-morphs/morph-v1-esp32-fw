#include "LanguagePackCollection.hpp"
#include "Languages.hpp"
#include "assert.h"
#include "Fonts.hpp"

using namespace Languages;

LanguagePackCollection::LanguagePackCollection()
    :   _currentLanguage{eLanguage::LANGUAGE_ENG},
        _rusPack(russianGlossary()),
        _engPack(englishGlossary()),
        _itaPack(italianGlossary()),
        _gerPack(germanGlossary()),
        _japPack(japaneseGlossary()),
        _fonts(fonts()),
        _japaneseFonts(japaneseFonts()) {

}

const LanguagePack& LanguagePackCollection::currentLanguagePack() const {

    switch (_currentLanguage) {
        case eLanguage::LANGUAGE_ENG:
            return _engPack;

        case eLanguage::LANGUAGE_RUS:
            return _rusPack;
        
        case eLanguage::LANGUAGE_ITA:
            return _itaPack;

        case eLanguage::LANGUAGE_GER:
            return _gerPack;

        case eLanguage::LANGUAGE_JAP:
            return _japPack;
    
        default:
            assert(false);
        break;
    }

    return _engPack;
}

const FontPack& LanguagePackCollection::currentFontPack() const {
    switch (_currentLanguage) {
        case eLanguage::LANGUAGE_JAP:\
            return _japaneseFonts;

        default:
            return _fonts;
    }
    return _fonts;
}