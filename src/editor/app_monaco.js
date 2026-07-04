var editor;
var monacoModel;

var cleanVersionId = 0;
var currentVersionId = 0;
var forceDirty = false;

// ── Helpers ──

function isCleanOrForced() {
    return !forceDirty && currentVersionId === cleanVersionId;
}

function posToMonaco(line, ch) {
    return { lineNumber: line + 1, column: ch + 1 };
}

function posFromMonaco(pos) {
    return [pos.lineNumber - 1, pos.column - 1];
}

function rangeFromMonaco(range) {
    return { anchor: { line: range.startLineNumber - 1, ch: range.startColumn - 1 },
             head:   { line: range.endLineNumber - 1,   ch: range.endColumn - 1 } };
}

function offsetToPos(model, offset) {
    var p = model.getPositionAt(offset);
    return { line: p.lineNumber - 1, ch: p.column - 1 };
}

function posToOffset(model, line, ch) {
    return model.getOffsetAt({ lineNumber: line + 1, column: ch + 1 });
}

function editLines(fn) {
    var model = editor.getModel();
    var len = model.getLineCount();
    var edits = [];
    for (var i = 0; i < len; i++) {
        var line = model.getLineContent(i + 1);
        var newLine = fn(line);
        if (newLine !== line) {
            edits.push({
                range: new monaco.Range(i + 1, 1, i + 1, line.length + 1),
                text: newLine
            });
        }
    }
    if (edits.length > 0)
        model.pushEditOperations([], edits, function() { return null; });
}

// ── Search ──

function Search(regexStr, regexModifiers, forward) {
    var model = editor.getModel();
    var content = model.getValue();
    var searchRegex = new RegExp(regexStr, regexModifiers);
    var cursorPos = editor.getPosition();
    var cursorOffset = model.getOffsetAt(cursorPos);

    var match = null;
    var matchOffset = -1;

    if (forward) {
        var sub = content.substring(cursorOffset);
        var m = searchRegex.exec(sub);
        if (m) {
            matchOffset = cursorOffset + m.index;
            match = m;
        } else {
            searchRegex.lastIndex = 0;
            m = searchRegex.exec(content);
            if (m) { matchOffset = m.index; match = m; }
        }
    } else {
        var lastM = null, lastOff = -1;
        searchRegex.lastIndex = 0;
        while ((m = searchRegex.exec(content)) !== null) {
            if (m.index < cursorOffset) { lastM = m; lastOff = m.index; }
            else break;
        }
        if (lastM) { matchOffset = lastOff; match = lastM; }
        else {
            var rev = new RegExp(regexStr, regexModifiers);
            rev.lastIndex = 0;
            while ((m = rev.exec(content)) !== null) { lastM = m; lastOff = m.index; }
            if (lastM) { matchOffset = lastOff; match = lastM; }
        }
    }

    if (match) {
        var startPos = model.getPositionAt(matchOffset);
        var endPos = model.getPositionAt(matchOffset + match[0].length);
        editor.setSelection(new monaco.Range(startPos.lineNumber, startPos.column, endPos.lineNumber, endPos.column));
        editor.revealRangeInCenter(editor.getSelection());
        return true;
    }
    return false;
}

function hasGroupReuseTokens(replacement) {
    return /\\([1-9])/g.test(replacement);
}

function applyReusedGroups(replacement, groups) {
    for (var i = 1; i < groups.length; i++) {
        replacement = replacement.replace(new RegExp("\\\\" + i, "g"), groups[i]);
    }
    replacement = replacement.replace(/\\([1-9])/g, "");
    return replacement;
}

var SearchMode = { PlainText: 1, SpecialChars: 2, Regex: 3 };

function searchAndReplaceAll(regexStr, regexModifiers, replacement, searchMode) {
    var model = editor.getModel();
    var content = model.getValue();
    var searchRegex = new RegExp(regexStr, regexModifiers);
    var hasReuse = hasGroupReuseTokens(replacement) && searchMode === SearchMode.Regex;
    var count = 0;
    var delta = 0;
    var match;
    searchRegex.lastIndex = 0;
    while ((match = searchRegex.exec(content)) !== null) {
        count++;
        var replace = hasReuse ? applyReusedGroups(replacement, match) : replacement;
        var matchLen = match[0].length;
        content = content.slice(0, match.index + delta) + replace + content.slice(match.index + delta + matchLen);
        delta += replace.length - matchLen;
        searchRegex.lastIndex = match.index + delta + (replace.length > 0 ? 1 : 0);
    }
    model.setValue(content);
    return count;
}

