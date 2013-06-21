#include "lexerfactory.h"
#include "generalfunctions.h"
#include <QFile>
#include <QFileInfo>
#include <QDomDocument>
#include <QDebug>

// LEXERS
#include <Qsci/qscilexerbash.h>
#include <Qsci/qscilexerbatch.h>
#include <Qsci/qscilexercmake.h>
#include <Qsci/qscilexercpp.h>
#include <Qsci/qscilexercsharp.h>
#include <Qsci/qscilexercss.h>
#include <Qsci/qscilexerd.h>
#include <Qsci/qscilexerdiff.h>
#include <Qsci/qscilexerfortran.h>
#include <Qsci/qscilexerfortran77.h>
#include <Qsci/qscilexerhtml.h>
#include <Qsci/qscilexeridl.h>
#include <Qsci/qscilexerjava.h>
#include <Qsci/qscilexerjavascript.h>
#include <Qsci/qscilexerlua.h>
#include <Qsci/qscilexermakefile.h>
#include <Qsci/qscilexerpascal.h>
#include <Qsci/qscilexerperl.h>
#include <Qsci/qscilexerpostscript.h>
#include <Qsci/qscilexerpov.h>
#include <Qsci/qscilexerproperties.h>
#include <Qsci/qscilexerpython.h>
#include <Qsci/qscilexerruby.h>
#include <Qsci/qscilexerspice.h>
#include <Qsci/qscilexersql.h>
#include <Qsci/qscilexertcl.h>
#include <Qsci/qscilexertex.h>
#include <Qsci/qscilexerverilog.h>
#include <Qsci/qscilexervhdl.h>
#include <Qsci/qscilexerxml.h>
#include <Qsci/qscilexeryaml.h>

#include "mainwindow.h"

namespace stylename
{
    const char* GLOBAL_OVERRIDE     = "Global override";
    const char* DEFAULT             = "Default Style";
    const char* INDENT_GUIDELINE    = "Indent guideline style";
    const char* BRACE_HIGHLIGHT     = "Brace highlight style";
    const char* BAD_BRACE           = "Bad brace colour";
    const char* CURRENT_LINE        = "Current line background colour";
    const char* SELECTED_TEXT       = "Selected text colour";
    const char* CARET               = "Caret colour";
    const char* EDGE                = "Edge colour";
    const char* LINE_NUMBER_MARGIN  = "Line number margin";
    const char* FOLD                = "Fold";
    const char* FOLD_MARGIN         = "Fold margin";
    const char* WHITE_SPACE_SYMBOL  = "White space symbol";
    const char* SMART_HIGHLIGHTING  = "Smart HighLighting";
    const char* FIND_MARK           = "Find Mark Style";
    const char* MARK1               = "Mark Style 1";
    const char* MARK2               = "Mark Style 2";
    const char* MARK3               = "Mark Style 3";
    const char* MARK4               = "Mark Style 4";
    const char* MARK5               = "Mark Style 5";
    const char* INCR_HIGHLIGHT_ALL  = "Incremental highlight all";
    const char* TAG_MATCH_HIGHLIGHT = "Tags match highlighting";
    const char* TAG_ATTRIBUTE       = "Tags attribute";
    const char* ACTV_TAB_FOCUS      = "Active tab focused indicator";
    const char* ACTV_TAB_UNFOCUS    = "Active tab unfocused indicator";
    const char* ACTV_TAB_TEXT       = "Active tab text";
    const char* INACTIVE_TAB        = "Inactive tabs";
}

LexerFactory::LexerFactory()
{
    langDefFile    = generalFunctions::getUserFilePath("langs.xml");
    stylersDefFile = generalFunctions::getUserFilePath("stylers.xml");
}

static QString read_attribute(QDomNode & node, const char* name, const char* def_value)
{
    if ( node.isNull() ) return def_value;
    if ( node.attributes().namedItem(name).isNull() ) return def_value;
    return node.attributes().namedItem(name).nodeValue();
}

static QString read_element_text(QDomNode & node, const char* def_value)
{
    if ( node.isNull() ) return def_value;
    if ( node.toElement().isNull() ) return def_value;
    return node.toElement().text();
}

