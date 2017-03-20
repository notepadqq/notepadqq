class EditorEventHandler {
    constructor() {
        this.lastCleanStatus = undefined;
        this.cursorActivityTimer = setTimeout(function() {},1);
    }

    cursorActivityObject(editor) {
        var sel = editor.getSelection("\n");
        var selLength = sel.length;
        var selLines = (sel.match(/\n/g)||[]).length;
        var cursor = editor.getCursor();
        var UiCursorInfo = {
            cursorLine: cursor.line,
            cursorColumn: cursor.ch,
            selectionCharCount: selLength,
            selectionLineCount: selLines
        }
        return UiCursorInfo;
    }

    changeActivityObject(editor) {
        var UiChangeInfo = {
            charCount: editor.getValue("\n").length,
            lineCount: editor.lineCount()
        }
        return UiChangeInfo;
    }

    cleanActivityObject(editor) {
        var clean = isCleanOrForced(changeGeneration);
        this.lastCleanStatus = clean;
        return clean;
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

    onLanguageChange(proxy, editor, langData) {
        console.error(JSON.stringify(langData));
        var lang = { id:langData.id, name:langData.name};
        console.error(JSON.stringify(lang));
        proxy.sendEditorEvent("J_EVT_CURRENT_LANGUAGE_CHANGED", lang);
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
        clearTimeout(this.cursorActivityTimer);
        // We put this in a small timer so we don't flood the queue
        // during selection and fast cursor movement.
        this.cursorActivityTimer = setTimeout(() => {
            proxy.sendEditorEvent("J_EVT_CURSOR_ACTIVITY", this.cursorActivityObject(editor));
        }, 20);
    }

    onFocus(proxy, editor) {

    }

    onChange(proxy, editor) {
        proxy.sendEditorEvent("J_EVT_CONTENT_CHANGED", this.changeActivityObject(editor));
        if (this.lastCleanStatus != isCleanOrForced(changeGeneration)) {
            proxy.sendEditorEvent("J_EVT_CLEAN_CHANGED", this.cleanActivityObject(editor));
        }
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
