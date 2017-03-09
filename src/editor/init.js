/*
    Quick small script that initializes the page.
    This is run as soon as possible, so jQuery and other libraries
    might not be available.
*/

var _initialized = false;

function swapStylesheet(path)
{
    console.error(path);
    console.error(localStorage.getItem('CodeMirrorTheme'));
    if(localStorage.getItem('CodeMirrorTheme') !== path) {
        localStorage.setItem('CodeMirrorTheme', path);
    }
    $('link.CodeMirrorTheme').prop('href', localStorage.getItem('CodeMirrorTheme'));
}

function init()
{
    if (_initialized)
        return;
    var themeName = localStorage.getItem('CodeMirrorThemeName');
    if(themeName === null) {
        localStorage.setItem('CodeMirrorThemeName', 'default');
    }
    $('link.CodeMirrorTheme').prop('href', localStorage.getItem('CodeMirrorTheme'));
}

init();
