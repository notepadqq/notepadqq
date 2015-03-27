var editor;
var changeGeneration;
var forceDirty = false;

UiDriver.registerEventHandler("C_CMD_SET_VALUE", function(msg, data, prevReturn) {
    editor.setValue(data);
});

UiDriver.registerEventHandler("C_FUN_GET_VALUE", function(msg, data, prevReturn) {
    return editor.getValue("\n");
});

/* Returns true if the editor is clean, false if
   it's dirty or it's clean but forceDirty = true.
   You'll generally want to use this function instead of
   CodeMirror.isClean()
*/
function isCleanOrForced(generation) {
    return !forceDirty && editor.isClean(generation);
}

UiDriver.registerEventHandler("C_CMD_MARK_CLEAN", function(msg, data, prevReturn) {
    forceDirty = false;
    changeGeneration = editor.changeGeneration(true);
    UiDriver.sendMessage("J_EVT_CLEAN_CHANGED", isCleanOrForced(changeGeneration));
});

UiDriver.registerEventHandler("C_CMD_MARK_DIRTY", function(msg, data, prevReturn) {
    forceDirty = true;
    UiDriver.sendMessage("J_EVT_CLEAN_CHANGED", isCleanOrForced(changeGeneration));
});

UiDriver.registerEventHandler("C_FUN_IS_CLEAN", function(msg, data, prevReturn) {
    return isCleanOrForced(changeGeneration);
});

UiDriver.registerEventHandler("C_CMD_SET_LANGUAGE", function(msg, data, prevReturn) {
    Languages.setLanguage(editor, data);
});

/*
    Sets the language by finding the more appropriate one for the file name.
    E.g. path/to/worker.js => JavaScript
    If nothing can be inferred from the file name, it looks at the editor content to see if it
    can guess something (eg. files starting with #! are probably shell scripts).
*/
UiDriver.registerEventHandler("C_FUN_SET_LANGUAGE_FROM_FILENAME", function(msg, data, prevReturn) {
    var lang = Languages.languageByFileName(editor, data);
    Languages.setLanguage(editor, lang);
    return lang;
});

/* Returns the id of the current language, and its data */
UiDriver.registerEventHandler("C_FUN_GET_CURRENT_LANGUAGE", function(msg, data, prevReturn) {
    var langId = Languages.currentLanguage(editor);
    return {id: langId, lang: Languages.languages[langId]};
});

UiDriver.registerEventHandler("C_CMD_SET_INDENTATION_MODE", function(msg, data, prevReturn) {
    editor.setOption("indentWithTabs", data.useTabs);
    if (data.size !== undefined && data.size > 0) {
        editor.setOption("indentUnit", data.size);
        editor.setOption("tabSize", data.size);
    }

    editor.refresh();
});

UiDriver.registerEventHandler("C_FUN_GET_INDENTATION_MODE", function(msg, data, prevReturn) {
    return { useTabs: editor.options.indentWithTabs, size: editor.options.indentUnit };
});

UiDriver.registerEventHandler("C_FUN_GET_SELECTIONS_TEXT", function(msg, data, prevReturn) {
    return editor.getSelections("\n");
});

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
UiDriver.registerEventHandler("C_CMD_SET_SELECTIONS_TEXT", function(msg, data, prevReturn) {
    var dataSelections = data.text;
    var selectedLines = editor.getSelections("\n");

    var selectMode = undefined;
    if (data.select === "before") selectMode = "start";
    else if (data.select === "selected") selectMode = "around";

    if (dataSelections.length == selectedLines.length)
        editor.replaceSelections(dataSelections, selectMode);
    else
        editor.replaceSelection(dataSelections.join("\n"), selectMode);
});

/*
    Retrieves a list of all current selections.
    These will always be sorted, and never overlap (overlapping selections
    are merged). Each object in the array contains anchor and head properties
    referring to {line, ch} objects.
*/
UiDriver.registerEventHandler("C_FUN_GET_SELECTIONS", function(msg, data, prevReturn) {
    var out = [];
    var sels = editor.listSelections();
    for (var i = 0; i < sels.length; i++) {
        out[i] = { anchor: {
                     line: sels[i].anchor.line,
                     ch: sels[i].anchor.ch
                   },
                   head: {
                     line: sels[i].head.line,
                     ch: sels[i].head.ch
                   }
                 };
    }
    return out;
});