// ── Event handlers (throttled) ──

function onCursorActivity() {
    if (!onCursorActivity._throttled) {
        require(['libs/throttle-debounce/index'], function(thdb) {
            onCursorActivity._throttled = thdb.throttle(50, function() {
                UiDriver.sendMessage("J_EVT_CURSOR_ACTIVITY", getDocumentInfo());
            });
            onCursorActivity._throttled();
        });
    } else {
        onCursorActivity._throttled();
    }
}

function onChange() {
    currentVersionId++;
    if (!onChange._throttled) {
        require(['libs/throttle-debounce/index'], function(thdb) {
            onChange._throttled = thdb.throttle(50, function() {
                UiDriver.sendMessage("J_EVT_CONTENT_CHANGED");
                UiDriver.sendMessage("J_EVT_CLEAN_CHANGED", isCleanOrForced());
            });
            onChange._throttled();
        });
    } else {
        onChange._throttled();
    }
}

function getDocumentInfo() {
    var model = editor.getModel();
    var cursor = editor.getPosition();
    var sel = editor.getSelection();
    var selectedText = model.getValueInRange(sel);
    var map = {};
    map["cursor"] = [cursor.lineNumber - 1, cursor.column - 1];
    map["selections"] = [selectedText.split(/\r\n|\r|\n/).length, selectedText.length];
    map["content"] = [model.getLineCount(), model.getValue().length];
    return map;
}

// ── Message handlers ──

UiDriver.registerEventHandler("C_CMD_SET_VALUE", function(msg, data, prevReturn) {
    editor.getModel().setValue(data);
});

UiDriver.registerEventHandler("C_FUN_GET_VALUE", function(msg, data, prevReturn) {
    return editor.getModel().getValue();
});

UiDriver.registerEventHandler("C_CMD_MARK_CLEAN", function(msg, data, prevReturn) {
    forceDirty = false;
    cleanVersionId = currentVersionId;
    UiDriver.sendMessage("J_EVT_CLEAN_CHANGED", isCleanOrForced());
});

UiDriver.registerEventHandler("C_CMD_MARK_DIRTY", function(msg, data, prevReturn) {
    forceDirty = true;
    UiDriver.sendMessage("J_EVT_CLEAN_CHANGED", isCleanOrForced());
});

UiDriver.registerEventHandler("C_FUN_IS_CLEAN", function(msg, data, prevReturn) {
    return isCleanOrForced();
});

UiDriver.registerEventHandler("C_FUN_GET_HISTORY_GENERATION", function(msg, data, prevReturn) {
    return currentVersionId;
});

UiDriver.registerEventHandler("C_CMD_SET_LANGUAGE", function(msg, data, prevReturn) {
    monaco.editor.setModelLanguage(editor.getModel(), data || "plaintext");
});

UiDriver.registerEventHandler("C_CMD_SET_INDENTATION_MODE", function(msg, data, prevReturn) {
    var opts = {};
    if (data.useTabs !== undefined) opts.insertSpaces = !data.useTabs;
    if (data.size !== undefined && data.size > 0) {
        opts.tabSize = data.size;
    }
    editor.getModel().updateOptions(opts);
});

UiDriver.registerEventHandler("C_FUN_GET_INDENTATION_MODE", function(msg, data, prevReturn) {
    var opts = editor.getModel().getOptions();
    return { useTabs: !opts.insertSpaces, size: opts.tabSize };
});

UiDriver.registerEventHandler("C_FUN_GET_SELECTIONS_TEXT", function(msg, data, prevReturn) {
    var sels = editor.getSelections() || [editor.getSelection()];
    var model = editor.getModel();
    var out = [];
    for (var i = 0; i < sels.length; i++) {
        out.push(model.getValueInRange(sels[i]));
    }
    return out.join("\n");
});

