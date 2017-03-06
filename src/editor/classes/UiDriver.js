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


    this.sendMessage = function(msg, data) {
        if (usingQtWebChannel()) {
            // QtWebEngine

            if (channel === undefined) {
                // Communication with the C++ part is not yet completed.
                msgQueue.push([msg, data]);
                return;
            }
            console.log("Reply data: " + msg);
            if (data !== null && data !== undefined) {
                channel.objects.cpp_ui_driver.receiveMessage(msg, data, function(ret) { console.error(msg + " sent to c++ (async)") });
            } else {
                channel.objects.cpp_ui_driver.receiveMessage(msg, "", function(ret) { console.error(msg + " sent to c++ (async)") });
            }
            channel.objects.cpp_ui_driver.makeReplyReady();
        } else {
            // QtWebKit
            cpp_ui_driver.receiveMessage(msg, data);
        }
    }

    this.registerEventHandler = function(msg, handler) {
        if (handlers[msg] === undefined)
            handlers[msg] = [];

        handlers[msg].push(handler);
    }

    this.setReturnData = function(data) {
        console.error("Setting return data to: " + data);
        channel.objects.cpp_ui_driver.m_result = data;
        channel.objects.cpp_ui_driver.makeReplyReady();
        channel.objects.cpp_ui_driver.m_result = "";
    }
    
    this.handleMessageInternally = function(msg, data) {
        console.error("Received internal message: "+ msg);

        if (handlers[msg] !== undefined) {
            console.error("Defined handlers[msg] has " + handlers[msg].length + ": " + msg);
            handlers[msg].forEach(function(handler) {
                console.error("Foreach: "+ handler);
            });
        }
 
    }

    this.messageReceived = function(msg, data) {
        console.error("Received message: "+ msg);
        if (!usingQtWebChannel()) {
            console.error("Not using QtWebChannel: "+ msg);
            data = cpp_ui_driver.getMsgData();
        }

        // Only one of the handlers (the last that gets
        // called) can return a value. So, to each handler
        // we provide the previous handler's return value.
        var prevReturn = undefined;
        if (handlers[msg] !== undefined) {
            console.error("Defined handlers[msg] has " + handlers[msg].length + ": " + msg);
            handlers[msg].forEach(function(handler) {
                console.error("Foreach: "+ handler);
                prevReturn = handler(msg, data, prevReturn);
            });
        }
        
        if(prevReturn !== undefined) {
            console.error("Setting return data for: "+ msg);
            _this.setReturnData(prevReturn);
        }
        return prevReturn;
    }  
        new QWebChannel(qt.webChannelTransport, function (_channel) {
            channel = _channel;
            // Send the messages in the queue
            while (msgQueue.length) {
                var cur = msgQueue.shift();
                _this.sendMessage(cur[0], cur[1]);
            //    console.error(msgQueue);
            }
            channel.objects.cpp_ui_driver.sendMsg.connect(function(msg, data) {
                _this.messageReceived(msg, data);
            });
        });
}


