// Original source: https://github.com/cben/CodeMirror-MathJax#16ee8d496625c11adee264d3136c55da061a15d7

define([], function() {
    var editor = null;
    var doc = null;
    var MathJax = null;

    var InlineMath = {};

    // Position arithmetic
    // -------------------

    var Pos = CodeMirror.Pos;

    // Return negative / 0 / positive.  a < b iff posCmp(a, b) < 0 etc.
    function posCmp(a, b) {
        return (a.line - b.line) || (a.ch - b.ch);
    }

    // True if inside, false if on edge.
    function posInsideRange(pos, fromTo) {
        return posCmp(fromTo.from, pos) < 0 && posCmp(pos, fromTo.to) < 0;
    }

    // True if there is at least one character in common, false if just touching.
    function rangesOverlap(fromTo1, fromTo2) {
        return (posCmp(fromTo1.from, fromTo2.to) < 0 &&
            posCmp(fromTo2.from, fromTo1.to) < 0);
    }

    // Returns whether math parsing is allowed for a formula starting in the specified position.
    // This will, for example, return true if we're within a comment.
    function mathModeAllowed(editor, startPosition) {
        // Plain text
        //if (editor.getMode().name === "null" || editor.getMode().name === null) return true;

        // Allow Markdown if within a simple text portion
        if (editor.getMode().name === "markdown" || editor.getMode().name === "gfm") {
            // For some reason, code blocks are marked as comments. Make sure to
            // not display math in such cases.
            var blacklist = ['comment'];
            return blacklist.indexOf(editor.getTokenTypeAt(startPosition)) === -1;
        }

        // Allow any other language if within a comment
        if (editor.getTokenTypeAt(startPosition) == "comment") return true; // TODO: use more precise "getTokenAt"?

        return false;
    }

    function catchAllErrors(func) {
        return function (var_args) {
            try {
                return func.apply(this, arguments);
            } catch (err) {
                console.error("catching error: " + err);
            }
        }
    }

    // If cursor is inside a formula, we don't render it until the
    // cursor leaves it.  To cleanly detect when that happens we
    // still markText() it but without replacedWith and store the
    // marker here.
    var unrenderedMath = null;

    function unrenderRange(fromTo) {
        if (unrenderedMath !== null) {
            var oldRange = unrenderedMath.find();
            if (oldRange !== undefined) {
                var text = doc.getRange(oldRange.from, oldRange.to);
                //console.error("overriding previous unrenderedMath:", text);
            } else {
                //console.error("overriding unrenderedMath whose .find() === undefined", text);
            }
        }
        unrenderedMath = doc.markText(fromTo.from, fromTo.to);
        unrenderedMath.xMathState = "unrendered"; // helps later remove only our marks.
    }

    function unrenderMark(mark) {
        var range = mark.find();
        if (range === undefined) {
            console.error(mark, "mark.find() === undefined");
        } else {
            unrenderRange(range);
        }
        mark.clear();
    }
    
    // Track currently-edited formula
    // ------------------------------
    // TODO: refactor this to generic simulation of cursor leave events.

    function onCursorActivity(doc) {
        if (unrenderedMath !== null) {
            try {
                // TODO: selection behavior?
                // TODO: handle multiple cursors/selections
                var cursor = doc.getCursor();
                var unrenderedRange = unrenderedMath.find();
                if (unrenderedRange === undefined) {
                    // This happens, not yet sure when and if it's fine.
                    console.error(unrenderedMath, ".find() === undefined");
                    return;
                }
                if (posInsideRange(cursor, unrenderedRange)) {
                    // console.log("cursorActivity", cursor, "in unrenderedRange", unrenderedRange);
                } else {
                    // console.log("cursorActivity", cursor, "left unrenderedRange.", unrenderedRange);
                    unrenderedMath = null;
                    processMath(unrenderedRange.from, unrenderedRange.to);
                    flushTypesettingQueue(flushMarkTextQueue);
                }
            } catch (e) { }
        }
    }

    // Rendering on changes
    // --------------------

    function createMathElement(from, to) {
        var doc = editor.getDoc();
        var text = doc.getRange(from, to);
        var elem = document.createElement("span");
        // Display math becomes a <div> (inside this <span>), which
        // confuses CM badly ("DOM node must be an inline element").
        elem.style.display = "inline-block";
        if (/\\(?:re)?newcommand/.test(text)) {
            // \newcommand{...} would render empty, which makes it hard to enter it for editing.
            text = text + " \\(" + text + "\\)";
        }
        elem.appendChild(document.createTextNode(text));
        elem.title = text;

        var isDisplay = /^\$\$|^\\\[|^\\begin/.test(text);  // TODO: probably imprecise.

        // TODO: style won't be stable given surrounding edits.
        // This appears to work somewhat well but only because we're
        // re-rendering too aggressively (e.g. one line below change)...

        // Sample style one char into the formula, because it's null at
        // start of line.
        var insideFormula = Pos(from.line, from.ch + 1);
        var tokenType = editor.getTokenAt(insideFormula, true).type;
        var className = isDisplay ? "display_math" : "inline_math";
        if (tokenType && !/delim/.test(tokenType)) {
            className += " cm-" + tokenType.replace(/ +/g, " cm-");
        }
        elem.className = className;
        return elem;
    }

    // MathJax returns rendered DOMs asynchroonously.
    // Batch inserting those into the editor to reduce layout & painting.
    // (that was the theory; it didn't noticably improve speed in practice.)
    var markTextQueue = [];
    var flushMarkTextQueue = function() {
        editor.operation(function () {
            for (var i = 0; i < markTextQueue.length; i++) {
                markTextQueue[i]();
            }
            markTextQueue = [];
        });
    };

    // MathJax doesn't support typesetting outside the DOM (https://github.com/mathjax/MathJax/issues/1185).
    // We can't put it into a CodeMirror widget because CM might unattach it when it's outside viewport.
    // So we need a stable invisible place to measure & typeset in.
    var typesettingDiv = null;
    function initializeTypesettingDiv() {
        if (typesettingDiv) return; // Already initialized

        typesettingDiv = document.createElement("div");
        typesettingDiv.style.position = "absolute";
        typesettingDiv.style.height = 0;
        typesettingDiv.style.overflow = "hidden";
        typesettingDiv.style.visibility = "hidden";
        typesettingDiv.className = "CodeMirror-MathJax-typesetting";
        editor.getWrapperElement().appendChild(typesettingDiv);
    }

    // MathJax is much faster when typesetting many formulas at once.
    // Each batch's elements will go into a div under typesettingDiv.
    var typesettingQueueDiv = document.createElement("div");
    var typesettingQueue = [];  // functions to call after typesetting.
    var flushTypesettingQueue = function(callback) {
        var currentDiv = typesettingQueueDiv;
        typesettingQueueDiv = document.createElement("div");
        var currentQueue = typesettingQueue;
        typesettingQueue = [];

        typesettingDiv.appendChild(currentDiv);

        MathJax.Hub.Queue(["Typeset", MathJax.Hub, currentDiv]);
        MathJax.Hub.Queue(function () {
            currentDiv.parentNode.removeChild(currentDiv);
            for (var i = 0; i < currentQueue.length; i++) {
                currentQueue[i]();
            }
            if (callback) {
                callback();
            }
        });
    };

    function processMath(from, to) {
        // By the time typesetting completes, from/to might shift.
        // Use temporary non-widget marker to track the exact range to be replaced.
        var typesettingMark = doc.markText(from, to, { className: "math-typesetting" });
        typesettingMark.xMathState = "typesetting";

        var elem = createMathElement(from, to);
        elem.style.position = "absolute";
        // Unrender the formula when clicking on it
        elem.addEventListener("click", function(){
            editor.focus();
            var x = Math.min(from.ch, to.ch)
            var y = Math.max(from.ch, to.ch)
            editor.setCursor(Pos(from.line, x + Math.ceil((y-x)/2)));
        })
        typesettingDiv.appendChild(elem);

        var text = elem.innerHTML;

        typesettingQueueDiv.appendChild(elem);
        typesettingQueue.push(function () {
            // Done typesetting `text`
            elem.parentNode.removeChild(elem);
            elem.style.position = "static";

            var range = typesettingMark.find();
            if (!range) {
                // Was removed by deletion and/or clearOurMarksInRange().
                return;
            }
            var from = range.from;
            var to = range.to;
            typesettingMark.clear();

            // TODO: behavior during selection?
            var cursor = doc.getCursor();

            if (posInsideRange(cursor, { from: from, to: to })) {
                // This doesn't normally happen during editing, more likely
                // during initial pass.
                //console.error("posInsideRange", cursor, from, to, "=> not rendering");
                unrenderRange({ from: from, to: to });
            } else {
                markTextQueue.push(function () {
                    var mark = doc.markText(from, to, {
                        replacedWith: elem,
                        clearOnEnter: false
                    });
                    mark.xMathState = "rendered"; // helps later remove only our marks.
                    var onBeforeCursorEnter = catchAllErrors(function () {
                        unrenderMark(mark);
                    });
                    CodeMirror.on(mark, "beforeCursorEnter", onBeforeCursorEnter)
                });
            }
        });
    }

    // Process a line, transforming text into formulas. Already rendered formulas
    // are ignored.
    // TODO: multi line \[...\]. Needs an approach similar to overlay modes.
    function processLine(lineHandle) {
        var text = lineHandle.text;
        var line = doc.getLineNumber(lineHandle);
        //console.log("processLine", line, text);

        // TODO: At least unit test this regexp mess.

        // TODO: doesn't handle escaping, e.g. \$.  Doesn't check spaces before/after $ like pandoc.
        // TODO: matches inner $..$ in $$..$ etc.
        // JS has lookahead but not lookbehind.
        // For \newcommand{...} can't match end reliably, just consume till last } on line.

        // this also matches \begin, \newcommand, etc.
        //var formulaRE = /\$\$.*?[^$\\]\$\$|\$.*?[^$\\]\$|\\\(.*?[^$\\]\\\)|\\\[.*?[^$\\]\\\]|\\begin\{([*\w]+)\}.*?\\end{\1}|\\(?:eq)?ref{.*?}|\\(?:re)?newcommand\{.*\}/g;
        var formulaRE = /\$\$.*?[^$\\]\$\$|\$.*?[^$\\]\$|\\\(.*?[^$\\]\\\)|\\\[.*?[^$\\]\\\]/g;
        var match;
        while ((match = formulaRE.exec(text)) != null) {
            var fromCh = match.index;
            var toCh = fromCh + match[0].length;
            var from = Pos(line, fromCh);
            var to = Pos(line, toCh);

            // Check if this formula is already rendered. If so, skip.
            var marks = findOurMarksInRange(from, to);
            if (marks.length === 1 && marks[0].xMathState === "rendered")
                continue;

            // Only process math if we're inside a comment, or markdown text, etc.
            if (mathModeAllowed(editor, from)) {
                processMath(from, to);
            }
        }
    }

    function clearOurMarksInRange(from, to) {
        var ourMarks = findOurMarksInRange(from, to);
        for (var i = 0; i < ourMarks.length; i++) {
            ourMarks[i].clear();
        }
    }

    function findOurMarksInRange(from, to) {
        var oldMarks = doc.findMarks(from, to);
        var ourMarks = [];
        for (var i = 0; i < oldMarks.length; i++) {
            var mark = oldMarks[i];
            if (mark.xMathState === undefined) {
                // not touching foreign mark
                continue;
            }

            // Verify it's in range, even after findMarks() - it returns
            // marks that touch the range, we want at least one char overlap.
            var found = mark.find();
            if (found.line !== undefined ?
                /* bookmark */ posInsideRange(found, { from: from, to: to }) :
                /* marked range */ rangesOverlap(found, { from: from, to: to })) {
                ourMarks.push(mark);
            }
        }
        return ourMarks;
    }

    function clearOurMarks() {
        var oldMarks = doc.getAllMarks();
        for (var i = 0; i < oldMarks.length; i++) {
            var mark = oldMarks[i];
            if (mark.xMathState === undefined) {
                // not touching foreign mark
                continue;
            }

            mark.clear();
        }
    }

    function onChange(doc, changeObj) {
        try {
            // changeObj.{from,to} are pre-change coordinates; adding text.length
            // (number of inserted lines) is a conservative(?) fix.
            // TODO: use cm.changeEnd()

            var endLine = changeObj.to.line + changeObj.text.length + 1;
            clearOurMarksInRange(changeObj.from, changeObj.to);
            doc.eachLine(changeObj.from.line, endLine, processLine);
            flushTypesettingQueue(flushMarkTextQueue);
        } catch (e) { }
    }

    // Make sure stuff doesn't somehow remain in the batching queues.
    /*var int1 = setInterval(function () {
        if (typesettingQueue.length !== 0) {
            flushTypesettingQueue();
        }
    }, 500);
    editor._mathStopSequence.push(function() { clearInterval(int1) })
    var int2 = setInterval(function () {
        if (markTextQueue.length !== 0) {
            flushMarkTextQueue();
        }
    }, 500);
    editor._mathStopSequence.push(function() { clearInterval(int2) })*/

    // Usage: first call InlineMath.hookMath(editor, MathJax),
    // then InlineMath.renderAll() to process initial content.
    // TODO: simplify usage when initial pass becomes cheap.
    // TODO: use defineOption(), support clearing widgets and removing handlers.
    InlineMath.hookMath = function (_editor, _MathJax) {
        if (editor) return;

        editor = _editor;
        doc = editor.getDoc();
        MathJax = _MathJax;

        initializeTypesettingDiv();

        editor.on("cursorActivity", onCursorActivity)
        CodeMirror.on(doc, "change", onChange)
    }

    InlineMath.unhookMath = function () {
        if (!editor) return;

        editor.off("cursorActivity", onCursorActivity)
        CodeMirror.off(doc, "change", onChange)
        clearOurMarks();

        editor = null;
    }

    InlineMath.renderAll = function(callback) {
        if (!editor) return;

        clearOurMarks()
        doc.eachLine(processLine);
        flushTypesettingQueue(function () {
            flushMarkTextQueue();
            // All math rendered.
            if (callback) {
                callback();
            }
        });
    };

    return InlineMath;
})