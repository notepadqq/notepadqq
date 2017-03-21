"use strict";
/*
 * @brief This class defines all of our message handlers for the
 *        CommunicationsManager class.... It's automatically created
 *        when a CommunicationsManager instance is made.
 */
class MessageHandler {
    constructor() 
    {

    }

    C_CMD_SET_VALUE(data) 
    {
        editor.setValue(data);
        App.eventHook.onSetValue();
    }

    C_FUN_GET_VALUE(data) 
    {
        return editor.getValue("\n");
    }

    C_CMD_MARK_CLEAN(data) 
    {
        App.state.setForceDirty(false);
        App.state.setChangeGeneration(editor.changeGeneration(true));
        App.eventHook.onChange();
    }

    C_CMD_MARK_DIRTY(data) 
    {
        App.state.setForceDirty(true);
        App.eventHook.onChange();
    }

    C_CMD_SET_LANGUAGE(data) 
    {
        Languages.setLanguage(editor, data);
    }

    C_CMD_SET_INDENTATION_MODE(data) {
        editor.setOption("indentWithTabs", data.useTabs);
        if (data.size !== undefined && data.size > 0) {
            editor.setOption("indentUnit", data.size);
            editor.setOption("tabSize", data.size);
        }

        editor.refresh();
    }

    C_FUN_GET_INDENTATION_MODE(data) 
    {
        return { useTabs: editor.options.indentWithTabs, size: editor.options.indentUnit };
    }

    C_FUN_GET_SELECTIONS_TEXT(data) {
        return editor.getSelections("\n");
    }

/* Replace the current selection with the provided array of strings.
   If the length of the array doesn't match the number of the current selections,
   the array will be joined by a newline and every selection will be replaced with the
   content of the joined string.

   data: {
        text: array of strings
        select: string used to specify where to place the cursor after the
                selection has been replaced. Possible values are "before",
                "after", or "selected".
   }
*/
    C_CMD_SET_SELECTIONS_TEXT(data) 
    {
        var dataSelections = data.text;
        var selectedLines = editor.getSelections("\n");

        var selectMode = undefined;
        if (data.select === "before") selectMode = "start";
        else if (data.select === "selected") selectMode = "around";

        if (dataSelections.length == selectedLines.length)
            editor.replaceSelections(dataSelections, selectMode);
        else
            editor.replaceSelection(dataSelections.join("\n"), selectMode);
    }

    C_CMD_SET_SELECTION(data) 
    {
        editor.setSelection(
            {
                line: data[0],
                ch: data[1]
            },
            {
                line: data[2],
                ch: data[3]
            }
        );
    }

    C_CMD_SET_CURSOR(data) 
    {
        var line = data[0];
        var ch = data[1];
        editor.setCursor(line, ch);
    }

    C_CMD_REQUEST_CURSOR_INFO(data) 
    {
        App.eventHook.onCursorActivity();
    }

    C_CMD_REQUEST_DOCUMENT_INFO(data) 
    {
        App.eventHook.onChange();
    }

    C_CMD_SET_SCROLL_POS(data) 
    {
        var left = data[0];
        var top = data[1];
        editor.scrollTo(left, top);
    }

    C_CMD_SELECT_ALL(data) 
    {
        editor.execCommand("selectAll");
    }

    C_CMD_UNDO(data) 
    {
        editor.undo();
    }

    C_CMD_REDO(data) 
    {
        editor.redo();
    }

    C_CMD_CLEAR_HISTORY(data) 
    {
        editor.clearHistory();
    }

    C_CMD_SET_LINE_WRAP(data) 
    {
        editor.setOption("lineWrapping", data == true);
    }

    C_CMD_SHOW_END_OF_LINE(data) 
    {
        editor.setOption("showEOL", !!data);
        editor.refresh();
    }

    C_CMD_SHOW_WHITESPACE(data) 
    {
        editor.setOption("showWhitespace", !!data);
        editor.refresh();
    }