UiDriver.registerEventHandler("C_CMD_SET_SELECTIONS_TEXT", function(msg, data, prevReturn) {
    var text = data.text;
    var selectMode = undefined;
    if (data.select === "before") selectMode = monaco.SelectionDirection.LTR;
    else if (data.select === "selected") selectMode = monaco.SelectionDirection.RTL;

    var sels = editor.getSelections() || [editor.getSelection()];
    var model = editor.getModel();
    if (text.length === sels.length) {
        var edits = [];
        for (var i = 0; i < sels.length; i++) {
            edits.push({
                range: sels[i],
                text: text[i]
            });
        }
        model.pushEditOperations([], edits, function() { return sels; });
    } else {
        editor.executeEdits("setSelectionsText", [{
            range: model.getFullModelRange(),
            text: text.join("\n")
        }]);
    }
});

UiDriver.registerEventHandler("C_FUN_GET_SELECTIONS", function(msg, data, prevReturn) {
    var sels = editor.getSelections() || [editor.getSelection()];
    var out = [];
    for (var i = 0; i < sels.length; i++) {
        out.push(rangeFromMonaco(sels[i]));
    }
    return out;
});

UiDriver.registerEventHandler("C_CMD_SET_SELECTION", function(msg, data, prevReturn) {
    var from = posToMonaco(data[0], data[1]);
    var to = posToMonaco(data[2], data[3]);
    editor.setSelection(new monaco.Range(from.lineNumber, from.column, to.lineNumber, to.column));
});

UiDriver.registerEventHandler("C_FUN_GET_TEXT_LENGTH", function(msg, data, prevReturn) {
    return editor.getModel().getValue().length;
});

UiDriver.registerEventHandler("C_FUN_GET_LINE_COUNT", function(msg, data, prevReturn) {
    return editor.getModel().getLineCount();
});

UiDriver.registerEventHandler("C_FUN_GET_CURSOR", function(msg, data, prevReturn) {
    return posFromMonaco(editor.getPosition());
});

UiDriver.registerEventHandler("C_CMD_SET_CURSOR", function(msg, data, prevReturn) {
    editor.setPosition(posToMonaco(data[0], data[1]));
});

UiDriver.registerEventHandler("C_CMD_SET_RTL", function(msg, data, prevReturn) {
    // Monaco direction toggle not exposed; no-op
});

UiDriver.registerEventHandler("C_CMD_SET_LTR", function(msg, data, prevReturn) {
    // No-op
});

UiDriver.registerEventHandler("C_FUN_GET_SCROLL_POS", function(msg, data, prevReturn) {
    return [editor.getScrollLeft(), editor.getScrollTop()];
});

UiDriver.registerEventHandler("C_CMD_SET_SCROLL_POS", function(msg, data, prevReturn) {
    editor.setScrollLeft(data[0]);
    editor.setScrollTop(data[1]);
});

UiDriver.registerEventHandler("C_CMD_SELECT_ALL", function(msg, data, prevReturn) {
    editor.setSelection(editor.getModel().getFullModelRange());
});

UiDriver.registerEventHandler("C_CMD_UNDO", function(msg, data, prevReturn) {
    editor.getModel().undo();
});

UiDriver.registerEventHandler("C_CMD_REDO", function(msg, data, prevReturn) {
    editor.getModel().redo();
});

UiDriver.registerEventHandler("C_CMD_CLEAR_HISTORY", function(msg, data, prevReturn) {
    // Monaco has no clearHistory; no-op
});

UiDriver.registerEventHandler("C_CMD_SET_LINE_WRAP", function(msg, data, prevReturn) {
    editor.updateOptions({ wordWrap: data ? "on" : "off" });
});

UiDriver.registerEventHandler("C_CMD_SHOW_END_OF_LINE", function(msg, data, prevReturn) {
    editor.updateOptions({ renderWhitespace: data ? "all" : "selection" });
});

UiDriver.registerEventHandler("C_CMD_SHOW_WHITESPACE", function(msg, data, prevReturn) {
    editor.updateOptions({ renderWhitespace: data ? "all" : "selection" });
});

UiDriver.registerEventHandler("C_CMD_SET_TABS_VISIBLE", function(msg, data, prevReturn) {
    editor.updateOptions({ renderWhitespace: data ? "all" : "selection" });
});

