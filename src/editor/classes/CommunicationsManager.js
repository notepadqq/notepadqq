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
        CppProxy.sendEditorEvent("J_EVT_PROXY_INIT", 0);
        CppProxy.sendMsgInternal.connect((msg, data) => {
            this.messageReceived(msg, data);
        });
    });
    
    // Wrapper for QWebChannel which avoids undefined behavior.
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

    this.setReturnData = function(data) {
        CppProxy.result = data;
    }

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

    this.setReady = () => {
        this.sendEvent("J_EVT_READY");
    }
}