UiDriver.registerEventHandler("C_CMD_SET_SELECTION", function(msg, data, prevReturn) {
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
});

UiDriver.registerEventHandler("C_FUN_GET_TEXT_LENGTH", function(msg, data, prevReturn) {
    return editor.getValue("\n").length;
});

UiDriver.registerEventHandler("C_FUN_GET_LINE_COUNT", function(msg, data, prevReturn) {
    return editor.lineCount();
});

UiDriver.registerEventHandler("C_FUN_GET_CURSOR", function(msg, data, prevReturn) {
    var cur = editor.getCursor();
    return [cur.line, cur.ch];
});

UiDriver.registerEventHandler("C_CMD_SET_CURSOR", function(msg, data, prevReturn) {
    var line = data[0];
    var ch = data[1];
    editor.setCursor(line, ch);
});

UiDriver.registerEventHandler("C_FUN_GET_SCROLL_POS", function(msg, data, prevReturn) {
    var scroll = editor.getScrollInfo();
    return [scroll.left, scroll.top];
});

UiDriver.registerEventHandler("C_CMD_SET_SCROLL_POS", function(msg, data, prevReturn) {
    var left = data[0];
    var top = data[1];
    editor.scrollTo(left, top);
});

UiDriver.registerEventHandler("C_CMD_SELECT_ALL", function(msg, data, prevReturn) {
    editor.execCommand("selectAll");
});

UiDriver.registerEventHandler("C_CMD_UNDO", function(msg, data, prevReturn) {
    editor.undo();
});

UiDriver.registerEventHandler("C_CMD_REDO", function(msg, data, prevReturn) {
    editor.redo();
});

UiDriver.registerEventHandler("C_CMD_CLEAR_HISTORY", function(msg, data, prevReturn) {
    editor.clearHistory();
});

UiDriver.registerEventHandler("C_CMD_SET_LINE_WRAP", function(msg, data, prevReturn) {
    editor.setOption("lineWrapping", data == true);
});

UiDriver.registerEventHandler("C_CMD_SET_TABS_VISIBLE", function(msg, data, prevReturn) {
    if (data) {
        $(".editor").addClass("show-tabs");
    } else {
        $(".editor").removeClass("show-tabs");
    }
});

/* Search with a specified regex. Automatically select the text when found.
   The return value indicates whether a match was found.
   The return value is the array returned by the regex match method, in case you
   want to extract matched groups.

   regexStr: contains the regex string
   regexModifiers: contains the regex modifiers (e.g. "ig")
   forward: true if you want to search forward, false if backwards
*/
function Search(regexStr, regexModifiers, forward) {
    var startPos;

    // Avoid getting stuck finding always the same text
    if (forward)
        startPos = editor.getCursor("to");
    else
        startPos = editor.getCursor("from");

    // We get a new cursor every time, because the user could have moved within
    // the editor and we want to start searching from the new position.
    var searchRegex = new RegExp(regexStr, regexModifiers);
    var searchCursor = editor.getSearchCursor(searchRegex, startPos, false);

    var ret = forward ? searchCursor.findNext() : searchCursor.findPrevious();

    if (!ret) {
        // Maybe the end was reached. Try again from the start.
        if (forward) {
            searchCursor = editor.getSearchCursor(searchRegex, null, false);
        } else {
            var line = editor.lineCount() - 1;
            var ch = editor.getLine(line).length;
            searchCursor = editor.getSearchCursor(searchRegex, {line: line, ch: ch}, false);
        }

        ret = forward ? searchCursor.findNext() : searchCursor.findPrevious();
    }

    if (ret) {
        if (forward)
             editor.setSelection(searchCursor.from(), searchCursor.to());
        else
             editor.setSelection(searchCursor.to(), searchCursor.from());
    }

    return ret;
}