UiDriver.registerEventHandler("C_FUN_SEARCH", function(msg, data, prevReturn) {
    return Search(data[0], data[1], data[2]);
});

UiDriver.registerEventHandler("C_FUN_REPLACE", function(msg, data, prevReturn) {
    var regexStr = data[0];
    var regexModifiers = data[1];
    var forward = data[2];
    var replacement = data[3];
    var searchMode = Number(data[4]);

    if (editor.getSelection() && !editor.getSelection().isEmpty()) {
        var sel = editor.getSelection();
        var selectedText = editor.getModel().getValueInRange(sel);
        if (searchMode === SearchMode.Regex && hasGroupReuseTokens(replacement)) {
            var searchRegex = new RegExp(regexStr, regexModifiers);
            var groups = searchRegex.exec(selectedText);
            if (groups !== null) {
                editor.executeEdits("replace", [{
                    range: sel,
                    text: applyReusedGroups(replacement, groups)
                }]);
            }
        } else {
            editor.executeEdits("replace", [{
                range: sel,
                text: replacement
            }]);
        }
    }

    return Search(regexStr, regexModifiers, forward);
});

UiDriver.registerEventHandler("C_FUN_REPLACE_ALL", function(msg, data, prevReturn) {
    return searchAndReplaceAll(data[0], data[1], data[2], Number(data[3]));
});

UiDriver.registerEventHandler("C_FUN_SEARCH_SELECT_ALL", function(msg, data, prevReturn) {
    var model = editor.getModel();
    var content = model.getValue();
    var searchRegex = new RegExp(data[0], data[1]);
    var selections = [];
    var match;
    searchRegex.lastIndex = 0;
    while ((match = searchRegex.exec(content)) !== null) {
        var start = model.getPositionAt(match.index);
        var end = model.getPositionAt(match.index + match[0].length);
        selections.push(new monaco.Selection(start.lineNumber, start.column, end.lineNumber, end.column));
    }
    if (selections.length > 0) {
        editor.setSelections(selections);
    }
    return selections.length;
});

UiDriver.registerEventHandler("C_FUN_GET_LANGUAGES", function(msg, data, prevReturn) {
    return Languages.languages;
});

var _loadedThemes = {};

function loadTheme(name, callback) {
    if (_loadedThemes[name]) { callback(); return; }
    var s = document.createElement("script");
    s.src = "libs/monaco-addons/themes/" + name + ".js";
    s.onload = function() { _loadedThemes[name] = true; callback(); };
    s.onerror = callback;
    document.body.appendChild(s);
}

UiDriver.registerEventHandler("C_CMD_SET_THEME", function(msg, data, prevReturn) {
    var name = data.name || "vs";
    if (_builtinThemes.indexOf(name) >= 0) {
        monaco.editor.setTheme(name);
    } else {
        loadTheme(name, function() {
            monaco.editor.setTheme(name);
        });
    }
});

UiDriver.registerEventHandler("C_CMD_SET_FONT", function(msg, data, prevReturn) {
    var fontSize = (data.size != "" && data.size > 0) ? ("font-size:" + (+data.size) + "px;") : "";
    var fontFamily = data.family ? ("font-family:'" + ('' + data.family).replace("'", "\\'") + "';") : "";
    var lineHeight = (data.lineHeight != "" && data.lineHeight > 0) ? ("line-height:" + (+data.lineHeight) + "em;") : "";

    var styleTag = document.getElementById('userFont');
    if (styleTag) {
        styleTag.innerHTML = ".monaco-editor { " + fontFamily + fontSize + lineHeight + " }";
    } else {
        styleTag = document.createElement("style");
        styleTag.id = 'userFont';
        styleTag.innerHTML = ".monaco-editor { " + fontFamily + fontSize + lineHeight + " }";
        document.getElementsByTagName("head")[0].appendChild(styleTag);
    }
});

UiDriver.registerEventHandler("C_CMD_SET_LINE_NUMBERS_VISIBLE", function(msg, data, prevReturn) {
    editor.updateOptions({ lineNumbers: data ? "on" : "off" });
});

UiDriver.registerEventHandler("C_CMD_SET_OVERWRITE", function(msg, data, prevReturn) {
    // Monaco 0.52+ has overtype via editor.updateOptions({overtypeAfter: ...}) but not directly; no-op
});

