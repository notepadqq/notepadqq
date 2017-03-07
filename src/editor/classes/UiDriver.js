
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

    // Hook for when the editor's language is changed.
    this.onLanguageChange = function(editor) {
        var langId = Languages.currentLanguage(editor);
        this.proxy.language = {id: langId, lang: Languages.languages[langId]};
    }

    // Hook for when we load a file/change content
    this.onSetValue = function(editor) {
        this.proxy.detectedIndent = this.detectIndentationMode(editor);
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
        this.proxy.cursor = [cur.line, cur.ch];
    }

    // Hook for when any content is changed
    this.onChange = function(editor) {
        this.proxy.textLength = editor.getValue("\n").length;
        this.proxy.lineCount = editor.lineCount();
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
        this.proxy.sendEditorEvent("J_EVT_READY", 0);
        editor.refresh();
        console.error("ONLOAD CALLED");
    }
    // END HOOKS
}


