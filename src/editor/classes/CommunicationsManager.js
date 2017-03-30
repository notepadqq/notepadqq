"use strict";
/**
 * @brief This class handles all of our JS-to-CPP communication.
 * 
 * It's defined as a function so QWebChannel will behave a little better.
 * There should never be a need to call CppProxy.sendEditorEvent directly.
 *
 * It's best to use the App interface provided for safety reasons.
 */
function CommunicationsManager() {
    var handlers = new MessageHandler();
    var CppProxy;
    new QWebChannel(qt.webChannelTransport, _channel => {
        CppProxy = _channel.objects.cpp_ui_driver;
        CppProxy.sendMsgInternal.connect((msg, data) => {
            this.messageReceived(msg, data);
        });
    });
    
    /**
     * @brief Sends a message to CPP containing event data for it to interpret
     * @param msg - J_EVT_SOME_THING
     * @param data - Typically a JSON object
     */
    this.sendEvent = function(msg, data) {
        // Support just sending a message
        if (data == undefined) {
            data = 0;
        }
        // Check if CppProxy initialized, otherwise defer.
        if (CppProxy != undefined) {
            CppProxy.sendEditorEvent(msg, data);
        } else {
            var wait = setInterval(function() {
                if(CppProxy != undefined) {
                    CppProxy.sendEditorEvent(msg, data);
                    clearInterval(wait);
                }
            }, 1);
        }
    }

    /**
     * @brief Sends return data to the CPP proxy
     * @param data - Typically a JSON object
     */
    this.setReturnData = function(data) {
        CppProxy.result = data;
    }

    /**
     * @brief A message was received from the CPP proxy.
     * @param msg - i.e. C_CMD_SOME_FUNCTION
     * @param data - QVariant converted to a JSON object.
     * @return return value of the handler called.
     */
    this.messageReceived = function(msg, data) {
        var retVal = undefined;
        if (typeof handlers[msg] === 'function') {
            var fn = handlers[msg];
            retVal = fn(data);
        }else {
            console.error('Unhandled message exception: ' + msg);
        }
        if(retVal !== undefined) {
            this.setReturnData(retVal);
        }
        return retVal;
    }

    this.cleanChangedEvent = data => {
        this.sendEvent("J_EVT_CLEAN_CHANGED", data);
    }

    this.contentChangedEvent = data => {
        this.sendEvent("J_EVT_CONTENT_CHANGED", data);
    }

    this.cursorActivityEvent = data => {
        this.sendEvent("J_EVT_CURSOR_ACTIVITY", data);
    }

    this.documentLoadedEvent = data => {
        this.sendEvent("J_EVT_DOCUMENT_LOADED", data);
    }

    this.scrollChangedEvent = data => {
        this.sendEvent("J_EVT_SCROLL_CHANGED", data);
    }

    this.focusChangedEvent = data => {
        this.sendEvent("J_EVT_GOT_FOCUS", data);
    }

    this.setReady = () => {
        this.sendEvent("J_EVT_READY");
    }

    this.optionChangedEvent = (key, value) => {
        let pair = {
            "key": key,
            "value": value
        };
        this.sendEvent("J_EVT_OPTION_CHANGED", pair);
    }
}

