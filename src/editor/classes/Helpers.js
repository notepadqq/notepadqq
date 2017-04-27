"use strict";
/**
 * @brief This file contains helper functions for use by the MessageHandler.js
 *        or others.
 */
var spaceToTabCounter = 0;
var tabToSpaceCounter = 0;
var SearchMode = {
    PlainText: 1,
    SpecialChars: 2,
    Regex: 3
}

function spaceToTabHelper(match, offset, tabSize) 
{
    // Like with tabToSpace, we need to keep track of 
    // the inserted/deleted character count.
    var start = offset + spaceToTabCounter;
    var len = match.length;
    var result = "";

    // Search for the first tab line manually
    var leading = tabSize - (start % tabSize);
    if (len >= leading) {
        result += "\t";
        len -= leading;
    }

    // then replace spaces with tabs
    while(len>=tabSize) {
        result += "\t";
        len -= tabSize;
    }

    // finally add spaces if we can't add tabs anymore
    while(len>0) {
        result += " ";
        len -= 1;
    }

    spaceToTabCounter -= (match.length - result.length);

    return result;
}

function tabToSpaceHelper(match, offset, tabSize) 
{
    /*
        string.replace() does not update the string inbetween invokations 
        of this update function.
        Since we replace a single tab with multiple spaces we've got 
        to keep track of the extra string length outselves. 
        tabToSpaceCounter holds the number of extra spaces we've added.
    */
    var trueOffset = offset + tabToSpaceCounter;

    var numSpaces = tabSize - (trueOffset % tabSize);

    // Since the original tab is replaced by a space we only need numSpaces-1 
    // new spaces.
    tabToSpaceCounter += numSpaces-1;

    // Generate the whitespace. Sadly " ".repeat(numSpaces) does not work 
    // with this js interpreter.
    var space = "";
    for (var i = 0; i< numSpaces; i++)
        space += " ";

    return space;
}

/*
    Determine whether the proposed replacement contains
    group reuse tokens i.e. \1, \2, etc.
    (Helper function for search/replace & replace all.)
*/
function hasGroupReuseTokens(replacement){
    var groupReuseRegex = /\\([1-9])/g;
    return (groupReuseRegex.exec(replacement) !== null);
}

/*
    Substitute group reuse tokens (i.e. \1, \2, etc.) with 
    the matched groups provided.
    (Helper function for search/replace & replace all.)
    groups: contains array of regexp matches, where the first 
    entry is the whole match and the rest are groups.

*/
function applyReusedGroups(replacement, groups)
{
    //If we got match subgroups, see if we need to alter the replacement
    for (var iReuseGroup = 1; iReuseGroup < groups.length; iReuseGroup ++){
        //takes care of non-consecutive group reuse tokens,
        //i.e. in "\1 \3" with no "\2", the "\3" is ignored 
        groupToReuse = groups[iReuseGroup];
        replacement = replacement.replace(new RegExp("\\\\"+iReuseGroup), groupToReuse);
    }
    var groupReuseRegex = /\\([1-9])/g;
    //take care of all non-matched group reuse tokens (replace with empty string)
    //this is the Notepad++ functionality
    replacement = replacement.replace(groupReuseRegex,"");
    return replacement;
}

function Search(regexStr, regexModifiers, forward) {
    var startPos;

    // Avoid getting stuck finding always the same text
    if (forward)
        startPos = editor.getCursor("to");
    else
        startPos = editor.getCursor("from");

    // We get a new cursor every time, because the user could have moved within
    // the editor and we want to start searching from the new position.
    var searchRegex = new RegExp(regexStr, regexModifiers);
    var searchCursor = editor.getSearchCursor(searchRegex, startPos, false);

    var ret = forward ? searchCursor.findNext() : searchCursor.findPrevious();

    if (!ret) {
        // Maybe the end was reached. Try again from the start.
        if (forward) {
            searchCursor = editor.getSearchCursor(searchRegex, null, false);
        } else {
            var line = editor.lineCount() - 1;
            var ch = editor.getLine(line).length;
            searchCursor = editor.getSearchCursor(searchRegex, {line: line, ch: ch}, false);
        }

        ret = forward ? searchCursor.findNext() : searchCursor.findPrevious();
    }

    if (ret) {
        if (forward)
            editor.setSelection(searchCursor.from(), searchCursor.to());
        else
            editor.setSelection(searchCursor.to(), searchCursor.from());
    }

    return ret;
}

function editLines(funct)
{
    editor.operation(function(){
        var len = editor.lineCount();
        for (var i = 0; i < len; i++) {
            var line = editor.getLine(i);
            var from = {line: i, ch: 0};
            var to = {line: i, ch: line.length};
            editor.replaceRange(funct(line), from,to);
        }
    });
}

function rgb2hex(rgb)
{
    rgb = rgb.match(/^rgba?[\s+]?\([\s+]?(\d+)[\s+]?,[\s+]?(\d+)[\s+]?,[\s+]?(\d+)[\s+]?/i);
    return (rgb && rgb.length === 4) ? "#" +
    ("0" + parseInt(rgb[1],10).toString(16)).slice(-2) +
    ("0" + parseInt(rgb[2],10).toString(16)).slice(-2) +
    ("0" + parseInt(rgb[3],10).toString(16)).slice(-2) : '';
}

function getBackgroundColor(theme)
{
    let style = $('.cm-s-' + theme);
    return rgb2hex(style.css('background-color'));
}
