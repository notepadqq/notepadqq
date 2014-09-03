var Languages = new function() {

    this.languages = {
        "unknown": {
            name: "Text",
            mode: "null",
            mime: "text/plain",
            fileExtensions: ["txt"]
        },

        "apl": {
            name: "APL",
            mode: "apl",
            mime: "text/apl",
            fileExtensions: ["apl"]
        },

        "asterisk": {
            name: "Asterisk",
            mode: "asterisk",
            mime: "text/x-asterisk",
            fileExtensions: []
        },

        "c": {
            name: "C",
            mode: "clike",
            mime: "text/x-csrc",
            fileExtensions: ["c", "h", "i"]
        },

        "cpp": {
            name: "C++",
            mode: "clike",
            mime: "text/x-c++src",
            fileExtensions: ["cc", "cp", "cpp", "c++", "cxx", "hh", "hpp", "hxx", "h++", "ii", "ino"]
        },

        "cobol": {
            name: "Cobol",
            mode: "cobol",
            mime: "text/x-cobol",
            fileExtensions: ["cob", "cbl"]
        },

        "java": {
            name: "Java",
            mode: "clike",
            mime: "text/x-java",
            fileExtensions: ["java"]
        },

        "csharp": {
            name: "C#",
            mode: "clike",
            mime: "text/x-csharp",
            fileExtensions: ["cs", "asax", "ashx"]
        },

        "scala": {
            name: "Scala",
            mode: "clike",
            mime: "text/x-scala",
            fileExtensions: ["scala", "sbt"]
        },

        "clojure": {
            name: "Clojure",
            mode: "clojure",
            mime: "text/x-clojure",
            fileExtensions: ["clj", "cljs", "cljx"]
        },

        "coffeescript": {
            name: "CoffeeScript",
            mode: "coffeescript",
            mime: "text/x-coffeescript",
            fileExtensions: ["coffee", "cf", "cson", "_coffee"],
            fileNames: ["Cakefile"]
        },

        "commonlisp": {
            name: "Common Lisp",
            mode: "commonlisp",
            mime: "text/x-common-lisp",
            fileExtensions: ["cl", "lisp"]
        },

        "cypher": {
            name: "Cypher",
            mode: "cypher",
            mime: "application/x-cypher-query",
            fileExtensions: ["cypher"]
        },

        "css": {
            name: "CSS",
            mode: "css",
            mime: "text/css",
            fileExtensions: ["css", "css.erb"]
        },

        "d": {
            name: "D",
            mode: "d",
            mime: "text/x-d",
            fileExtensions: ["d"]
        },

        "diff": {
            name: "diff",
            mode: "diff",
            mime: "text/x-diff",
            fileExtensions: ["diff", "patch"]
        },

        "dtd": {
            name: "DTD",
            mode: "dtd",
            mime: "application/xml-dtd",
            fileExtensions: ["dtd"]
        },

        "dylan": {
            name: "Dylan",
            mode: "dylan",
            mime: "text/x-dylan",
            fileExtensions: ["dylan"]
        },



        "scss": {
            name: "SCSS",
            mode: "css",
            mime: "text/x-scss",
            fileExtensions: ["scss", "scss.erb"]
        },

        "html": {
            name: "HTML",
            mode: "htmlmixed",
            mime: "text/html",
            fileExtensions: ["html", "htm", "shtm", "shtml", "xhtml", "cfm", "cfml", "cfc", "dhtml", "xht", "tpl", "twig", "hbs", "handlebars", "kit", "jsp", "aspx", "ascx", "asp", "master", "cshtml", "vbhtml"]
        },

        "javascript": {
            name: "JavaScript",
            mode: "javascript",
            mime: "text/javascript",
            fileExtensions: ["js", "jsx", "js.erb", "jsm", "_js"]
        },

        "typescript": {
            name: "TypeScript",
            mode: "javascript",
            mime: "application/typescript",
            fileExtensions: ["ts"]
        },

        "markdown-github": {
            name: "Markdown (GitHub-flavour)",
            mode: "markdown",
            mime: "text/x-markdown",
            fileExtensions: ["md", "markdown"]
        },

        "php": {
            name: "PHP",
            mode: "php",
            mime: "application/x-httpd-php",
            fileExtensions: ["php", "php3", "php4", "php5", "phtm", "phtml", "ctp"]
        },

        "python": {
            name: "Python",
            mode: "python",
            mime: "text/x-python",
            fileExtensions: ["py", "pyw", "wsgi"]
        },

        "ruby": {
            name: "Ruby",
            mode: "ruby",
            mime: "text/x-ruby",
            fileExtensions: ["rb", "ru", "gemspec", "rake"],
            fileNames: ["Gemfile", "Rakefile", "Guardfile"]
        },

        "xml": {
            name: "XML",
            mode: "xml",
            mime: "application/xml",
            fileExtensions: ["xml", "svg", "wxs", "wxl", "wsdl", "rss", "atom", "rdf", "xslt", "xsl", "xul", "xbl", "mathml", "config", "plist", "xaml"]
        },

        "z80": {
            name: "Z80",
            mode: "z80",
            mime: "text/x-z80",
            fileExtensions: []
        },

    }

    this.languageByFileName = function(filename) {
        for (var id in this.languages) { 
            if (this.languages.hasOwnProperty(id)) {
                var lang = this.languages[id];
                
                // Case-insensitive search for extension match
                if (lang.fileExtensions !== undefined) {
                    for (var i = 0; i < lang.fileExtensions.length; i++) {
                        if (endsWith(filename.toLowerCase(), "." + lang.fileExtensions[i].toLowerCase()))
                            return lang;
                    }
                }
                
                // Case-sensitive search for file name match
                if (lang.fileNames !== undefined) {
                    for (var i = 0; i < lang.fileNames.length; i++) {
                        if (filename === lang.fileNames[i] || endsWith(filename, "/" + lang.fileNames[i]) || endsWith(filename, "\\" + lang.fileNames[i]))
                            return lang;
                    }
                }
            }
        }

        return this.languages["unknown"];
    }
    
    this.setLanguage = function(editor, language) {
        if (language.mime !== undefined && language.mime !== null && language.mime !== "")
            editor.setOption("mode", language.mime);
        else
            editor.setOption("mode", language.mode);
    }
    
    function endsWith(str, suffix) {
        return str.indexOf(suffix, str.length - suffix.length) !== -1;
    }

}
