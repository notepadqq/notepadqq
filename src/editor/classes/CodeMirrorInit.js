"use strict";
var editor;
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
    editor.on("change", function(instance, changeObj) {
        App.eventHook.onChange();
    });

    editor.on("scroll", function(instance) {
        App.eventHook.onScroll();
    });

    editor.on("cursorActivity", function(instance, changeObj) {
        App.eventHook.onCursorActivity();
    });

    editor.on("focus", function(instance) {
        App.eventHook.onFocus();
    });
    editor.on("optionChange", function() {
        App.eventHook.onOptionChange();
    });

    App.proxy.setReady();

});
