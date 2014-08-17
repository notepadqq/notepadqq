var editor;
var generation;

UiDriver.registerEventHandler("C_CMD_SET_VALUE", function(msg, data, prevReturn) {
    editor.setValue(data);
});

UiDriver.registerEventHandler("C_FUN_GET_VALUE", function(msg, data, prevReturn) {
    return editor.getValue("\n");
});

UiDriver.registerEventHandler("C_CMD_MARK_CLEAN", function(msg, data, prevReturn) {
    generation = editor.changeGeneration(true);
    UiDriver.sendMessage("J_EVT_CLEAN_CHANGED", editor.isClean(generation));
});

UiDriver.registerEventHandler("C_FUN_IS_CLEAN", function(msg, data, prevReturn) {
    return editor.isClean(generation);
});

UiDriver.registerEventHandler("C_CMD_SET_LANGUAGE", function(msg, data, prevReturn) {
    console.log("setting " + data);
    editor.setOption("mode", data);
});

UiDriver.registerEventHandler("C_FUN_GET_SELECTIONS_TEXT", function(msg, data, prevReturn) {
    return editor.getSelections("\n");
});

UiDriver.registerEventHandler("C_CMD_SET_SELECTION_TEXT", function(msg, data, prevReturn) {
    return editor.replaceSelection(data);
});

UiDriver.registerEventHandler("C_FUN_GET_TEXT_LENGTH", function(msg, data, prevReturn) {
    return editor.getValue("\n").length;
});

UiDriver.registerEventHandler("C_FUN_GET_LINE_COUNT", function(msg, data, prevReturn) {
    return editor.lineCount();
});

$(document).ready(function () {
    editor = CodeMirror($(".editor")[0], {
        autofocus: true,
        lineNumbers: true,
        mode: { name: "" },
    });

    generation = editor.changeGeneration(true);

    editor.on("change", function(instance, changeObj) {
        UiDriver.sendMessage("J_EVT_CONTENT_CHANGED");
        UiDriver.sendMessage("J_EVT_CLEAN_CHANGED", editor.isClean());
    });

    editor.on("cursorActivity", function(instance, changeObj) {
        UiDriver.sendMessage("J_EVT_CURSOR_ACTIVITY");
    });

    UiDriver.sendMessage("J_EVT_READY", null);
});
