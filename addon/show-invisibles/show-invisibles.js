/* global CodeMirror */
/* global define */

(function(mod) {
    'use strict';
    
    if (typeof exports === 'object' && typeof module === 'object') // CommonJS
        mod(require('../../lib/codemirror'));
    else if (typeof define === 'function' && define.amd) // AMD
        define(['../../lib/codemirror'], mod);
    else
        mod(CodeMirror);
})(function(CodeMirror) {
    'use strict';
    
    CodeMirror.defineOption('showWhitespace', false, function(cm, val, prev) {
        if (prev === CodeMirror.Init) {
            prev = false;
        }
        
        if (prev && !val) {
            cm.setOption('flattenSpans',true);
            cm.removeOverlay('whitespace');
            rm();
        } else if (!prev && val) {
            add(cm);
            cm.setOption('flattenSpans', false);
            cm.addOverlay({
                name: 'whitespace',
                token:  function nextToken(stream) {
                    if(stream.eatWhile(/[^\ ]/)) {
                        return null;
                    }

                    stream.next();
                    return "whitespace";
                }
            });
        }
    });
    
    function add(cm) {
        var style = document.createElement('style');
        var colour = getStyle(new RegExp("\\.cm-s-" + cm.getOption('theme') + ".*cm-comment"))['color'];
        var spaceChar = String.fromCharCode(183);
        var css = [
            '.cm-whitespace::before {',
            'content: "' + spaceChar + '";',
            'color: ' + colour + ';',
            'position:absolute;',
        '}'].join('');
        style.type = 'text/css';
        style.setAttribute('data-name','js-show-invisibles');
        style.appendChild(document.createTextNode(css));
        document.head.appendChild(style);
    }
    
    function rm() {
        var style = document.querySelector('[data-name="js-show-invisibles]');
        document.head.removeChild(style);
    }
    
    CodeMirror.defineOption("showEOL", false, function(cm, val, prev) {
        if (prev === CodeMirror.Init)
            prev = false;
        if(prev && !val) { 
            var style = document.querySelector('[data-name="js-show-eol"]');
            document.head.removeChild(style);
            cm.off('renderLine',renderEOL);
        }else if(!prev && val) {
            var style = document.createElement('style');
            var colour = getStyle(new RegExp("\\.cm-s-" + cm.getOption('theme') + ".*cm-comment"))['color'];
            var css = [
                '.cm-eol::after{',
                'color: ' + colour + ';',
                'display: inline-block;',
                'pointer-events: none;',
                'content: "' + String.fromCharCode(172) +'";',
                '}'].join('');
            style.type = 'text/css';
            style.setAttribute('data-name','js-show-eol');
            style.appendChild(document.createTextNode(css));
            document.head.appendChild(style);
            cm.on('renderLine',renderEOL);
        }
    });

    function renderEOL(cm,line,elt)
    { 
        //Do not run on the last line.
        if(cm.lineInfo(line).line == cm.lastLine())return;
        elt.className += ' cm-eol';
    }

    function getStyle(selector) 
    {
        var regmatch = false;
        if(selector instanceof RegExp) regmatch = true;
        for(var i=0; i<document.styleSheets.length; i++) {
            var sheet = document.styleSheets[i];
            for(var j=0,k = sheet.cssRules.length;j < k;j++) {
                var rule = sheet.cssRules[j];
                if(rule.selectorText) {
                    if(regmatch && rule.selectorText.match(selector)) {
                        return rule.style;
                    }else if(rule.selectorText.indexOf(selector) !== -1) {
                        return rule.style;
                    }
                }
            }
        }
    }

    CodeMirror.commands.updateLineEndings = function(cm) {
        var colour = getStyle(new RegExp("\\.cm-s-" + cm.getOption('theme') + ".*cm-comment"))['color'];
        var endLines = getStyle('.cm-eol::after');
        var whiteSpace = getStyle('.cm-whitespace::before');
        if(endLines) endLines.color = colour;
        if(whiteSpace) whiteSpace.color = colour;
    };
    
});
