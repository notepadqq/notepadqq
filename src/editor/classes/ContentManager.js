"use strict";
/**
 * @brief Contains functions related to the content and state of the current
 *        editor context.
 */

class ContentManager {
    constructor() {
        this.forceDirty = false;
        this.changeGeneration = false;
        this.previousCleanState = undefined;
    }

    /**
     * @brief Returns true if the document is clean and forceDirty is false.
     * @return bool
     */
    isCleanOrForced(generation)
    {
        return !this.forceDirty && editor.isClean(generation);
    }

    /**
     * @brief Checks whether the clean state changed and updates
     *        this.previousCleanState accordingly.  This keeps needlessly
     *        sending clean state events through the CPP proxy.
     * @return bool True if the clean state changed, false otherwise.
     */
    cleanStateChanged()
    {
        var cleanOrForced = this.isCleanOrForced(this.changeGeneration);
        if (this.previousCleanState != cleanOrForced) {
            this.previousCleanState = cleanOrForced;
            return true;
        }
        return false;
    }

    /**
     * @brief Forces the editor content to be "dirty" if on is true.
     *        This is mainly used for session/file loading.
     */
    setForceDirty(on)
    {
        this.forceDirty = on;
    }

    /**
     * @brief Sets the change generation for the current editor content for use with
     *        isCleanOrForced
     */
    setChangeGeneration(on)
    {
        this.changeGeneration = on;
    }

    /**
     * @brief Detects the current indentation mode of the editor content.
     * @return array containing the indent mode in the form of [useTabs, size],
     *         or undefined if there was a failure in detection.
     */
    detectIndentationMode()
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
