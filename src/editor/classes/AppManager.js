"use strict";
/**
 * @brief This is our controller class.  All calls are made through here
 *        so we have a consistent interface for javascript communication.
 * 
 * Some brief examples of how to use this:
 * App.proxy.sendEvent(Message, Data);
 * App.eventHook.onChange();
 * App.helpers.Search(a, b, c);
 *
 */
class AppManager {
    constructor()
    {
        this.helpers = new Helpers();
        this.proxy = new CommunicationsManager();
        this.eventHook = new EditorEventHook();
        this.state = new StateManager();
    }
}

var App = new AppManager();

