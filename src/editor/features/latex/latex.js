define([], function () {

    function appendConfig() {
        if ($("head > script[type='text/x-mathjax-config']").length === 0) {
            // It doesn't exist yet; we can add it.
    
            var head = document.getElementsByTagName("head")[0];
            var script = document.createElement("script");
            script.type = "text/x-mathjax-config";
    
            var config = {
                jax: ["input/TeX", "output/CommonHTML"], // TODO: Use HTML-CSS when on WebEngine
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

    obj.enable = function(editor) {
        // Put configuration object inside head *before* loading MathJax.
        appendConfig();

        enabled = true;

        requirejs(['features/latex/render-math', 'libs/MathJax/MathJax'], function(InlineMath){
            InlineMath.hookMath(editor, window.MathJax);
            InlineMath.renderAll();
        })
    }

    obj.disable = function(editor) {
        enabled = false;
        requirejs(['features/latex/render-math'], function(InlineMath){
            InlineMath.unhookMath();
        })
    }

    obj.isEnabled = function() {
        return enabled;
    }

    obj.refresh = function(editor) {
        if (obj.isEnabled) {
            requirejs(['features/latex/render-math'], function(InlineMath){
                InlineMath.renderAll();
            })
        }
    }

    return obj;
});