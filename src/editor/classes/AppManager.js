"use strict";
/**
 * @brief This is our controller class.  All calls are made through here
 *        so we have a consistent interface for javascript communication.
 * 
 * Some brief examples of how to use this:
 * App.proxy.sendEvent(Message, Data);
 * App.eventHook.onChange();
 *
 */
var editor;
class AppManager {
    constructor()
    {
        // Sub-function objects
        this.proxy = new CommunicationsManager();
        this.events = new EditorEventHook();
        this.content = new ContentManager();

        //Variables
        this.defaultTheme = "default";

        function getQueryValue(name) 
        {
            name = name.replace(/[\[]/, "\\[").replace(/[\]]/, "\\]");
            let regex = new RegExp("[\\?&]" + name + "=([^&#]*)"),
                results = regex.exec(location.search);
            if (results !== null) {
                return decodeURIComponent(results[1].replace(/\+/g, " "));
            }
            return "";
        }

        //Try to get the current preferred theme.
        let themePath = getQueryValue("themePath");
        let themeName = getQueryValue("themeName");
        if (themePath !== "") {
            this.setEditorStyleSheet(themePath);
            this.defaultTheme = themeName;
        }
    }

    setEditorStyleSheet(sheet)
    {
        document.getElementById('pagestyle').setAttribute('href', sheet);
    }

    initializeCodeMirror()
    {
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
            theme: App.defaultTheme
        });
    
        editor.addKeyMap({
            "Tab": function (cm) {
                if (cm.somethingSelected()) {
                    var sel = editor.getSelection("\n");
                    // Indent only if there are multiple lines selected, 
                    // or if the selection spans a full line
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
            App.events.onChange();
        });

        editor.on("scroll", function(instance) {
            App.events.onScroll();
        });

        editor.on("cursorActivity", function(instance, changeObj) {
            App.events.onCursorActivity();
        });

        editor.on("focus", function(instance) {
            App.events.onFocus();
        });
        editor.on("optionChange", function() {
            App.events.onOptionChange();
        });

        App.proxy.setReady();
        setTimeout(function() {
            editor.focus();
        }, 100);
    }
}

var App = new AppManager();
$(document).ready(function() {
    App.initializeCodeMirror();
});