    C_CMD_SET_TABS_VISIBLE(data) 
    {
        editor.setOption("showTab", !!data);
        editor.refresh();
    }

/*
   data[0]: contains the regex string
   data[1]: contains the regex modifiers (e.g. "ig")
   data[2]: true if you want to search forward, false if backwards
*/
    C_FUN_SEARCH(data) 
    {
        return App.helpers.Search(data[0], data[1], data[2]);
    }

/* Replace the currently selected text, then search with a specified regex (calls C_FUN_SEARCH)

   The return value indicates whether a match was found.
   The return value is the array returned by the regex match method, in case you
   want to extract matched groups.

   data[0]: contains the regex string
   data[1]: contains the regex modifiers (e.g. "ig")
   data[2]: true if you want to search forward, false if backwards
   data[3]: string to use as replacement
*/
    C_FUN_REPLACE(data) 
    {
        var regexStr = data[0];
        var regexModifiers = data[1];
        var forward = data[2];
        var searchMode = Number(data[4]);
        if (editor.somethingSelected()) {
            var replacement = data[3];
            // Replace
            if (searchMode == App.helpers.SearchMode.Regex && App.helpers.hasGroupReuseTokens(replacement)) {
                var searchRegex = new RegExp(regexStr, regexModifiers);
                groups = searchRegex.exec(editor.getSelection())
                if (groups !== null) { //groups === null should never occur!
                    editor.replaceSelection(App.helpers.applyReusedGroups(replacement,groups));
                }
            } else {
                editor.replaceSelection(replacement);
            }
        }

        // Find next/prev
        return App.helpers.Search(regexStr, regexModifiers, forward);
    }

    C_FUN_REPLACE_ALL(data) {
        var regexStr = data[0];
        var regexModifiers = data[1];
        var replacement = data[2];
        var searchMode = Number(data[3]);
        var searchCursor = editor.getSearchCursor(new RegExp(regexStr, regexModifiers), undefined, false);

        var count = 0;
        var id = Math.round(Math.random() * 1000000) + "/" + Date.now();
    
        var hasReuseTokens = App.helpers.hasGroupReuseTokens(replacement) && searchMode == App.helpers.SearchMode.Regex;

        while (groups = searchCursor.findNext()) {
            count++;
            // Replace
            if (hasReuseTokens){
                searchCursor.replace(App.helpers.applyReusedGroups(replacement, groups), "*C_FUN_REPLACE_ALL" + id);
            } else {
                searchCursor.replace(replacement, "*C_FUN_REPLACE_ALL" + id);
            }        
        }

        return count;
    }

    C_FUN_SEARCH_SELECT_ALL(data) 
    {
        var regexStr = data[0];
        var regexModifiers = data[1];
        var searchCursor = editor.getSearchCursor(new RegExp(regexStr, regexModifiers), undefined, false);

        var count = 0;
        var selections = [];

        while (searchCursor.findNext()) {
            count++;
            selections.push({anchor: searchCursor.from(), head: searchCursor.to()});
        }

        editor.setSelections(selections);
        return count;
    }

    C_CMD_SET_THEME(data) 
    {
        var link = undefined;
        if (data.path != "") {
            var stylesheet = $("link[href='" + data.path + "']");
            if (stylesheet.length > 0) {
                // Stylesheet already exists, move it to the bottom
                stylesheet.appendTo('head');
            } else {
                // Add the stylesheet
                link = addStylesheet(data.path);
            }
        }
        editor.setOption("theme", data.name);
        setTimeout(function() {
            editor.setOption("fixedGutter", true);
            editor.setOption("fixedGutter", false);
            editor.refresh();
        }, 100);
    }

    C_CMD_SET_FONT(data) 
    {
        var fontSize = (data.size != "" && data.size > 0) ? ("font-size:" + (+data.size) + "px;") : "";
        var fontFamily = data.family ? ("font-family:'" + ('' + data.family).replace("'", "\\'") + "';") : "";
        var lineHeight = (data.lineHeight != "" && data.lineHeight > 0) ? ("line-height:" + (+data.lineHeight) + "em;") : "";
    
        var styleTag = document.getElementById('userFont');

        if (styleTag) {
            styleTag.innerHTML = "div.editor > .CodeMirror { " + fontFamily + fontSize + lineHeight + " }";
        } else {
            styleTag = document.createElement("style");
            styleTag.id = 'userFont';
            styleTag.innerHTML = "div.editor > .CodeMirror { " + fontFamily + fontSize + lineHeight + " }";
            document.getElementsByTagName("head")[0].appendChild(styleTag);
        }
    }

    C_CMD_SET_OVERWRITE(data) 
    {
        editor.toggleOverwrite(data);
    }

