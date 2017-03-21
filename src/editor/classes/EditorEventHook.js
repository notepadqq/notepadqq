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
        var detectedIndent = App.state.detectIndentationMode();
        editor.clearHistory();
        App.proxy.sendEvent("J_EVT_DOCUMENT_LOADED", detectedIndent);
    }

    onCursorActivity() 
    {
        clearTimeout(this.cursorActivityTimer);
        this.cursorActivityTimer = setTimeout(() => {
            App.proxy.sendEvent("J_EVT_CURSOR_ACTIVITY", this.cursorActivityObject());
        }, 20);
    }

    onFocus() 
    {

    }

    onChange() 
    {
        App.proxy.sendEvent("J_EVT_CONTENT_CHANGED", this.changeActivityObject());
        if (App.state.cleanStateChanged()) {
            App.proxy.sendEvent("J_EVT_CLEAN_CHANGED", App.state.isCleanOrForced(App.state.changeGeneration));
        }
    }

    onScroll() 
    {
        var scroll = editor.getScrollInfo();
        App.proxy.sendEvent("J_EVT_SCROLL_CHANGED", [scroll.left, scroll.top]);
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
