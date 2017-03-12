class CommDriver {
    constructor() {
        var _this = this;
        this.handlers = [];
        new QWebChannel(qt.webChannelTransport, function (_channel) {
            _this.proxy = _channel.objects.cpp_ui_driver;
            _this.proxy.sendEditorEvent("J_EVT_PROXY_INIT", 0);
            _this.proxy.sendMsgInternal.connect(function(msg, data) {
                _this.messageReceived(msg, data);
            });
        });
    }
    
    registerEventHandler(msg, handler) {
        if (this.handlers[msg] === undefined)
            this.handlers[msg] = [];
        this.handlers[msg].push(handler);
    }

    setReturnData(data) {
        this.proxy.result = data;
    }
    
    messageReceived(msg, data) {
        // Only one of the handlers (the last that gets
        // called) can return a value. So, to each handler
        // we provide the previous handler's return value.
        var prevReturn = undefined;
        if (this.handlers[msg] !== undefined) {
            this.handlers[msg].forEach(function(handler) {
                prevReturn = handler(msg, data, prevReturn);
            });
        }
        
        if(prevReturn !== undefined) {
            this.setReturnData(prevReturn);
        }
        return prevReturn;
    }
}


