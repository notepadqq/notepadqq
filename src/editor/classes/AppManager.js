"use strict";
/**
 * @brief This is our controller class.  All calls are made through here
 *        so we have a consistent interface for javascript communication.
 * 
 * Some brief examples of how to use this:
 * App.proxy.sendEvent(Message, Data);
 * App.eventHook.onChange();
 *
 */
class AppManager {
    constructor()
    {
        this.proxy = new CommunicationsManager();
        this.eventHook = new EditorEventHook();
        this.content = new ContentManager();
    }
}

var App = new AppManager();