static QColor hex_to_qcolor(QString hexVal)
{
    QColor c; c.setNamedColor("#" + hexVal);
    return c;
}

bool LexerFactory::setColorSchemeFile(QString filePath)
{
    if ( !QFile(filePath).exists() )
        return false;
    stylersDefFile = filePath;
    return parseColorSchemeDefinitions();
}

bool LexerFactory::parseLanguageDefinitions()
{
    QFile xml(langDefFile);

    qDebug() << "parsing " << langDefFile;

    if ( !xml.exists() ) return false;

    QDomDocument doc("langs");
    doc.setContent(&xml);

    QDomElement docElement = doc.documentElement();   // docElement now refers to the node "xml"
    QDomNode    node;

    node = docElement.firstChildElement("Languages");

    // Not a valid Language Definition file
    if ( node.isNull() ) return false;

    node = node.firstChildElement("Language");
    while(!node.isNull()) {

        ShrPtrLangDefinition lang( new LangDefinition() );
        lang->name            = read_attribute(node, "name" , "");
        lang->file_extensions = read_attribute(node, "ext", "").split(" ", QString::SkipEmptyParts);
        lang->comment_line    = read_attribute(node, "commentLine" , NULL);
        lang->comment_start   = read_attribute(node, "commentStart", NULL);
        lang->comment_end     = read_attribute(node, "commentEnd"  , NULL);
        QDomNode keywords_node = node.firstChildElement("Keywords");
        while(!keywords_node.isNull()) {
            ShrPtrKeywordCollection kw_class( new KeywordCollection() );
            kw_class->type     = read_attribute   (keywords_node, "name", "");
            kw_class->keywords = read_element_text(keywords_node, "").split(" ", QString::SkipEmptyParts);
            keywords_node     = keywords_node.nextSiblingElement("Keywords");

            lang->keywords.append(kw_class);
            lang->keywords_by_class.insert(kw_class->type, kw_class);
        }
        node = node.nextSiblingElement("Language");
        _languages.append        (lang);
        languages_by_name.insert(lang->name, lang);

        foreach(QString ext, lang->file_extensions) {
            language_by_extension.insert(ext, lang); // IF MORE LANGUAGES USES THE SAME EXTENSIONS ( should not! ) THE LAST WINS
        }
    }
    return true;
}


QHash<QString,QString> LexerFactory::languages()
{
    QHash<QString,QString> list;
    foreach(ShrPtrStylerDefinition lang, stylers) {
        list[lang->desc] = lang->name;
    }
    return list;
}

