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
        // Sub-function objects
        this.proxy = new CommunicationsManager();
        this.events = new EditorEventHook();
        this.content = new ContentManager();

        //Variables
        this.defaultTheme = "default";

        function getQueryValue(name) 
        {
            name = name.replace(/[\[]/, "\\[").replace(/[\]]/, "\\]");
            let regex = new RegExp("[\\?&]" + name + "=([^&#]*)"),
                results = regex.exec(location.search);
            if (results !== null) {
                return decodeURIComponent(results[1].replace(/\+/g, " "));
            }
            return "";
        }

        //Try to get the current preferred theme.
        let themePath = getQueryValue("themePath");
        let themeName = getQueryValue("themeName");
        if (themePath !== "") {
            this.setEditorStyleSheet(themePath);
            this.defaultTheme = themeName;
        }
    }

    setEditorStyleSheet(sheet)
    {
        document.getElementById('pagestyle').setAttribute('href', sheet);
    }
}

var App = new AppManager();
