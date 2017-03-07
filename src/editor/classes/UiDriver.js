/*

             Synchronization with QtWebEngine

  [JS]               [QtWebEngine]                [C++]
   |                       |                        |
   |                  pageLoaded()  --------------->|
   |                       |                   Create socket and bind cpp_ui_driver with QWebChannel
   |<----------------------|-----------------  Call js function connectSocket()
connectSocket()            |                        |
J_EVT_READY ---------------|----------------------->|
Editor is ready.           |                   Editor is ready.
   |                       |                        |
   -                       -                        -


             Synchronization with QtWebKit

  [JS]                 [QtWebKit]                 [C++]
   |                       |                        |
   |               jsObjectCleared()  ------------->|
   |                       |                   Bind cpp_ui_driver
J_EVT_READY ---------------|----------------------->|
Editor is ready.           |                   Editor is ready.
   |                       |                        |
   -                       -                        -

*/

var UiDriver = new function() {
    var handlers = [];

    var socket;
    var channel;
    var msgQueue = [];

    var _this = this;

    function usingQtWebChannel() {
        return (typeof cpp_ui_driver === 'undefined')
    }

    this.proxy = undefined;


    
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

/*    this.sendMessage = function(msg, data) {
        if (usingQtWebChannel()) {
            // QtWebEngine

            if (channel === undefined) {
                // Communication with the C++ part is not yet completed.
                msgQueue.push([msg, data]);
                return;
            }
            console.log("Reply data: " + msg);
//            if (data !== null && data !== undefined) {
//                channel.objects.cpp_ui_driver.receiveMessage(msg, data, function(ret) { console.error(msg + " sent to c++ (async)") });
//            } else {
//                channel.objects.cpp_ui_driver.receiveMessage(msg, "", function(ret) { console.error(msg + " sent to c++ (async)") });
//            }
        } else {
            // QtWebKit
            cpp_ui_driver.receiveMessage(msg, data);
        }
    }
*/
    this.registerEventHandler = function(msg, handler) {
        if (handlers[msg] === undefined)
            handlers[msg] = [];

        handlers[msg].push(handler);
    }

    this.setReturnData = function(data) {
//        console.error("Setting return data to: " + data);
        channel.objects.cpp_ui_driver.result = data;
    }
    
    this.handleMessageInternally = function(msg, data) {
//        console.error("Received internal message: "+ msg);

        if (handlers[msg] !== undefined) {
//            console.error("Defined handlers[msg] has " + handlers[msg].length + ": " + msg);
            handlers[msg].forEach(function(handler) {
//                console.error("Foreach: "+ handler);
            });
        }
 
    }

    this.messageReceived = function(msg, data) {
//        console.error("Received message: "+ msg);
        if (!usingQtWebChannel()) {
            console.error("Not using QtWebChannel: "+ msg);
            data = cpp_ui_driver.getMsgData();
        }

        // Only one of the handlers (the last that gets
        // called) can return a value. So, to each handler
        // we provide the previous handler's return value.
        var prevReturn = undefined;
        if (handlers[msg] !== undefined) {
//            console.error("Defined handlers[msg] has " + handlers[msg].length + ": " + msg);
            handlers[msg].forEach(function(handler) {
//                console.error("Foreach: "+ handler);
                prevReturn = handler(msg, data, prevReturn);
            });
        }
        
        if(prevReturn !== undefined) {
//            console.error("Setting return data for: "+ msg);
            _this.setReturnData(prevReturn);
        }
        return prevReturn;
    }

    // BEGIN EVENT FUNCTIONS
    // These functions hook into the CodeMirror interface and allow us to
    // perform asynchronous data transfer between C++ and Javascript.


    // Hook for when we load a file/change content
    this.onSetValue = function(editor) {
        if(this.proxy === undefined) {
            return;
        }
        this.proxy.detectedIndent = this.detectIndentationMode(editor);
    }

    // Hook for when cursor activity is detected(cursor moved/selection changed)
    this.onCursorActivity = function(editor) {
        if(this.proxy === undefined) {
            return;
        }
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
        if(this.proxy === undefined) {
            return;
        }
        this.proxy.textLength = editor.getValue("\n").length;
        this.proxy.lineCount = editor.lineCount();
    }

    // Hook for when the user scrolls the page.
    this.onScroll = function(editor) {
        if(this.proxy === undefined) {
            return;
        }
        var scroll = editor.getScrollInfo();
        this.proxy.scrollPosition = [scroll.left, scroll.top];
    }

    this.onLoad = function(editor) {
        // Initialize all values here for safety.
        if (this.proxy === undefined) {
            return;
        }
        this.onCursorActivity(editor);
        this.onChange(editor);
        this.proxy.sendEditorEvent("J_EVT_READY", 0);
        editor.refresh();
        console.error("ONLOAD CALLED");
    }

    // END EVENT FUNCTIONS
}