bool LexerFactory::parseColorSchemeDefinitions()
{
    QFile xml(stylersDefFile);

    qDebug() << "parsing " << stylersDefFile;

    if ( !xml.exists() ) return false;

    QDomDocument doc("stylers");
    doc.setContent(&xml);

    QDomElement docElement = doc.documentElement();   // docElement now refers to the node "xml"
    QDomNode    node;

    node = docElement.firstChildElement("LexerStyles");

    // Not a valid Stylers Definition file
    if ( node.isNull() ) return false;

    node = node.firstChildElement("LexerType");
    while(!node.isNull()) {

        ShrPtrStylerDefinition styler( new StylerDefinition() );
        styler->name            = read_attribute(node, "name" , "");
        styler->desc            = read_attribute(node, "desc" , "");
        // I'll ignore the "ext" field, because it's already present in the langs.xml file

        QDomNode words_style_node = node.firstChildElement("WordsStyle");
        while(!words_style_node.isNull()) {
            ShrPtrWordsStyle words_style( new WordsStyle() );

            words_style->name            = read_attribute(words_style_node, "name", "");
            words_style->style_id        = read_attribute(words_style_node, "styleID", "").toInt();
            words_style->fg_color        = hex_to_qcolor (read_attribute(words_style_node, "fgColor", "000000"));
            words_style->bg_color        = hex_to_qcolor (read_attribute(words_style_node, "bgColor", "FFFFFF"));
            words_style->font_name       = read_attribute(words_style_node, "fontName" , "");
            words_style->font_style      = read_attribute(words_style_node, "fontStyle", "").toInt();
            words_style->font_size       = read_attribute(words_style_node, "fontSize" , "").toInt();
            words_style->keyword_class   = read_attribute(words_style_node, "keywordClass" , NULL);

            words_style_node = words_style_node.nextSiblingElement("WordsStyle");

            styler->words_stylers.append(words_style);
            styler->words_stylers_by_name.insert(words_style->name, words_style);
        }
        node = node.nextSiblingElement("LexerType");
        stylers.append        (styler);
        stylers_by_name.insert(styler->name, styler);
    }

    global_styler = ShrPtrStylerDefinition( new StylerDefinition() );

    node = docElement.firstChildElement("GlobalStyles");
    // No global styles
    if ( node.isNull() )  {
        qDebug() << "Global Styles not found";
        return true;
    }

    node = node.firstChildElement("WidgetStyle");
    while(!node.isNull()) {
        ShrPtrWordsStyle widget_style( new WordsStyle() );
        widget_style->name           = read_attribute(node, "name" , "");
        widget_style->style_id       = read_attribute(node, "styleID" , "").toInt();
        widget_style->fg_color        = hex_to_qcolor (read_attribute(node, "fgColor", "000000"));
        widget_style->bg_color        = hex_to_qcolor (read_attribute(node, "bgColor", "FFFFFF"));
        widget_style->font_name       = read_attribute(node, "fontName" , "");
        widget_style->font_style      = read_attribute(node, "fontStyle", "").toInt();
        widget_style->font_size       = read_attribute(node, "fontSize" , "").toInt();
        widget_style->keyword_class   = read_attribute(node, "keywordClass" , NULL);

        qDebug() << "Global style " << widget_style->name;
        node = node.nextSiblingElement("WidgetStyle");
        global_styler->words_stylers.append(widget_style);
        global_styler->words_stylers_by_name.insert(widget_style->name, widget_style);
    }

    return true;
}

bool LexerFactory::init()
{
    return parseLanguageDefinitions() && parseColorSchemeDefinitions();
}

ShrPtrLangDefinition LexerFactory::detectLanguage(QFileInfo info)
{
    // Basic extension-based heuristic
    QString              name = info.fileName();
    QString              ext  = info.completeSuffix();
    ShrPtrLangDefinition ret  = language_by_extension.value(ext);

    //Special case files
    if(!(QString::compare(name, QObject::tr("CmakeLists.txt"),  Qt::CaseInsensitive)))
        ret = languages_by_name.value("cmake");
    else if((!QString::compare(name, QObject::tr("makefile"),   Qt::CaseInsensitive))||
            (!QString::compare(name, QObject::tr("gnumakefile"),Qt::CaseInsensitive)))
        ret = languages_by_name.value("makefile");
    else if((!QString::compare(name, QObject::tr("SConstruct"), Qt::CaseInsensitive))||
            (!QString::compare(name, QObject::tr("SConscript"), Qt::CaseInsensitive))||
            (!QString::compare(name, QObject::tr("wscript"),    Qt::CaseInsensitive)))
        ret = languages_by_name.value("python");
    else if((!QString::compare(name, QObject::tr("Rakefile"),   Qt::CaseInsensitive)))
        ret = languages_by_name.value("ruby");

    // TODO: add more heuristic to detect languages
    // e.g. file starting with "<?xml" ==> "xml"
    return ret;
}

ShrPtrStylerDefinition LexerFactory::getGlobalStyler() const
{
    return global_styler;
}