/*
   data[0]: contains the regex string
   data[1]: contains the regex modifiers (e.g. "ig")
   data[2]: true if you want to search forward, false if backwards
*/
UiDriver.registerEventHandler("C_FUN_SEARCH", function(msg, data, prevReturn) {
    return Search(data[0], data[1], data[2]);
});

/* Replace the currently selected text, then search with a specified regex (calls C_FUN_SEARCH)

   The return value indicates whether a match was found.
   The return value is the array returned by the regex match method, in case you
   want to extract matched groups.

   data[0]: contains the regex string
   data[1]: contains the regex modifiers (e.g. "ig")
   data[2]: true if you want to search forward, false if backwards
   data[3]: string to use as replacement
*/
UiDriver.registerEventHandler("C_FUN_REPLACE", function(msg, data, prevReturn) {
    if (editor.somethingSelected()) {
        // Replace
        editor.replaceSelection(data[3]);
    }

    // Find next/prev
    return Search(data[0], data[1], data[2]);
});

UiDriver.registerEventHandler("C_FUN_REPLACE_ALL", function(msg, data, prevReturn) {
    var regexStr = data[0];
    var regexModifiers = data[1];
    var replacement = data[2];

    var searchCursor = editor.getSearchCursor(new RegExp(regexStr, regexModifiers), undefined, false);

    var count = 0;
    var id = Math.round(Math.random() * 1000000) + "/" + Date.now();

    while (searchCursor.findNext()) {
        count++;

        // Replace
        searchCursor.replace(replacement, "*C_FUN_REPLACE_ALL" + id);
    }

    return count;
});

UiDriver.registerEventHandler("C_FUN_SEARCH_SELECT_ALL", function(msg, data, prevReturn) {
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
});

UiDriver.registerEventHandler("C_FUN_GET_LANGUAGES", function(msg, data, prevReturn) {
    return Languages.languages;
});

UiDriver.registerEventHandler("C_CMD_SET_THEME", function(msg, data, prevReturn) {
    if (data.path != "") {
        var stylesheet = $("link[href='" + data.path + "']");
        if (stylesheet.length > 0) {
            // Stylesheet already exists, move it to the bottom
            stylesheet.appendTo('head');
        } else {
            // Add the stylesheet
            addStylesheet(data.path);
        }
    }

    editor.setOption("theme", data.name);
});

UiDriver.registerEventHandler("C_CMD_SET_OVERWRITE", function(msg, data, prevReturn) {
    editor.toggleOverwrite(data);
});

UiDriver.registerEventHandler("C_CMD_SET_FOCUS", function(msg, data, prevReturn) {
    editor.focus();
});

UiDriver.registerEventHandler("C_FUN_DETECT_INDENTATION_MODE", function(msg, data, prevReturn) {
    var len = editor.lineCount();
    var regexIndented = /^([ ]{2,}|[\t]+)[^ \t]+?/g; // Is not blank, and is indented with tab or space

    for (var i = 0; i < len && i < 100; i++) {
        var line = editor.getLine(i);
        var matches = regexIndented.exec(line);
        if (matches !== null) {
            if (line[0] === "\t") { // Is a tab
                return {found: true, useTabs: true, size: 0};
            } else { // Is a space
                var size = matches[1].length;
                if (size === 2 || size === 4 || size === 8) {
                    return {found: true, useTabs: false, size: size};
                } else {
                    return {found: false};
                }
            }
        }
    }

    return {found: false};
});

UiDriver.registerEventHandler("C_CMD_DISPLAY_PRINT_STYLE", function(msg, data, prevReturn) {
    Printer.displayPrintStyle($(".editor")[0], editor);
});

UiDriver.registerEventHandler("C_CMD_DISPLAY_NORMAL_STYLE", function(msg, data, prevReturn) {
    Printer.displayNormalStyle($(".editor")[0], editor);
});

/*
    Get the word under the (first) cursor head, or an empty string if there isn't one.
*/
UiDriver.registerEventHandler("C_FUN_GET_CURRENT_WORD", function(msg, data, prevReturn) {
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
});

