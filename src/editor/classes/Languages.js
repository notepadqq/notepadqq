var Languages = new function() {

    var m_currentLanguage = "plaintext";
    this.languages = undefined;

    this.setLanguage = function(editor, lang) {
        if (lang.mime !== undefined && lang.mime !== null && lang.mime !== "") {
            editor.setOption("mode", lang.mime);
        }else {
            editor.setOption("mode", lang.mode);
        }

        m_currentLanguage = lang.id;
        editor.refresh();
        evhook.onLanguageChange(UiDriver.proxy, editor, lang);
    }
    
    this.currentLanguage = function() {
        return m_currentLanguage;
    }
}
