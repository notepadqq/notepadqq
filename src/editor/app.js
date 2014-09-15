var editor;
var changeGeneration;

UiDriver.registerEventHandler("C_CMD_SET_VALUE", function(msg, data, prevReturn) {
    editor.setValue(data);
});

UiDriver.registerEventHandler("C_FUN_GET_VALUE", function(msg, data, prevReturn) {
    return editor.getValue("\n");
});

UiDriver.registerEventHandler("C_CMD_MARK_CLEAN", function(msg, data, prevReturn) {
    changeGeneration = editor.changeGeneration(true);
    UiDriver.sendMessage("J_EVT_CLEAN_CHANGED", editor.isClean(changeGeneration));
});

UiDriver.registerEventHandler("C_FUN_IS_CLEAN", function(msg, data, prevReturn) {
    return editor.isClean(changeGeneration);
});

UiDriver.registerEventHandler("C_CMD_SET_LANGUAGE", function(msg, data, prevReturn) {
    Languages.setLanguage(editor, data);
});

UiDriver.registerEventHandler("C_FUN_SET_LANGUAGE_FROM_FILENAME", function(msg, data, prevReturn) {
    var lang = Languages.languageByFileName(data);
    Languages.setLanguage(editor, lang);
    return lang;
});

/* Returns the id of the current language, and its data */
UiDriver.registerEventHandler("C_FUN_GET_CURRENT_LANGUAGE", function(msg, data, prevReturn) {
    var langId = Languages.currentLanguage(editor);
    return {id: langId, lang: Languages.languages[langId]};
});

UiDriver.registerEventHandler("C_CMD_SET_INDENTATION_MODE", function(msg, data, prevReturn) {
    editor.options.indentWithTabs = data.useTabs;
    editor.options.indentUnit = data.size;
    editor.options.tabSize = data.size;
    editor.refresh();
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
    var searchCursor = editor.getSearchCursor(new RegExp(regexStr, regexModifiers), startPos, false);

    var ret = forward ? searchCursor.findNext() : searchCursor.findPrevious();
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

    while (searchCursor.findNext()) {
        count++;

        // Replace
        searchCursor.replace(replacement);
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


$(document).ready(function () {
    editor = CodeMirror($(".editor")[0], {
        autofocus: true,
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
        matchBrackets: true
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
        UiDriver.sendMessage("J_EVT_CLEAN_CHANGED", editor.isClean(changeGeneration));
    });

    editor.on("cursorActivity", function(instance, changeObj) {
        UiDriver.sendMessage("J_EVT_CURSOR_ACTIVITY");
    });

    editor.on("focus", function() {
        UiDriver.sendMessage("J_EVT_GOT_FOCUS");
    });

    UiDriver.sendMessage("J_EVT_READY", null);
});
