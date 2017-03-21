class StateManager {
    constructor() {
        this.forceDirty = false;
        this.changeGeneration = false;
        this.previousCleanState = undefined;
    }

    isCleanOrForced(generation)
    {
        return !this.forceDirty && editor.isClean(generation);
    }

    cleanStateChanged()
    {
        var cleanOrForced = this.isCleanOrForced(this.changeGeneration);
        if (this.previousCleanState != cleanOrForced) {
            this.previousCleanState = cleanOrForced;
            return true;
        }
        return false;
    }

    setForceDirty(on)
    {
        this.forceDirty = on;
    }

    setChangeGeneration(on)
    {
        this.changeGeneration = on;
    }

    detectIndentationMode(editor)
    {
        var len = editor.lineCount();
        var regexIndented = /^([ ]{2,}|[\t]+)[^ \t]+?/g; // Is not blank, and is indented with tab or space 

        for (var i = 0; i < len && i < 100; i++) {
            var line = editor.getLine(i);
            var matches = regexIndented.exec(line);
            if (matches !== null) {
                if (line[0] === "\t") { // Is a tab
                    return [true, 0]; 
                } else { // Is a space
                    var size = matches[1].length;
                    if (size === 2 || size === 4 || size === 8) {
                        return [false, size];
                    } else {
                        return undefined;
                    }
                }
            }
        }
        return undefined;
    }
}
