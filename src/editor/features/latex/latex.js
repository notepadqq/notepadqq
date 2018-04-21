define([], function () {

    function appendConfig() {
        if ($("head > script[type='text/x-mathjax-config']").length === 0) {
            // It doesn't exist yet; we can add it.
    
            var head = document.getElementsByTagName("head")[0];
            var script = document.createElement("script");
            script.type = "text/x-mathjax-config";
    
            var config = {
                jax: ["input/TeX", "output/CommonHTML"], // TODO: Use HTML-CSS when on WebEngine
                //extensions: ["tex2jax.js","MathMenu.js","MathZoom.js", "fast-preview.js", "AssistiveMML.js", "a11y/accessibility-menu.js"],
                extensions: ["tex2jax.js"],
                tex2jax: {
                    //inlineMath: [ ['$','$'], ["\\(","\\)"] ],
                    //displayMath: [ ['$$','$$'], ["\\[","\\]"] ],
                    inlineMath: [ ['$','$'] ],
                    displayMath: [ ['$$','$$'] ],
                    processEscapes: true,
                    /* Previews of all kinds are useless — math is hidden until fully rendered. */
                    preview: "none"
                },
                /* "CHTML-preview" is for MathJax 2.5, "fast-preview" is for 2.6. */
                //"CHTML-preview": { disabled: true },
                //"fast-preview": { disabled: true },
                "HTML-CSS": {
                    imageFont: null // Prevent the use of image fonts, as we deleted them
                },
                TeX: {
                    noErrors: { disabled: true },
                    equationNumbers: { autoNumber: "AMS" },
                    extensions: ["AMSmath.js","AMSsymbols.js","noErrors.js","noUndefined.js", "autoload-all.js"]
                },
                messageStyle: "none", // hide loading/processing messages
                showMathMenu: false
            }
    
            script.innerHTML = 'MathJax.Hub.Config(' + JSON.stringify(config) + ');';
            head.appendChild(script);
        }
    }

    var enabled = false;

    var obj = {};

    obj.enable = function() {
        // Put configuration object inside head *before* loading MathJax.
        appendConfig();

        enabled = true;

        requirejs(['features/latex/render-math'], function(InlineMath){
            InlineMath.hookMath(editor, window.MathJax);
            editor.renderAllMath();
        })
    }

    obj.disable = function() {
        enabled = false;
        requirejs(['features/latex/render-math'], function(InlineMath){
            InlineMath.unhookMath(editor, window.MathJax);
        })
    }

    obj.isEnabled = function() {
        return enabled;
    }

    return obj;
});