UiDriver.registerEventHandler("C_CMD_SET_SMART_INDENT", function(msg, data, prevReturn) {
    editor.updateOptions({ autoIndent: data ? "full" : "none" });
    return data;
});

UiDriver.registerEventHandler("C_CMD_SET_FOCUS", function(msg, data, prevReturn) {
    editor.focus();
});

UiDriver.registerEventHandler("C_CMD_BLUR", function(msg, data, prevReturn) {
    document.activeElement.blur();
});

UiDriver.registerEventHandler("C_FUN_DETECT_INDENTATION_MODE", function(msg, data, prevReturn) {
    var model = editor.getModel();
    var len = model.getLineCount();
    var regexIndented = /^([ ]{2,}|[\t]+)[^ \t]+?/g;

    for (var i = 0; i < len && i < 100; i++) {
        var line = model.getLineContent(i + 1);
        var matches = regexIndented.exec(line);
        if (matches !== null) {
            if (line[0] === "\t") {
                return { found: true, useTabs: true, size: 0 };
            } else {
                var size = matches[1].length;
                if (size === 2 || size === 4 || size === 8) {
                    return { found: true, useTabs: false, size: size };
                } else {
                    return { found: false };
                }
            }
        }
        regexIndented.lastIndex = 0;
    }
    return { found: false };
});

UiDriver.registerEventHandler("C_CMD_DISPLAY_PRINT_STYLE", function(msg, data, prevReturn) {
    document.querySelector(".editor").classList.add("print");
});

UiDriver.registerEventHandler("C_CMD_DISPLAY_NORMAL_STYLE", function(msg, data, prevReturn) {
    document.querySelector(".editor").classList.remove("print");
});

UiDriver.registerEventHandler("C_FUN_GET_CURRENT_WORD", function(msg, data, prevReturn) {
    var pos = editor.getPosition();
    var word = editor.getModel().getWordAtPosition(pos);
    return word ? word.word : "";
});

UiDriver.registerEventHandler("C_CMD_DUPLICATE_LINE", function(msg, data, prevReturn) {
    editor.trigger("keyboard", "editor.action.copyLinesDownAction");
});

UiDriver.registerEventHandler("C_CMD_MOVE_LINE_UP", function(msg, data, prevReturn) {
    editor.trigger("keyboard", "editor.action.moveLinesUpAction");
});

UiDriver.registerEventHandler("C_CMD_MOVE_LINE_DOWN", function(msg, data, prevReturn) {
    editor.trigger("keyboard", "editor.action.moveLinesDownAction");
});

UiDriver.registerEventHandler("C_CMD_TRANSPOSE_LINE", function(msg, data, prevReturn) {
    var pos = editor.getPosition();
    var line = pos.lineNumber;
    if (line > 1) {
        var model = editor.getModel();
        var lineContent = model.getLineContent(line);
        var prevLineContent = model.getLineContent(line - 1);
        model.pushEditOperations(
            [],
            [
                { range: new monaco.Range(line - 1, 1, line - 1, prevLineContent.length + 1), text: lineContent + "\n" },
                { range: new monaco.Range(line, 1, line, lineContent.length + 1), text: prevLineContent }
            ],
            function() { return [new monaco.Selection(line, pos.column, line, pos.column)]; }
        );
    }
});

UiDriver.registerEventHandler("C_CMD_DELETE_LINE", function(msg, data, prevReturn) {
    editor.trigger("keyboard", "editor.action.deleteLines");
});

UiDriver.registerEventHandler("C_CMD_TRIM_LEADING_TRAILING_SPACE", function(msg, data, prevReturn) {
    editLines(function(x) { return x.trim(); });
});

UiDriver.registerEventHandler("C_CMD_TRIM_TRAILING_SPACE", function(msg, data, prevReturn) {
    editLines(function(x) { return x.replace(/\s+$/, ""); });
});

UiDriver.registerEventHandler("C_CMD_TRIM_LEADING_SPACE", function(msg, data, prevReturn) {
    editLines(function(x) { return x.replace(/^\s+/, ""); });
});

UiDriver.registerEventHandler("C_CMD_ENABLE_MATH", function(msg, data, prevReturn) {
    // Monaco doesn't support LaTeX math rendering; no-op
});