UiDriver.registerEventHandler("C_CMD_DUPLICATE_LINE", function(msg, data, prevReturn) {
    var cur = editor.getCursor();
    var line = editor.getLine(cur.line);
    var pos = {line: cur.line, ch: line.length};
    editor.replaceRange('\n' + line, pos);
});

UiDriver.registerEventHandler("C_CMD_DELETE_LINE", function(msg, data, prevReturn) {
    editor.execCommand("deleteLine");
    editor.changeGeneration(true);
});

UiDriver.registerEventHandler("C_CMD_TRIM_LEADING_TRAILING_SPACE", function(msg, data, prevReturn) {
    editLines(function (x) { return x.trim(); });
});

UiDriver.registerEventHandler("C_CMD_TRIM_TRAILING_SPACE", function(msg, data, prevReturn) {
    editLines(function (x) { return x.replace(/\s+$/, ""); });
});

UiDriver.registerEventHandler("C_CMD_TRIM_LEADING_SPACE", function(msg, data, prevReturn) {
    editLines(function (x) { return x.replace(/^\s+/, ""); });
});

UiDriver.registerEventHandler("C_CMD_TAB_TO_SPACE", function(msg, data, prevReturn) {
    editLines(function (x) { return x.replace(/\t/g, " "); });
});

UiDriver.registerEventHandler("C_CMD_SPACE_TO_TAB_ALL", function(msg, data, prevReturn) {
    editLines(function (x) { return x.replace(/ /g, "\t"); });
});

UiDriver.registerEventHandler("C_CMD_SPACE_TO_TAB_LEADING", function(msg, data, prevReturn) {
    editLines(function (x) { return x.replace(/^\s+/g, function(m){ return m.replace(/\s/g, "\t");}); });
});

function editLines(funct){
    editor.operation(function(){
        var len = editor.lineCount();
        for (var i = 0; i < len; i++) {
            var line = editor.getLine(i);
            var from = {line: i, ch: 0};
            var to = {line: i, ch: line.length};
            editor.replaceRange(funct(line), from,to);
        }
    });
}

UiDriver.registerEventHandler("C_CMD_EOL_TO_SPACE", function(msg, data, prevReturn) {
    var text = editor.getValue("\n");
    editor.setValue(text.replace(/\n/gm," "));
});

$(document).ready(function () {
    editor = CodeMirror($(".editor")[0], {
        lineNumbers: true,
        mode: { name: "" },
        highlightSelectionMatches: {style: "selectedHighlight", wordsOnly: true, delay: 25},
        styleSelectedText: true,
        styleActiveLine: true,
        foldGutter: true,
        gutters: ["CodeMirror-linenumbers", "CodeMirror-foldgutter"],
        indentWithTabs: true,
        indentUnit: 4,
        tabSize: 4,
        matchBrackets: true,
        extraKeys: {"Ctrl-Space": "autocomplete"},
        theme: _defaultTheme
    });

    editor.addKeyMap({
        "Tab": function (cm) {
            if (cm.somethingSelected()) {
                var sel = editor.getSelection("\n");
                // Indent only if there are multiple lines selected, or if the selection spans a full line
                if (sel.length > 0 && (sel.indexOf("\n") > -1 || sel.length === cm.getLine(cm.getCursor().line).length)) {
                    cm.indentSelection("add");
                    return;
                }
            }

            if (cm.options.indentWithTabs)
                cm.execCommand("insertTab");
            else
                cm.execCommand("insertSoftTab");
        },
        "Shift-Tab": function (cm) {
            cm.indentSelection("subtract");
        }
    });

    changeGeneration = editor.changeGeneration(true);

    editor.on("change", function(instance, changeObj) {
        UiDriver.sendMessage("J_EVT_CONTENT_CHANGED");
        UiDriver.sendMessage("J_EVT_CLEAN_CHANGED", isCleanOrForced(changeGeneration));
    });

    editor.on("cursorActivity", function(instance, changeObj) {
        UiDriver.sendMessage("J_EVT_CURSOR_ACTIVITY");
    });

    editor.on("focus", function() {
        UiDriver.sendMessage("J_EVT_GOT_FOCUS");
    });

    UiDriver.sendMessage("J_EVT_READY", null);
});
