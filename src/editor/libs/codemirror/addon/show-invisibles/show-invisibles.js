/* global CodeMirror */
/* global define */

(function (mod) {
    'use strict';

    if (typeof exports === 'object' && typeof module === 'object') // CommonJS
        mod(require('../../lib/codemirror'));
    else if (typeof define === 'function' && define.amd) // AMD
        define(['../../lib/codemirror'], mod);
    else
        mod(CodeMirror);
})(function (CodeMirror) {
    'use strict';

    CodeMirror.defineOption('showWhitespace', false, function (cm, val, prev) {
        if (prev === CodeMirror.Init) {
            prev = false;
        }

        if (prev && !val) {
            cm.setOption('flattenSpans', true);
            cm.removeOverlay('whitespace');
            removeStyle('js-show-whitespace');
        } else if (!prev && val) {
            var css = [
                '.cm-whitespace::before {',
                'content: "' + String.fromCharCode(183) + '";',
                'opacity: 0.5;',
                'position:absolute;',
                '}'].join('');
            addStyle(cm, css, 'js-show-whitespace');

            cm.setOption('flattenSpans', false);
            cm.addOverlay({
                name: 'whitespace',
                token: function nextToken(stream) {
                    if (stream.eatWhile(/[^\ ]/)) {
                        return null;
                    }

                    stream.next();
                    return "whitespace";
                }
            });
        }
    });

    CodeMirror.defineOption("showEOL", false, function (cm, val, prev) {
        if (prev === CodeMirror.Init) {
            prev = false;
        }

        if (prev && !val) {
            removeStyle('js-show-eol');
            cm.off('renderLine', renderEOL);
        } else if (!prev && val) {
            var css = [
                '.cm-eol::after{',
                'opacity: 0.5;',
                'display: inline-block;',
                'pointer-events: none;',
                'content: "' + String.fromCharCode(172) + '";',
                '}'].join('');
            addStyle(cm, css, 'js-show-eol');
            cm.on('renderLine', renderEOL);
        }
    });

    function renderEOL(cm, line, elt) {
        // Do not run on the last line.
        if (cm.lineInfo(line).line == cm.lastLine()) return;
        elt.className += ' cm-eol';
    }

    CodeMirror.defineOption("showTab", false, function (cm, val, prev) {
        if (prev === CodeMirror.Init) {
            prev = false;
        }

        if (prev && !val) {
            removeStyle('js-show-tab');
        } else if (!prev && val) {
            var css = [
                '.cm-tab::before{',
                'opacity: 0.5;',
                'display: inline-block;',
                'pointer-events: none;',
                'content: "' + String.fromCharCode(8594) + '";',
                'position: relative;',
                'width: 0;',
                'left: calc(50% - 0.5ch);',
                '}'].join('');
            addStyle(cm, css, 'js-show-tab');
        }
    });

    /**
     * Add a style rule for whitespaces, EOL or tabs.
     * 
     * @param cm          CodeMirror instance
     * @param style       CSS rule (only one)
     * @param styleName   Identifier for this style, so that we can reference it later (e.g. `js-show-whitespace`)
     *                    Should not contain special characters other than a...z A...Z - _ 
     */
    function addStyle(cm, style, styleName) {
        var styleEl = document.createElement('style');
        styleEl.type = 'text/css';
        styleEl.setAttribute('data-name', styleName);
        styleEl.appendChild(document.createTextNode(style));
        document.head.appendChild(styleEl);
    }

    /**
     * Remove a style rule based on its identifier (see `addStyle()`)
     */
    function removeStyle(styleName) {
        var style = document.querySelector('[data-name="' + styleName + '"]');
        document.head.removeChild(style);
    }

});
