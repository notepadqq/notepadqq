/*
    Quick small script that initializes the page.
    This is run as soon as possible, so jQuery and other libraries
    are not available.
*/

var _initialized = false;
var _defaultTheme = "";

function addStylesheet(path) {
    var link = document.createElement("link");
    link.href = path;
    link.type = "text/css";
    link.rel = "stylesheet";
    link.media = "screen,print";

    document.getElementsByTagName("head")[0].appendChild(link);
}

function init()
{
    if (_initialized)
        return;
    
    function getParameterByName(name) {
        name = name.replace(/[\[]/, "\\[").replace(/[\]]/, "\\]");
        var regex = new RegExp("[\\?&]" + name + "=([^&#]*)"),
            results = regex.exec(location.search);
        return results == null ? "" : decodeURIComponent(results[1].replace(/\+/g, " "));
    }
    
    var themePath = getParameterByName("themePath");
    var themeName = getParameterByName("themeName");
    if (themePath !== "") {
        addStylesheet(themePath);
    }
    
    _defaultTheme = themeName === "" ? "default" : themeName;
}

init();