    C_CMD_SET_SMART_INDENT(data) 
    {
        editor.options.smartIndent = data;
        //return data;
    }

    C_CMD_SET_FOCUS(data) 
    {
        editor.focus();
    }

    C_CMD_BLUR(data) 
    {
        document.activeElement.blur();
    }

    C_CMD_DISPLAY_PRINT_STYLE(data) 
    {
        Printer.displayPrintStyle($(".editor")[0], editor);
    }

    C_CMD_DISPLAY_NORMAL_STYLE(data) 
    {
        Printer.displayNormalStyle($(".editor")[0], editor);
    }

/*
    Get the word under the (first) cursor head, or an empty string if there isn't one.
*/
    C_FUN_GET_CURRENT_WORD(data) 
    {
        var cur = editor.getCursor();
        var line = editor.getLine(cur.line);

        var strL = line.substring(0, cur.ch);
        var strR = line.substring(cur.ch);
        var regexL = /[^\w]?(\w*)$/;
        var regexR = /^(\w*)[^\w]?/;
        var matchL = regexL.exec(strL);
        var matchR = regexR.exec(strR);

        var word = "";
        if (matchL !== null) word += matchL[1];
        if (matchR !== null) word += matchR[1];

        return word;
    }

    C_CMD_DUPLICATE_LINE(data) 
    {
        var cur = editor.getCursor();
        var line = editor.getLine(cur.line);
        var pos = {line: cur.line, ch: line.length};
        editor.replaceRange('\n' + line, pos);
    }

    C_CMD_MOVE_LINE_UP(data) 
    {
        var cur = editor.getCursor();
    
        // check previous line is not beginning of the document
        if ((cur.line - 1) < 0) {
            return;
        }
    
        var line = editor.getLine(cur.line) + '\n' + editor.getLine(cur.line-1);
        var from = { line: cur.line - 1, ch: 0           };
        var to   = { line: cur.line,     ch: line.length };
    
        editor.replaceRange(line, from, to);
        editor.setCursor(cur.line - 1, cur.ch );
    }

    C_CMD_MOVE_LINE_DOWN(data) 
    {
        var cur = editor.getCursor();
    
        // check that next line is not past end of document
        if ((cur.line + 1) == editor.lineCount() )  {
            return;
        }
    
        var line = editor.getLine(cur.line + 1) + '\n' + editor.getLine(cur.line);
        var from = { line: cur.line,     ch: 0           };
        var to   = { line: cur.line + 1, ch: line.length };
    
        editor.replaceRange(line, from, to);
        editor.setCursor(cur.line + 1, cur.ch );
    }


    C_CMD_DELETE_LINE(data) 
    {
        editor.execCommand("deleteLine");
        editor.changeGeneration(true);
    }

    C_CMD_TRIM_LEADING_TRAILING_SPACE(data) 
    {
        editLines(function (x) { return x.trim(); });
    }

    C_CMD_TRIM_TRAILING_SPACE(data) 
    {
        editLines(function (x) { return x.replace(/\s+$/, ""); });
    }

    C_CMD_TRIM_LEADING_SPACE(data) 
    {
        editLines(function (x) { return x.replace(/^\s+/, ""); });
    }

    C_CMD_TAB_TO_SPACE(data) 
    {
        editLines(function (x) {
            tabToSpaceCounter = 0
            var tabSz = editor.getOption("tabSize")

            return x.replace(/\t/g, function(match, offset) {
                return tabToSpaceHelper(match, offset, tabSz)
            });

        });
    }

    C_CMD_SPACE_TO_TAB_ALL(data) 
    {
        editLines(function (x) {
            App.helpers.spaceToTabCounter = 0
            var tabSz = editor.getOption("tabSize")

            return x.replace(/ +/g, function(match, offset) {
                return App.helpers.spaceToTab(match, offset, tabSz)
            });

        });
    }

    C_CMD_SPACE_TO_TAB_LEADING(data) 
    {
        editLines(function (x) {
            App.helpers.spaceToTabCounter = 0
            var tabSz = editor.getOption("tabSize")

            return x.replace(/^ +/g, function(match, offset) {
                return App.helpers.spaceToTab(match, offset, tabSz)
            });

        });
    }

    C_CMD_EOL_TO_SPACE(data) 
    {
        var text = editor.getValue("\n");
        editor.setValue(text.replace(/\n/gm," "));
    }
}
