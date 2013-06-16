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
#include <Qsci/qscilexerproperties.h> /**/
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

LexerFactory::LexerFactory()
{
    langDefFile = generalFunctions::getUserFilePath("langs.xml");
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

bool LexerFactory::init()
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

        LangDefinition lang;
        lang.name            = node.attributes().namedItem("name").nodeValue();
        lang.file_extensions = read_attribute(node, "ext", "").split(" ", QString::SkipEmptyParts);
        lang.comment_line    = read_attribute(node, "commentLine" , NULL);
        lang.comment_start   = read_attribute(node, "commentStart", NULL);
        lang.comment_end     = read_attribute(node, "commentEnd"  , NULL);
        QDomNode keywords_node = node.firstChildElement("Keywords");
        while(!keywords_node.isNull()) {
            KeywordCollection kw_class;
            kw_class.type     = read_attribute   (keywords_node, "name", "");
            kw_class.keywords = read_element_text(keywords_node, "").split(" ", QString::SkipEmptyParts);
            keywords_node     = keywords_node.nextSiblingElement("Keywords");

            lang.keywords.append(kw_class);
            lang.keywords_by_class.insert(kw_class.type, kw_class);
        }
        node = node.nextSiblingElement("Language");
        languages.append        (lang);
        languages_by_name.insert(lang.name, lang);

        foreach(QString ext, lang.file_extensions) {
            language_by_extension.insert(ext, lang); // IF MORE LANGUAGES USES THE SAME EXTENSIONS ( should not! ) THE LAST WINS
        }
    }
    return true;
}

LangDefinition LexerFactory::detectLanguage(QFileInfo info)
{
    // GET COMPLETE FILE EXTENSIONS
    QString ext = info.completeSuffix();
    return language_by_extension.value(ext); // RETURNS DEFAULT IF NOT FOUND
}

QsciLexer* LexerFactory::createLexer(QFileInfo info, QObject *parent)
{
    return createLexer( detectLanguage(info).name, parent );
}

QsciLexer* LexerFactory::createLexer(QString lg, QObject* parent)
{
    if ( lg == "bash" )
        return new QsciLexerBash(parent);
    if ( lg == "batch" )
        return new QsciLexerBatch(parent);
    if ( lg == "cmake" )
        return new QsciLexerCMake(parent);
    if ( lg == "cpp" )
        return new QsciLexerCPP(parent);
    if ( lg == "csharp" )
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
