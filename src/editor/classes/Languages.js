var Languages = new function() {

    var m_currentLanguage = "plaintext";

    this.languages = {
        "plaintext": {
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
            fileExtensions: ["agi"]
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

        "ecl": {
            name: "ECL",
            mode: "ecl",
            mime: "text/x-ecl",
            fileExtensions: []
        },

        "eiffel": {
            name: "Eiffel",
            mode: "eiffel",
            mime: "text/x-eiffel",
            fileExtensions: []
        },

        "erlang": {
            name: "Erlang",
            mode: "erlang",
            mime: "text/x-erlang",
            fileExtensions: ["erl", "hrl"]
        },

        "fortran": {
            name: "Fortran",
            mode: "fortran",
            mime: "text/x-fortran",
            fileExtensions: ["f", "f90", "for", "fpp", "ftn"]
        },

        "fsharp": {
            name: "F#",
            mode: "mllike",
            mime: "text/x-fsharp",
            fileExtensions: ["fs"]
        },

        "gas": {
            name: "Gas",
            mode: "gas",
            mime: "text/x-gas",
            fileExtensions: []
        },

        "gherkin": {
            name: "Gherkin",
            mode: "gherkin",
            mime: "text/x-feature",
            fileExtensions: []
        },

        "go": {
            name: "Go",
            mode: "go",
            mime: "text/x-go",
            fileExtensions: ["go"]
        },

        "groovy": {
            name: "Groovy",
            mode: "groovy",
            mime: "text/x-groovy",
            fileExtensions: ["gvy", "groovy"]
        },

        "haml": {
            name: "HAML",
            mode: "haml",
            mime: "text/x-haml",
            fileExtensions: ["haml"]
        },

        "haskell": {
            name: "Haskell",
            mode: "haskell",
            mime: "text/x-haskell",
            fileExtensions: ["has", "hs", "lhs", "lit"]
        },

        "haxe": {
            name: "Haxe",
            mode: "haxe",
            mime: "text/x-haxe",
            fileExtensions: ["hx"]
        },

        "aspnet": {
            name: "ASP.NET",
            mode: "htmlembedded",
            mime: "application/x-aspx",
            fileExtensions: ["asp", "aspx"]
        },

        "ejs": {
            name: "Embedded Javascript",
            mode: "htmlembedded",
            mime: "application/x-ejs",
            fileExtensions: ["ejs", "dust"]
        },

        "jsp": {
            name: "JavaServer Pages",
            mode: "htmlembedded",
            mime: "application/x-jsp",
            fileExtensions: ["jsp", "jspx", "jspf", "jst"]
        },

        "html": {
            name: "HTML",
            mode: "htmlmixed",
            mime: "text/html",
            fileExtensions: ["html", "htm", "shtm", "shtml", "xhtml", "cfm", "cfml", "cfc", "dhtml", "xht", "tpl", "twig", "hbs", "handlebars", "kit", "ascx", "master", "cshtml", "vbhtml"]
        },

        "http": {
            name: "HTTP",
            mode: "http",
            mime: "message/http",
            fileExtensions: ["htt"]
        },

        "jade": {
            name: "Jade",
            mode: "jade",
            mime: "text/x-jade",
            fileExtensions: []
        },

        "javascript": {
            name: "JavaScript",
            mode: "javascript",
            mime: "text/javascript",
            fileExtensions: ["js", "jsx", "js.erb", "jsm", "_js"]
        },

        "json": {
            name: "JSON",
            mode: "javascript",
            mime: "application/json",
            fileExtensions: ["json", "geojson", "resjson"]
        },

        "xjson": {
            name: "JSON",
            mode: "javascript",
            mime: "application/x-json",
            fileExtensions: []
        },

        "jsonld": {
            name: "JSON-LD",
            mode: "javascript",
            mime: "application/ld+json",
            fileExtensions: []
        },

        "typescript": {
            name: "TypeScript",
            mode: "javascript",
            mime: "application/typescript",
            fileExtensions: ["ts"]
        },

        "jinja2": {
            name: "Jinja2",
            mode: "jinja2",
            mime: "",
            fileExtensions: []
        },

        "julia": {
            name: "Julia",
            mode: "julia",
            mime: "text/x-julia",
            fileExtensions: ["jl"]
        },

        "kotlin": {
            name: "Kotlin",
            mode: "kotlin",
            mime: "text/x-kotlin",
            fileExtensions: []
        },

        "less": {
            name: "LESS",
            mode: "css",
            mime: "text/x-less",
            fileExtensions: ["less"]
        },

        "livescript": {
            name: "Livescript",
            mode: "livescript",
            mime: "text/x-livescript",
            fileExtensions: []
        },

        "lua": {
            name: "Lua",
            mode: "lua",
            mime: "text/x-lua",
            fileExtensions: ["lua"]
        },

        "makefile": {
            name: "Makefile",
            mode: "makefile",
            mime: "text/x-makefile",
            fileExtensions: ["mak", "make", "mk", "mke", "mkg", "am"],
            fileNames: ["Makefile", "Makefile.in", "GMakefile", "rules"]
        },

        "markdown-github": {
            name: "Markdown (GitHub-flavour)",
            mode: "markdown",
            mime: "text/x-markdown",
            fileExtensions: ["md", "mkdn", "mdown", "markdn", "markdown"]
        },

        "mirc": {
            name: "mIRC",
            mode: "mirc",
            mime: "text/mirc",
            fileExtensions: ["mrc"]
        },

        "nginx": {
            name: "Nginx",
            mode: "nginx",
            mime: "text/x-nginx-conf",
            fileExtensions: []
        },

        "ntriples": {
            name: "NTriples",
            mode: "ntriples",
            mime: "text/n-triples",
            fileExtensions: []
        },

        "ocaml": {
            name: "OCaml",
            mode: "mllike",
            mime: "text/x-ocaml",
            fileExtensions: ["ml", "mli"]
        },

        "octave": {
            name: "Octave",
            mode: "octave",
            mime: "text/x-octave",
            fileExtensions: []
        },

        "pascal": {
            name: "Pascal",
            mode: "pascal",
            mime: "text/x-pascal",
            fileExtensions: ["p", "pp", "pas", "lpr", "dpr"]
        },

        "pegjs": {
            name: "PEG.js",
            mode: "pegjs",
            mime: "",
            fileExtensions: ["pegjs"]
        },

        "perl": {
            name: "Perl",
            mode: "perl",
            mime: "text/x-perl",
            fileExtensions: ["pl", "p6", "pdl", "ph", "pm"]
        },

        "php": {
            name: "PHP",
            mode: "php",
            mime: "application/x-httpd-php",
            fileExtensions: ["php", "php3", "php4", "php5", "phtm", "phtml", "ctp"]
        },

        "pig": {
            name: "Pig",
            mode: "pig",
            mime: "text/x-pig",
            fileExtensions: ["pig"]
        },

        "properties": {
            name: "Properties files",
            mode: "properties",
            mime: "text/x-properties",
            fileExtensions: ["properties", "desktop", "theme", "ini"]
        },

        "python": {
            name: "Python",
            mode: "python",
            mime: "text/x-python",
            fileExtensions: ["py", "pyd", "pyw", "wsgi"]
        },

        "cython": {
            name: "Cython",
            mode: "python",
            mime: "text/x-cython",
            fileExtensions: ["pyx", "pxi", "pxd"]
        },

        "puppet": {
            name: "Puppet",
            mode: "puppet",
            mime: "text/x-puppet",
            fileExtensions: []
        },

        "r": {
            name: "R",
            mode: "r",
            mime: "text/x-rsrc",
            fileExtensions: ["r"]
        },

        "rst": {
            name: "reStructuredText",
            mode: "rst",
            mime: "text/x-rst",
            fileExtensions: ["rst"]
        },

        "ruby": {
            name: "Ruby",
            mode: "ruby",
            mime: "text/x-ruby",
            fileExtensions: ["rb", "ru", "gemspec", "rake"],
            fileNames: ["Gemfile", "Rakefile", "Guardfile"]
        },

        "rust": {
            name: "Rust",
            mode: "rust",
            mime: "text/x-rustsrc",
            fileExtensions: ["rs"]
        },

        "sass": {
            name: "Sass",
            mode: "sass",
            mime: "text/x-sass",
            fileExtensions: []
        },

        "scheme": {
            name: "Scheme",
            mode: "scheme",
            mime: "text/x-scheme",
            fileExtensions: ["xcscheme"]
        },

        "scss": {
            name: "SCSS",
            mode: "css",
            mime: "text/x-scss",
            fileExtensions: ["scss", "scss.erb"]
        },

        "bash": {
            name: "Bash",
            mode: "shell",
            mime: "text/x-sh",
            fileExtensions: ["sh", "shr", "shar"]
        },

        "sieve": {
            name: "Sieve",
            mode: "sieve",
            mime: "application/sieve",
            fileExtensions: []
        },

        "slim": {
            name: "Slim",
            mode: "slim",
            mime: "text/x-slim",
            fileExtensions: []
        },

        "smalltalk": {
            name: "Smalltalk",
            mode: "smalltalk",
            mime: "text/x-stsrc",
            fileExtensions: []
        },

        "smarty": {
            name: "Smarty",
            mode: "smarty",
            mime: "text/x-smarty",
            fileExtensions: []
        },

        "smartymixed": {
            name: "SmartyMixed",
            mode: "smartymixed",
            mime: "text/x-smarty",
            fileExtensions: []
        },

        "solr": {
            name: "Solr",
            mode: "solr",
            mime: "text/x-solr",
            fileExtensions: []
        },

        "sparql": {
            name: "SPARQL",
            mode: "sparql",
            mime: "application/x-sparql-query",
            fileExtensions: []
        },

        "sql": {
            name: "SQL",
            mode: "sql",
            mime: "text/x-sql",
            fileExtensions: ["sql", "myd"]
        },

        "mariadb": {
            name: "MariaDB",
            mode: "sql",
            mime: "text/x-mariadb",
            fileExtensions: []
        },

        "stex": {
            name: "sTeX",
            mode: "stex",
            mime: "text/x-stex",
            fileExtensions: []
        },

        "latex": {
            name: "LaTeX",
            mode: "stex",
            mime: "text/x-latex",
            fileExtensions: ["tex", "latex", "ltx"]
        },

        "systemverilog": {
            name: "SystemVerilog",
            mode: "verilog",
            mime: "text/x-systemverilog",
            fileExtensions: ["sv"]
        },

        "tcl": {
            name: "Tcl",
            mode: "tcl",
            mime: "text/x-tcl",
            fileExtensions: ["tcl"]
        },

        "tiddlywiki": {
            name: "TiddlyWiki",
            mode: "tiddlywiki",
            mime: "text/x-tiddlywiki",
            fileExtensions: []
        },

        "tiki": {
            name: "Tiki wiki",
            mode: "tiki",
            mime: "text/tiki",
            fileExtensions: []
        },

        "toml": {
            name: "TOML",
            mode: "toml",
            mime: "text/x-toml",
            fileExtensions: []
        },

        "turtle": {
            name: "Turtle",
            mode: "turtle",
            mime: "text/turtle",
            fileExtensions: []
        },

        "vb": {
            name: "VB.NET",
            mode: "vb",
            mime: "text/x-vb",
            fileExtensions: ["vbproj", "vbz"]
        },

        "vbscript": {
            name: "VBScript",
            mode: "vbscript",
            mime: "text/vbscript",
            fileExtensions: ["vbs", "vbe", "wsf", "wsc"]
        },

        "velocity": {
            name: "Velocity",
            mode: "velocity",
            mime: "text/velocity",
            fileExtensions: ["vm", "vsl"]
        },

        "verilog": {
            name: "Verilog",
            mode: "verilog",
            mime: "text/x-verilog",
            fileExtensions: ["v"]
        },

        "xml": {
            name: "XML",
            mode: "xml",
            mime: "application/xml",
            fileExtensions: ["xml", "svg", "wxs", "wxl", "wsdl", "rss", "atom", "rdf", "xslt", "xsl", "xul", "xbl", "mathml", "config", "plist", "xaml", "qrc"]
        },

        "xquery": {
            name: "XQuery",
            mode: "xquery",
            mime: "application/xquery",
            fileExtensions: ["xq", "xql", "xqm", "xquery", "xqy"]
        },

        "yaml": {
            name: "YAML",
            mode: "yaml",
            mime: "text/x-yaml",
            fileExtensions: ["yml", "yaml"]
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
                            return id;
                    }
                }
                
                // Case-sensitive search for file name match
                if (lang.fileNames !== undefined) {
                    for (var i = 0; i < lang.fileNames.length; i++) {
                        if (filename === lang.fileNames[i] || endsWith(filename, "/" + lang.fileNames[i]) || endsWith(filename, "\\" + lang.fileNames[i]))
                            return id;
                    }
                }
            }
        }

        return "plaintext";
    }
    
    this.setLanguage = function(editor, languageId) {
        var lang = this.languages[languageId];
        if (lang.mime !== undefined && lang.mime !== null && lang.mime !== "")
            editor.setOption("mode", lang.mime);
        else
            editor.setOption("mode", lang.mode);
            
        m_currentLanguage = languageId;
        UiDriver.sendMessage("J_EVT_CURRENT_LANGUAGE_CHANGED", {id: languageId, name: lang.name});
    }
    
    this.currentLanguage = function() {
        return m_currentLanguage;
    }
    
    function endsWith(str, suffix) {
        return str.indexOf(suffix, str.length - suffix.length) !== -1;
    }

}
