"use strict";
/**
 * @brief These are our CodeMirror event hooks.  We place these as necessary
 *        in order to keep Notepadqq synchronized with CodeMirror.
 */
class EditorEventHook {
    constructor() 
    {
        this.cursorActivityTimer = setTimeout(function() {},1);
    }

    cursorActivityObject() 
    {
        var sel = editor.getSelection("\n");
        var selLength = sel.length;
        var selLines = (sel.match(/\n/g)||[]).length;
        var cursor = editor.getCursor();
        var UiCursorInfo = {
            cursorLine: cursor.line,
            cursorColumn: cursor.ch,
            selectionCharCount: selLength,
            selectionLineCount: selLines,
            selections: editor.listSelections()
        }
        return UiCursorInfo;
    }

    changeActivityObject() 
    {
        var UiChangeInfo = {
            charCount: editor.getValue("\n").length,
            lineCount: editor.lineCount()
        }
        return UiChangeInfo;
    }

    onOptionChange() 
    {
    }

    onSetValue() 
    {
        var detectedIndent = App.content.detectIndentationMode();
        editor.clearHistory();
        App.proxy.documentLoadedEvent(detectedIndent);
    }

    onCursorActivity() 
    {
        clearTimeout(this.cursorActivityTimer);
        this.cursorActivityTimer = setTimeout(() => {
            App.proxy.cursorActivityEvent(this.cursorActivityObject());
        }, 20);
    }

    onFocus() 
    {
        App.proxy.focusChangedEvent();
    }

    onChange() 
    {
        App.proxy.contentChangedEvent(this.changeActivityObject());
        if (App.content.cleanStateChanged()) {
            App.proxy.cleanChangedEvent(App.content.isCleanOrForced(App.content.changeGeneration));
        }
    }

    onScroll() 
    {
        var scroll = editor.getScrollInfo();
        App.proxy.scrollChangedEvent([scroll.left, scroll.top]);
    }

    onLoad() 
    {
        this.onCursorActivity();
        this.onChange();
        this.onScroll();
        this.onOptionChange();
        editor.refresh();
    }
}
