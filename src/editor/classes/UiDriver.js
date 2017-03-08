
var UiDriver = new function() {
    var handlers = [];

    var socket;
    var channel;
    var msgQueue = [];

    var _this = this;

    function usingQtWebChannel() {
        return (typeof cpp_ui_driver === 'undefined')
    }
    // UiDriver holds onto the JsProxy transport so we can access it elsewhere.
    this.proxy = undefined;


    // QWebChannel initialization starts here... as early as humanly possible.
    new QWebChannel(qt.webChannelTransport, function (_channel) {
        channel = _channel;

        _this.proxy = channel.objects.cpp_ui_driver;
        _this.proxy.sendMsg.connect(function(msg, data) {
            _this.messageReceived(msg, data);
        });

    }); 
    

    // Helper functions for hooks
    this.detectIndentationMode = function(editor) {
        if(this.proxy === undefined) {
            return;
        }
        var len = editor.lineCount();
        var regexIndented = /^([ ]{2,}|[\t]+)[^ \t]+?/g; // Is not blank, and is indented with tab or space

        for (var i = 0; i < len && i < 100; i++) {
            var line = editor.getLine(i);
            var matches = regexIndented.exec(line);
            if (matches !== null) {
                if (line[0] === "\t") { // Is a tab
                    return [1, 0];
                } else { // Is a space
                    var size = matches[1].length;
                    if (size === 2 || size === 4 || size === 8) {
                        return [0, size];
                    } else {
                        return;
                    }
                }
            }
        }
        return;

    }

    this.registerEventHandler = function(msg, handler) {
        if (handlers[msg] === undefined)
            handlers[msg] = [];
        handlers[msg].push(handler);
    }

    this.setReturnData = function(data) {
        this.proxy.result = data;
    }
    
    this.messageReceived = function(msg, data) {
        if (!usingQtWebChannel()) {
            console.error("Not using QtWebChannel: "+ msg);
        }

        // Only one of the handlers (the last that gets
        // called) can return a value. So, to each handler
        // we provide the previous handler's return value.
        var prevReturn = undefined;
        if (handlers[msg] !== undefined) {
            handlers[msg].forEach(function(handler) {
                prevReturn = handler(msg, data, prevReturn);
            });
        }
        
        if(prevReturn !== undefined) {
            this.setReturnData(prevReturn);
        }
        return prevReturn;
    }

    // BEGIN HOOKS
    // These functions hook into the CodeMirror interface and allow us to
    // perform asynchronous data transfer between C++ and Javascript.

    this.onOptionChange = function(editor) {
        var indentMode = [editor.options.indentWithTabs, editor.options.indentUnit];
        this.proxy.setValue("indentMode", indentMode);
    }

    // Hook for when the editor's language is changed.
    this.onLanguageChange = function(editor) {
        var langId = Languages.currentLanguage(editor);
        var langData =  {id: langId, lang: Languages.languages[langId]}; 
        this.proxy.language = langData;
        this.proxy.sendEditorEvent("J_EVT_CURRENT_LANGUAGE_CHANGED", langData);
    }

    // Hook for when we load a file/change content
    this.onSetValue = function(editor) {
        this.proxy.setValue("detectedIndent", this.detectIndentationMode(editor));
        editor.clearHistory();
        this.proxy.sendEditorEvent("J_EVT_FILE_LOADED", 0);
    }

    // Hook for when cursor activity is detected(cursor moved/selection changed)
    this.onCursorActivity = function(editor) {
        var out = [];
        var sels = editor.listSelections();
        for (var i = 0; i < sels.length; i++) {
            out[i] = {  anchor: {
                            line: sels[i].anchor.line,
                            ch: sels[i].anchor.ch
                        },
                        head: {
                            line: sels[i].head.line,
                            ch: sels[i].head.ch
                        }
                    };
        }
        this.proxy.selections = out;
        this.proxy.selectionsText = editor.getSelections("\n");
        var cur = editor.getCursor();
        this.proxy.setValue("cursor", [cur.line, cur.ch]);
    }

    // Hook for when any content is changed
    this.onChange = function(editor) {
        this.proxy.textLength = editor.getValue("\n").length;
        this.proxy.lineCount = editor.lineCount();
        this.proxy.clean = isCleanOrForced(changeGeneration);
    }

    // Hook for when the user scrolls the page.
    this.onScroll = function(editor) {
        var scroll = editor.getScrollInfo();
        this.proxy.scrollPosition = [scroll.left, scroll.top];
    }

    // Hook for when the editor is in a functional loaded state
    this.onLoad = function(editor) {
        this.onCursorActivity(editor);
        this.onChange(editor);
        this.onScroll(editor);
        this.onOptionChange(editor);
        this.proxy.sendEditorEvent("J_EVT_READY", 0);
        editor.refresh();
    }
    // END HOOKS
}