QsciLexer* LexerFactory::applyColorScheme(ShrPtrLangDefinition lang, QsciLexer* lex)
{
    QFont *f = MainWindow::instance()->systemMonospace();
    if ( lex ) lex->setDefaultFont(*f);

    ShrPtrStylerDefinition styler = stylers_by_name.value(lang->name);
    if ( styler.isNull() || lex == NULL )
        return lex;
    qDebug() << "found styler named " << styler->name << ", " << styler->desc;
    foreach( ShrPtrWordsStyle ws, styler->words_stylers ) {
        if ( ws.isNull() ) continue;
        qDebug() << ws->name << " id: " << ws->style_id << " fg: " << ws->fg_color << " bg:" << ws->bg_color;
        lex->setColor( ws->fg_color, ws->style_id );
        lex->setPaper( ws->bg_color, ws->style_id );
        QFont lf = QFont( lex->defaultFont().family(), lex->defaultFont().pointSize() );

        // support fontstyle
        if ( ws->font_style == 2 ) { // ITALIC
            lf.setItalic(true);
        } else if ( ws->font_style == 3 ) { // BOLD
            lf.setBold(true);
        }
        lex->setFont(lf, ws->style_id);
    }

    ShrPtrWordsStyle ws = global_styler->words_stylers_by_name.value(stylename::DEFAULT);
    if ( !ws.isNull() ) {
        qDebug() << "global background: " << ws->bg_color << " foreground: " << ws->fg_color;
        lex->setDefaultPaper( ws->bg_color );
        lex->setDefaultColor( ws->fg_color );
    }

    return lex;
}

QsciLexer* LexerFactory::createLexer(QFileInfo info, QObject *parent)
{
    ShrPtrLangDefinition lang = detectLanguage(info);
    if ( lang.isNull() )
        return NULL;
    return applyColorScheme( lang, createLexer( lang->name, parent ) );
}

QsciLexer* LexerFactory::createLexerByName(QString lg, QObject* parent)
{
    ShrPtrLangDefinition lang = languages_by_name.value(lg);
    if ( lang.isNull() )
        return NULL;
    return applyColorScheme( lang, createLexer( lang->name, parent ) );
}

QsciLexer* LexerFactory::createLexer(QString lg, QObject* parent)
{
    if ( lg == "bash" )
        return new QsciLexerBash(parent);
    if ( lg == "batch" )
        return new QsciLexerBatch(parent);
    if ( lg == "c" ) // WHAT ABOUT THE "C" LEXER?
        return new QsciLexerCPP(parent);
    if ( lg == "cmake" )
        return new QsciLexerCMake(parent);
    if ( lg == "cpp" )
        return new QsciLexerCPP(parent);
    if ( lg == "cs" )
        return new QsciLexerCSharp(parent);
    if ( lg == "css" )
        return new QsciLexerCSS(parent);
    if ( lg == "d" )
        return new QsciLexerD(parent);
    if ( lg == "diff" )
        return new QsciLexerDiff(parent);
    if ( lg == "fortran" )
        return new QsciLexerFortran(parent);
    if ( lg == "fortran77" )
        return new QsciLexerFortran77(parent);
    if ( lg == "html" )
        return new QsciLexerHTML(parent);
    if ( lg == "idl" )
        return new QsciLexerIDL(parent);
    if ( lg == "java" )
        return new QsciLexerJava(parent);
    if ( lg == "javascript" )
        return new QsciLexerJavaScript(parent);
    if ( lg == "lua" )
        return new QsciLexerLua(parent);
    if ( lg == "makefile" )
        return new QsciLexerMakefile(parent);
    if ( lg == "pascal" )
        return new QsciLexerPascal(parent);
    if ( lg == "perl" )
        return new QsciLexerPerl(parent);
    if ( lg == "postscript" )
        return new QsciLexerPostScript(parent);
    if ( lg == "pov" )
        return new QsciLexerPOV(parent);
    if ( lg == "properties" )
        return new QsciLexerProperties(parent);
    if ( lg == "python" )
        return new QsciLexerPython(parent);
    if ( lg == "ruby" )
        return new QsciLexerRuby(parent);
    if ( lg == "spice" )
        return new QsciLexerSpice(parent);
    if ( lg == "sql" )
        return new QsciLexerSQL(parent);
    if ( lg == "tcl" )
        return new QsciLexerTCL(parent);
    if ( lg == "tex" )
        return new QsciLexerTeX(parent);
    if ( lg == "verilog" )
        return new QsciLexerVerilog(parent);
    if ( lg == "vhdl" )
        return new QsciLexerVHDL(parent);
    if ( lg == "xml" )
        return new QsciLexerXML(parent);
    if ( lg == "yaml" )
        return new QsciLexerYAML(parent);
    return NULL;
}
