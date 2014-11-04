var Printer = new function() {

    this.displayPrintStyle = function (editorDOM, editor) {
        editorDOM.classList.add("print");
        editor.refresh();
    }

    this.displayNormalStyle = function (editorDOM, editor) {
        editorDOM.classList.remove("print");
        editor.refresh();
    }
}
