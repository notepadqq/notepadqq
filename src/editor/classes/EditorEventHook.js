"use strict";
/**
 * @brief These are our CodeMirror event hooks.  We place these as necessary
 *        in order to keep Notepadqq synchronized with CodeMirror. Please note
 *        that these should RARELY need to be called outside of
 *        editor.on("event", () => {}); due to how CodeMirror works.
 *
 *        Best practice would be to test your function without using one of
 *        these first, then applying the appropriate one in your function
 *        if something isn't behaving as expected.
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

    /**
     * @brief Placeholder for future CodeMirror version update
     */
    onOptionChange() 
    {
    }

    /**
     * @brief Should be called every time C_CMD_SET_VALUE is called.  Prevents
     *        being able to erase the initially loaded document.
     */
    onSetValue() 
    {
        var detectedIndent = App.content.detectIndentationMode();
        editor.clearHistory();
        App.proxy.documentLoadedEvent(detectedIndent);
    }

    /**
     * @brief Should be called every time the CodeMirror cursor moves, as well
     *        as on document load to keep the CPP status UI in sync with the
     *        editor.
     */
    onCursorActivity() 
    {
        clearTimeout(this.cursorActivityTimer);
        this.cursorActivityTimer = setTimeout(() => {
            App.proxy.cursorActivityEvent(this.cursorActivityObject());
        }, 20);
    }

    /**
     * @brief Should be called every time the editor receives focus.
     */
    onFocus() 
    {
        App.proxy.focusChangedEvent();
    }

    /**
     * @brief Should be called every time the content of the editor is changed.
     */
    onChange() 
    {
        App.proxy.contentChangedEvent(this.changeActivityObject());
        if (App.content.cleanStateChanged()) {
            App.proxy.cleanChangedEvent(App.content.isCleanOrForced(App.content.changeGeneration));
        }
    }
    /**
     * @brief Should be called every time the CodeMirror view is scrolled.
     */
    onScroll() 
    {
        var scroll = editor.getScrollInfo();
        App.proxy.scrollChangedEvent([scroll.left, scroll.top]);
    }
}
