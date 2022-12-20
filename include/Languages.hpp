#pragma once

namespace Languages {

    enum eLanguage {
        LANGUAGE_ENG,
        LANGUAGE_RUS,
        LANGUAGE_ITA,
        LANGUAGE_GER,
        LANGUAGE_JAP,
        LANGUAGE_MAX,
    };

    const char** russianGlossary();
    const char** englishGlossary();
    const char** italianGlossary();
    const char** germanGlossary();
    const char** japaneseGlossary();
};