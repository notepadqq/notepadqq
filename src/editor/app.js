var editor;
var generation;

UiDriver.registerEventHandler("C_CMD_SET_VALUE", function(msg, data, prevReturn) {
    editor.setValue(data);
});

UiDriver.registerEventHandler("C_FUN_GET_VALUE", function(msg, data, prevReturn) {
    return editor.getValue("\n");
});

UiDriver.registerEventHandler("C_CMD_MARK_CLEAN", function(msg, data, prevReturn) {
    generation = editor.changeGeneration(true);
    UiDriver.sendMessage("J_EVT_CLEAN_CHANGED", editor.isClean(generation));
});

UiDriver.registerEventHandler("C_FUN_IS_CLEAN", function(msg, data, prevReturn) {
    return editor.isClean(generation);
});

UiDriver.registerEventHandler("C_CMD_SET_LANGUAGE", function(msg, data, prevReturn) {
    console.log("setting " + data);
    editor.setOption("mode", data);
});

UiDriver.registerEventHandler("C_FUN_GET_SELECTIONS_TEXT", function(msg, data, prevReturn) {
    return editor.getSelections("\n");
});

UiDriver.registerEventHandler("C_CMD_SET_SELECTIONS_TEXT", function(msg, data, prevReturn) {
    var dataLines = data.split("\n");
    var selectedLines = editor.getSelections("\n");

    if (dataLines.length == selectedLines.length)
        editor.replaceSelections(dataLines);
    else
        editor.replaceSelection(data);
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

/* Search with a specified regex. Automatically select the text when found.
   The return value indicates whether a match was found. 
   The return value is the array returned by the regex match method, in case you
   want to extract matched groups.

   data is an array. data[0] contains the regex string. data[1] is a boolean
   (true if you want to search forward).
*/
function Search(data) {
    var forward = data[1];
    var startPos;
    
    // Avoid getting stuck finding always the same text
    if (forward)
        startPos = editor.getCursor("to");
    else
        startPos = editor.getCursor("from");
    
    // We get a new cursor every time, because the user could have moved within
    // the editor and we want to start searching from the new position.
    var searchCursor = editor.getSearchCursor(new RegExp(data[0]), startPos, false);

    var ret = forward ? searchCursor.findNext() : searchCursor.findPrevious();
    if (ret) {
        if (forward)
             editor.setSelection(searchCursor.from(), searchCursor.to()); 
        else
             editor.setSelection(searchCursor.to(), searchCursor.from());
    }

    return ret;
}
UiDriver.registerEventHandler("C_FUN_SEARCH", function(msg, data, prevReturn) {
    return Search(data);
});

/* Replace the currently selected text, then search with a specified regex (calls C_FUN_SEARCH)

   The return value indicates whether a match was found. 
   The return value is the array returned by the regex match method, in case you
   want to extract matched groups.

   data[0]: contains the regex string
   data[1]: string to use as replacement
   data[2]: true if you want to search forward, false if backwards
*/
UiDriver.registerEventHandler("C_FUN_REPLACE", function(msg, data, prevReturn) {
	if (editor.somethingSelected()) {
		// Replace
		editor.replaceSelection(data[1]);
	}
	
	// Find next/prev
	return Search([data[0], data[2]]);
});

UiDriver.registerEventHandler("C_FUN_REPLACE_ALL", function(msg, data, prevReturn) { 
    var dataLines = data[1].split("\n");
    
    var searchCursor = editor.getSearchCursor(new RegExp(data[0]), undefined, false);
    
    var count = 0;
    
    while (searchCursor.findNext()) {
    	count++;
    	
		// Replace
		searchCursor.replace(data[1]);
    }

    return count;
});

UiDriver.registerEventHandler("C_FUN_SEARCH_SELECT_ALL", function(msg, data, prevReturn) { 
	var searchCursor = editor.getSearchCursor(new RegExp(data[0]), undefined, false);
    
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
    var invertedMap = {};
    var mimes = CodeMirror.mimeModes;
    
    // Transform { text/css: 'css', text/x-css: 'css', text/php: { xxx: yy, name: 'php'}, ... }
    // to { css: ['text/css', 'text/x-css'], php: ['text/php'], ... }

    for (var mime in mimes) {
        if(mimes.hasOwnProperty(mime)) {
            // If mimes[mime] is an object insead of a string, use
            // the 'name' field as key instead of the object itself
            var key = mimes[mime].name || mimes[mime]

            if (key != "null") { // Exclude text/plain

                if (invertedMap[key] === undefined)
                    invertedMap[key] = [];

                invertedMap[key].push(mime);
            }
        }
    }
    
    return invertedMap;
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
    });

    generation = editor.changeGeneration(true);

    editor.on("change", function(instance, changeObj) {
        UiDriver.sendMessage("J_EVT_CONTENT_CHANGED");
        UiDriver.sendMessage("J_EVT_CLEAN_CHANGED", editor.isClean());
    });

    editor.on("cursorActivity", function(instance, changeObj) {
        UiDriver.sendMessage("J_EVT_CURSOR_ACTIVITY");
    });

    editor.on("focus", function() {
        UiDriver.sendMessage("J_EVT_GOT_FOCUS");
    });

    UiDriver.sendMessage("J_EVT_READY", null);
});
