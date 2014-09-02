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
        
        "css": {
            name: "CSS",
            mode: "css",
            mime: "text/css",
            fileExtensions: ["css", "css.erb"]
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
                
                if (lang.fileExtensions !== undefined) {
                    for (var i = 0; i < lang.fileExtensions.length; i++) {
                        if (endsWith(filename, "." + lang.fileExtensions[i]))
                            return lang;
                    }
                }
                
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
