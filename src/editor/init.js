/*
    Quick small script that initializes the page.
    This is run as soon as possible, so jQuery and other libraries
    might not be available.
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
    return link;
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

    // For Monaco: pre-load the theme script synchronously so it's defined before
    // the editor is created. Only load custom themes — builtins are hardcoded.
    var engine = getParameterByName("engine");
    var monacoBuiltins = ["vs", "vs-dark", "hc-black", "hc-light"];
    if (engine === "monaco" && themeName !== "" && themeName !== "default" && monacoBuiltins.indexOf(themeName) < 0) {
        // Provide a stub for defineTheme in case monaco.editor isn't loaded yet.
        // Without this, the theme script will throw because monaco is undefined.
        // Actual themes are applied from the queue once monaco is ready.
        document.write('<script>window.__pendingThemes=window.__pendingThemes||{};if(typeof monaco==="undefined"||!monaco.editor){window.monaco=window.monaco||{};window.monaco.editor=window.monaco.editor||{};window.monaco.editor.defineTheme=function(n,d){window.__pendingThemes[n]=d;}}<\/script>');
        document.write('<script src="libs/monaco-addons/themes/' + themeName + '.js"><\/script>');
    }

    _defaultTheme = themeName === "" ? "default" : themeName;
}

init();
