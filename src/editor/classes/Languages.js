var Languages = new function() {

    var m_currentLanguage = "plaintext";
    this.languages = undefined;

    this.setLanguage = function(editor, lang) {
        if (lang.mime !== undefined && lang.mime !== null && lang.mime !== "")
            editor.setOption("mode", lang.mime);
        else
            editor.setOption("mode", lang.mode);
            
        m_currentLanguage = lang.id;

        if (editor.getOption("foldGutter") == true) {
            editor.setOption("foldGutter", false);
            editor.setOption("foldGutter", true);
        }

        evhook.onLanguageChange(UiDriver.proxy, editor, lang);
    }
    
    this.currentLanguage = function() {
        return m_currentLanguage;
    }
    
    function endsWith(str, suffix) {
        return str.indexOf(suffix, str.length - suffix.length) !== -1;
    }

}