UiDriver.registerEventHandler("C_FUN_IS_MATH_ENABLED", function(msg, data, prevReturn) {
    return false;
});

var tabToSpaceCounter = 0;
function tabToSpaceHelper(match, offset, tabSize) {
    var trueOffset = offset + tabToSpaceCounter;
    var numSpaces = tabSize - (trueOffset % tabSize);
    tabToSpaceCounter += numSpaces - 1;
    var space = "";
    for (var i = 0; i < numSpaces; i++) space += " ";
    return space;
}

UiDriver.registerEventHandler("C_CMD_TAB_TO_SPACE", function(msg, data, prevReturn) {
    editLines(function(x) {
        tabToSpaceCounter = 0;
        var tabSz = editor.getModel().getOptions().tabSize;
        return x.replace(/\t/g, function(match, offset) {
            return tabToSpaceHelper(match, offset, tabSz);
        });
    });
});

var spaceToTabCounter = 0;
function spaceToTabHelper(match, offset, tabSize) {
    var start = offset + spaceToTabCounter;
    var len = match.length;
    var result = "";
    var leading = tabSize - (start % tabSize);
    if (len >= leading) {
        result += "\t";
        len -= leading;
    }
    while (len >= tabSize) {
        result += "\t";
        len -= tabSize;
    }
    while (len > 0) {
        result += " ";
        len -= 1;
    }
    spaceToTabCounter -= (match.length - result.length);
    return result;
}

UiDriver.registerEventHandler("C_CMD_SPACE_TO_TAB_ALL", function(msg, data, prevReturn) {
    editLines(function(x) {
        spaceToTabCounter = 0;
        var tabSz = editor.getModel().getOptions().tabSize;
        return x.replace(/ +/g, function(match, offset) {
            return spaceToTabHelper(match, offset, tabSz);
        });
    });
});

UiDriver.registerEventHandler("C_CMD_SPACE_TO_TAB_LEADING", function(msg, data, prevReturn) {
    editLines(function(x) {
        spaceToTabCounter = 0;
        var tabSz = editor.getModel().getOptions().tabSize;
        return x.replace(/^ +/g, function(match, offset) {
            return spaceToTabHelper(match, offset, tabSz);
        });
    });
});

UiDriver.registerEventHandler("C_CMD_EOL_TO_SPACE", function(msg, data, prevReturn) {
    var text = editor.getModel().getValue();
    editor.getModel().setValue(text.replace(/\n/gm, " "));
});

UiDriver.registerEventHandler("C_CMD_GET_DOCUMENT_INFO", function(msg, data, prevReturn) {
    UiDriver.sendMessage("J_EVT_DOCUMENT_INFO", getDocumentInfo());
});

// ── Initialization ──

var _builtinThemes = ["vs", "vs-dark", "hc-black", "hc-light"];

$(document).ready(function () {
    // Apply any themes that were queued before monaco was fully loaded
    if (window.__pendingThemes) {
        for (var name in window.__pendingThemes) {
            if (window.__pendingThemes.hasOwnProperty(name)) {
                monaco.editor.defineTheme(name, window.__pendingThemes[name]);
            }
        }
        delete window.__pendingThemes;
    }

    var initialTheme = _defaultTheme === "default" ? "vs" : (_defaultTheme || "vs");

    editor = monaco.editor.create(document.querySelector(".editor"), {
        value: "",
        language: "plaintext",
        lineNumbers: "on",
        renderLineHighlight: "all",
        matchBrackets: true,
        folding: true,
        minimap: { enabled: false },
        scrollBeyondLastLine: false,
        automaticLayout: true,
        theme: initialTheme,
        wordWrap: "off",
        fixedOverflowWidgets: true
    });

    monacoModel = editor.getModel();

    cleanVersionId = 0;
    currentVersionId = 0;

    editor.onDidChangeModelContent(function() {
        currentVersionId++;
        onChange();
    });

    editor.onDidChangeCursorPosition(function() {
        onCursorActivity();
    });

    editor.onDidFocusEditorText(function() {
        UiDriver.sendMessage("J_EVT_GOT_FOCUS");
    });

    editor.focus();

    UiDriver.sendMessage("J_EVT_READY", null);
});
