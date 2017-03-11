class EditorEventHandler {
    constructor() {

    }

    detectIndentationMode(editor) {
        var len = editor.lineCount();
        var regexIndented = /^([ ]{2,}|[\t]+)[^ \t]+?/g; // Is not blank, and is indented with tab or space

        for (var i = 0; i < len && i < 100; i++) {
            var line = editor.getLine(i);
            var matches = regexIndented.exec(line);
            if (matches !== null) {
                if (line[0] === "\t") { // Is a tab
                    return [1, 0];
                } else { // Is a space
                    var size = matches[1].length;
                    if (size === 2 || size === 4 || size === 8) {
                        return [0, size];
                    } else {
                        return undefined;
                    }
                }
            }
        }
        return undefined;
    }

    onOptionChange(proxy, editor) {
        var indentMode = [editor.options.indentWithTabs, editor.options.indentUnit];
        proxy.setValue("indentMode", indentMode);
    }

    onLanguageChange(proxy, editor) {
        var langId = Languages.currentLanguage(editor);
        var langData =  {id: langId, lang: Languages.languages[langId]};
        proxy.setValue("language", langData);
        proxy.sendEditorEvent("J_EVT_CURRENT_LANGUAGE_CHANGED", langData);
    }

    onSetValue(proxy, editor) {
        var detectedIndent = this.detectIndentationMode(editor);
        if (detectedIndent !== undefined) {
            proxy.setValue("detectedIndent", detectedIndent);
        }
        editor.clearHistory();
        proxy.sendEditorEvent("J_EVT_DOCUMENT_LOADED", 0);
    }

    onCursorActivity(proxy, editor) {
        var out = [];
        var sels = editor.listSelections();
        for (var i = 0; i < sels.length; i++) {
            out[i] = {  anchor: {
                            line: sels[i].anchor.line,
                            ch: sels[i].anchor.ch
                        },
                        head: {
                            line: sels[i].head.line,
                            ch: sels[i].head.ch
                        }
                    };
        }
        proxy.setValue("selections", out);
        proxy.setValue("selectionsText", editor.getSelections("\n"));
        var cur = editor.getCursor();
        proxy.setValue("cursor", [cur.line, cur.ch]);
    }

    onChange(proxy, editor) {
        proxy.setValue("charCount", editor.getValue("\n").length);
        proxy.setValue("lineCount", editor.lineCount());
        proxy.setValue("clean", isCleanOrForced(changeGeneration));
    }

    onScroll(proxy, editor) {
        var scroll = editor.getScrollInfo();
        proxy.setValue("scrollPosition", [scroll.left, scroll.top]);
    }

    onLoad(proxy, editor) {
        this.onCursorActivity(proxy, editor);
        this.onChange(proxy, editor);
        this.onScroll(proxy, editor);
        this.onOptionChange(proxy, editor);
        proxy.sendEditorEvent("J_EVT_READY", 0);
        editor.refresh();
    }
}
