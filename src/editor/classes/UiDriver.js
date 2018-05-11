var UiDriver = new function() {
    var handlers = [];

    var msgQueue = [];
    var cpp_ui_driver = null;

    // Setup the communication channel
    document.addEventListener("DOMContentLoaded", () => {
        new QWebChannel(qt.webChannelTransport, (channel) => {

            cpp_ui_driver = channel.objects.cpp_ui_driver;

            // Connect to the signal that tells us when we have a new incoming message
            channel.objects.cpp_ui_driver.messageReceivedByJs.connect((msg, data) => {
                this.messageReceived(msg, data);
            });

            // Send the queued messages that were sent while the channel wasn't ready yet.
            for (var i = 0; i < msgQueue.length; i++) {
                this.sendMessage(msgQueue[i][0], msgQueue[i][1]);
            }
            msgQueue = [];
        
            // http://doc.qt.io/archives/qt-5.7/qtwebchannel-javascript.html
        });
    });

    // Send a message to C++
    this.sendMessage = function(msg, data) {
        if (cpp_ui_driver === null) { // Channel not yet ready: enqueue the message
            msgQueue.push([msg, data]);
            return;
        }

        if (data !== null && data !== undefined) {
            cpp_ui_driver.receiveMessage(msg, data, function(ret) {  });
        } else {
            cpp_ui_driver.receiveMessage(msg, "", function(ret) {  });
        }
    }

    this.registerEventHandler = function(msg, handler) {
        if (handlers[msg] === undefined)
            handlers[msg] = [];

        handlers[msg].push(handler);
    }

    // Invoked whenever we've got an incoming message from C++
    this.messageReceived = function(msg, data) {
        // Check if the message is async
        if (msg.startsWith("[ASYNC_REQUEST]")) {
            
            // Async mode
            var rgx = /^\[ASYNC_REQUEST\](.*)\[ID=(\d+)\]$/g;
            var match = rgx.exec(msg);

            if (match.length == 3) {
                var real_msg = match[1];
                var msg_id = parseInt(match[2]);

                // Only one of the handlers (the last that gets
                // called) can return a value. So, to each handler
                // we provide the previous handler's return value.
                var prevReturn = undefined;

                if (handlers[real_msg] !== undefined) {
                    handlers[real_msg].forEach(function(handler) {
                        prevReturn = handler(real_msg, data, prevReturn);
                    });
                }

                // Send an asynchronous reply
                this.sendMessage("[ASYNC_REPLY]" + real_msg + "[ID=" + msg_id + "]", prevReturn);
            }

        } else {

            // Only one of the handlers (the last that gets
            // called) can return a value. So, to each handler
            // we provide the previous handler's return value.
            var prevReturn = undefined;

            if (handlers[msg] !== undefined) {
                handlers[msg].forEach(function(handler) {
                    prevReturn = handler(msg, data, prevReturn);
                });
            }

            // Send a synchronous reply
            return prevReturn;
        }
    }
}


if (!String.prototype.startsWith) {
	String.prototype.startsWith = function(search, pos) {
		return this.substr(!pos || pos < 0 ? 0 : +pos, search.length) === search;
	};
}