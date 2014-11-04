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
