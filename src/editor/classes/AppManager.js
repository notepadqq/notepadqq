"use strict";
/**
 * @brief This is our controller class.  All calls are made through here
 *        so we have a consistent interface for javascript communication.
 *
 * You can find extended documentation on the individual components in
 * the following files:
 * App.proxy.function(): CommunicationsManager.js
 * App.events.function(): EditorEventHooks.js
 * App.content.function(): ContentManager.js
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
        this.loadLink = undefined;

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
        $(document).ready(() => {this.initializeCodeMirror()});
    }

    setEditorStyleSheet(sheet)
    {
        this.loadLink = document.getElementById('pagestyle')
        // This logic probably doesn't belong here, but its pretty reliable
        // this way.
        this.loadLink.setAttribute('href', sheet);
        $(this.loadLink).ready(function() {
            let edwait = setInterval(function() {
                if (editor != undefined) {
                    let colour = getBackgroundColor(editor.getOption('theme'));
                    App.proxy.optionChangedEvent('theme',
                        getBackgroundColor(editor.getOption('theme')));
                    clearInterval(edwait);
                }
            }, 10);
        });
    }

    initializeCodeMirror()
    {
        let preferredTheme = this.defaultTheme;
        editor = CodeMirror($(".editor")[0], {
            lineNumbers: true,
            mode: "plaintext",
            highlightSelectionMatches: {
                style: "selectedHighlight", 
                wordsOnly: true, 
                delay: 25
            },
            styleSelectedText: true,
            styleActiveLine: true,
            foldGutter: true,
            gutters: [
                "CodeMirror-linenumbers", 
                "CodeMirror-foldgutter"
            ],
            indentWithTabs: true,
            indentUnit: 4,
            tabSize: 4,
            matchBrackets: true,
            extraKeys: {
                "Ctrl-Space": "autocomplete"
            },
            theme: preferredTheme
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

        let ev = this.events;
        editor.on('change', function() {ev.onChange()});
        editor.on('scroll', function() {ev.onScroll()});
        editor.on('cursorActivity', function() {ev.onCursorActivity()});
        editor.on('focus', function() {ev.onFocus()});
        App.proxy.setReady();
    }
}

var App = new AppManager();
