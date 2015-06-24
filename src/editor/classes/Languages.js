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

        "asn.1": {
            name: "ASN.1",
            mode: "asn.1",
            mime: "text/x-ttcn-asn",
            fileExtensions: []
        },

        "pgp": {
            name: "PGP",
            mode: "asciiarmor",
            mime: "application/pgp",
            fileExtensions: ["pgp"]
        },

        "asterisk": {
            name: "Asterisk",
            mode: "asterisk",
            mime: "text/x-asterisk",
            fileExtensions: ["agi"],
            fileNames: ["extensions.conf", "extensions_custom.conf", "extensions_general.conf", "extensions_globals.conf"]
        },

        "c": {
            name: "C",
            mode: "clike",
            mime: "text/x-csrc",
            fileExtensions: ["c", "h", "i", "xbm", "xpm"]
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

        "cmake": {
            name: "CMake",
            mode: "cmake",
            mime: "text/x-cmake",
            fileExtensions: ["cmake", "cmake.in"],
            fileNames: ["CMakeLists.txt"]
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
            fileExtensions: ["cyp", "cypher"]
        },

        "cython": {
            name: "Cython",
            mode: "python",
            mime: "text/x-cython",
            fileExtensions: ["pyx", "pxd", "pxi"]
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

        "dart": {
            name: "Dart",
            mode: "dart",
            mime: "application/dart",
            fileExtensions: ["dart"]
        },

        "diff": {
            name: "diff",
            mode: "diff",
            mime: "text/x-diff",
            fileExtensions: ["diff", "patch"]
        },

        "django": {
            name: "Django",
            mode: "django",
            mime: "text/x-django",
            fileExtensions: []
        },

        "dockerfile": {
            name: "Dockerfile",
            mode: "dockerfile",
            mime: "text/x-dockerfile",
            fileExtensions: [],
            fileNames: ["Dockerfile"]
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

        "ebnf": {
            name: "EBNF",
            mode: "ebnf",
            mime: "text/x-ebnf",
            fileExtensions: []
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

        "erb": {
            name: "Embedded Ruby",
            mode: "htmlembedded",
            mime: "application/x-erb",
            fileExtensions: ["erb"]
        },

        "erlang": {
            name: "Erlang",
            mode: "erlang",
            mime: "text/x-erlang",
            fileExtensions: ["erl", "hrl"]
        },

        "forth": {
            name: "Forth",
            mode: "forth",
            mime: "text/x-forth",
            fileExtensions: ["forth", "fth", "4th"]
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

        "markdown-github": {
            name: "Markdown (GitHub-flavour)",
            mode: "gfm",
            mime: "text/x-gfm",
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

        "idl": {
            name: "IDL",
            mode: "idl",
            mime: "text/x-idl",
            fileExtensions: []
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

        "jsonld": {
            name: "JSON-LD",
            mode: "javascript",
            mime: "application/ld+json",
            fileExtensions: ["jsonld"]
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

        "m4": {
            name: "m4",
            mode: "m4",
            mime: "application/x-m4",
            fileExtensions: ["m4"],
            fileNames: ["configure.ac"]
        },

        "makefile": {
            name: "Makefile",
            mode: "makefile",
            mime: "text/x-makefile",
            fileExtensions: ["mak", "make", "mk", "mke", "mkg", "am", "pro"],
            fileNames: ["Makefile", "Makefile.in", "GNUmakefile", "rules"]
        },

        "markdown": {
            name: "Markdown",
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

        "modelica": {
            name: "Modelica",
            mode: "modelica",
            mime: "text/x-modelica",
            fileExtensions: ["mo"]
        },

        "mumps": {
            name: "MUMPS",
            mode: "mumps",
            mime: "text/x-mumps",
            fileExtensions: ["mo"]
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

        "objective_c": {
            name: "Objective C",
            mode: "clike",
            mime: "text/x-objectivec",
            fileExtensions: ["m", "mm"]
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
            fileExtensions: ["pl", "p6", "pdl", "ph", "pm"],
            firstNonBlankLine: [/^#!.*\/perl($| )/, /^#!\/usr\/bin\/env perl($| )/]
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

        "plsql": {
            name: "PLSQL",
            mode: "sql",
            mime: "text/x-plsql",
            fileExtensions: ["pls"]
        },

        "properties": {
            name: "Properties files",
            mode: "properties",
            mime: "text/x-properties",
            fileExtensions: ["properties", "desktop", "theme", "ini", "la"]
        },

        "python": {
            name: "Python",
            mode: "python",
            mime: "text/x-python",
            fileExtensions: ["py", "pyd", "pyw", "vpy", "wsgi"],
            firstNonBlankLine: [/^#!.*\/python[\d\.]*($| )/, /^#!\/usr\/bin\/env python[\d\.]*($| )/]
        },

        "puppet": {
            name: "Puppet",
            mode: "puppet",
            mime: "text/x-puppet",
            fileExtensions: ["pp"]
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

        "rpm_changes": {
            name: "RPM Changes",
            mode: "rpm",
            mime: "text/x-rpm-changes",
            fileExtensions: []
        },

        "rpm_spec": {
            name: "RPM Spec",
            mode: "rpm",
            mime: "text/x-rpm-spec",
            fileExtensions: ["spec"]
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
            fileExtensions: ["sass"]
        },

        "scheme": {
            name: "Scheme",
            mode: "scheme",
            mime: "text/x-scheme",
            fileExtensions: ["xcscheme", "scm", "ss"]
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
            fileExtensions: ["sh", "shr", "shar"],
            firstNonBlankLine: [/^#!.*\/sh($| )/, /^#!\/usr\/bin\/env sh($| )/,
                                /^#!.*\/bash($| )/, /^#!\/usr\/bin\/env bash($| )/,
                                /^#!.*\/ksh($| )/, /^#!\/usr\/bin\/env ksh($| )/,
                                /^#!.*\/csh($| )/, /^#!\/usr\/bin\/env csh($| )/,
                                /^#!.*\/tcsh($| )/, /^#!\/usr\/bin\/env tcsh($| )/,
                                /^#!.*\/zsh($| )/, /^#!\/usr\/bin\/env zsh($| )/,
                                /^#!.*\/fish($| )/, /^#!\/usr\/bin\/env fish($| )/]
        },

        "sieve": {
            name: "Sieve",
            mode: "sieve",
            mime: "application/sieve",
            fileExtensions: ["siv", "sieve"]
        },

        "slim": {
            name: "Slim",
            mode: "slim",
            mime: "text/x-slim",
            fileExtensions: ["slim"]
        },

        "smalltalk": {
            name: "Smalltalk",
            mode: "smalltalk",
            mime: "text/x-stsrc",
            fileExtensions: ["st"]
        },

        "smarty": {
            name: "Smarty",
            mode: "smarty",
            mime: "text/x-smarty",
            fileExtensions: ["tpl"]
        },

        "solr": {
            name: "Solr",
            mode: "solr",
            mime: "text/x-solr",
            fileExtensions: []
        },

        "soy": {
            name: "Soy",
            mode: "soy",
            mime: "text/x-soy",
            fileExtensions: ["soy"]
        },

        "sparql": {
            name: "SPARQL",
            mode: "sparql",
            mime: "application/x-sparql-query",
            fileExtensions: ["sparql"]
        },

        "spreadsheet": {
            name: "Spreadsheet",
            mode: "spreadsheet",
            mime: "text/x-spreadsheet",
            fileExtensions: ["excel", "formula"]
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

        "mathematica": {
            name: "Mathematica",
            mode: "mathematica",
            mime: "text/x-mathematica",
            fileExtensions: ["m", "nb"]
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

        "textile": {
            name: "Textile",
            mode: "textile",
            mime: "text/x-textile",
            fileExtensions: ["textile"]
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
            fileExtensions: ["toml"]
        },

        "turtle": {
            name: "Turtle",
            mode: "turtle",
            mime: "text/turtle",
            fileExtensions: ["ttl"]
        },

        "tornado": {
            name: "Tornado",
            mode: "tornado",
            mime: "text/x-tornado",
            fileExtensions: []
        },

        "troff": {
            name: "troff",
            mode: "troff",
            mime: "troff",
            fileExtensions: ["1", "2", "3", "4", "5", "6", "7", "8", "9"]
        },

        "ttcn": {
            name: "TTCN",
            mode: "ttcn",
            mime: "text/x-ttcn",
            fileExtensions: []
        },

        "ttcn-cfg": {
            name: "TTCN-CFG",
            mode: "ttcn-cfg",
            mime: "text/x-ttcn-cfg",
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
            fileExtensions: ["xml", "svg", "wxs", "wxl", "wsdl", "rss", "atom", "rdf", "xslt", "xsl", "xul", "xbl", "mathml", "config", "plist", "xaml", "qrc"],
            firstNonBlankLine: [/^<\?xml /, /^<.+?>/]
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
            fileExtensions: ["yaml", "yml"]
        },

        "z80": {
            name: "Z80",
            mode: "z80",
            mime: "text/x-z80",
            fileExtensions: ["z80"]
        },

    }

    this.languageByFileName = function(editor, filename) {

        // Case-sensitive search for file name match
        for (var id in this.languages) { 
            if (this.languages.hasOwnProperty(id)) {
                var lang = this.languages[id];
                if (lang.fileNames !== undefined) {
                    for (var i = 0; i < lang.fileNames.length; i++) {
                        if (filename === lang.fileNames[i] || endsWith(filename, "/" + lang.fileNames[i]) || endsWith(filename, "\\" + lang.fileNames[i]))
                            return id;
                    }
                }
            }
        }

        // Case-insensitive search for extension match
        for (var id in this.languages) { 
            if (this.languages.hasOwnProperty(id)) {
                var lang = this.languages[id];
                if (lang.fileExtensions !== undefined) {
                    for (var i = 0; i < lang.fileExtensions.length; i++) {
                        if (endsWith(filename.toLowerCase(), "." + lang.fileExtensions[i].toLowerCase()))
                            return id;
                    }
                }
            }
        }

        // First non blank line match
        for (var id in this.languages) { 
            if (this.languages.hasOwnProperty(id)) {
                var lang = this.languages[id];
                if (lang.firstNonBlankLine !== undefined &&
                    editor !== undefined && editor !== null) {

                    // Find the first non blank line
                    var lineCount = editor.lineCount();
                    var line = null;
                    for (var i = 0; i < lineCount && line === null; i++) {
                        var iLine = editor.getLine(i);
                        if (iLine.trim() !== "")
                            line = iLine;
                    }

                    if (line !== null) {
                        for (var i = 0; i < lang.firstNonBlankLine.length; i++) {
                            if (line.match(lang.firstNonBlankLine[i]))
                                return id;
                        }
                    }
                }
            }
        }

        return "plaintext";
    }
    
    this.setLanguage = function(editor, languageId) {
        if (languageId === "" || this.languages[languageId] === undefined)
            languageId = "plaintext";

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
