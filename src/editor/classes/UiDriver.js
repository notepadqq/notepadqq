var UiDriver = new function() {
    var handlers = [];

    this.sendMessage = function(msg, data) {
        cpp_ui_driver.messageReceived(msg, data);
    }

    this.registerEventHandler = function(msg, handler) {
        if (handlers[msg] === undefined)
            handlers[msg] = [];

        handlers[msg].push(handler);
    }

    this.messageReceived = function(msg) {
        var data = cpp_ui_driver.getMsgData();
        
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

            return prevReturn;
        }
    }
}


if (!String.prototype.startsWith) {
	String.prototype.startsWith = function(search, pos) {
		return this.substr(!pos || pos < 0 ? 0 : +pos, search.length) === search;
	};
}