#include "include/Search/filesearcher.h"
#include "include/Search/searchhelpers.h"
#include "include/docengine.h"

#include <QtTest>

#include <vector>

DocEngine::DecodedText DocEngine::readToString(QFile* file)
{
    DecodedText decoded;
    if (!file->open(QIODevice::ReadOnly)) {
        decoded.error = true;
        return decoded;
    }

    decoded.text = QString::fromUtf8(file->readAll());
    return decoded;
}

class CppCorrectnessTest : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void searchPlainText_handlesContentEndingInLf();
    void linePositions_data();
    void linePositions();
};

void CppCorrectnessTest::searchPlainText_handlesContentEndingInLf()
{
    SearchConfig config;
    config.searchString = "needle";

    const DocResult result = FileSearcher::searchPlainText(config, "needle\n");

    QCOMPARE(result.results.size(), 1);
    QCOMPARE(result.results.constFirst().lineNumber, 1);
}

void CppCorrectnessTest::linePositions_data()
{
    QTest::addColumn<QString>("data");
    QTest::addColumn<std::vector<int>>("expected");

    QTest::newRow("empty") << "" << std::vector<int>({0, 0});
    QTest::newRow("no-line-break") << "abc" << std::vector<int>({0, 3});
    QTest::newRow("terminal-lf") << "abc\n" << std::vector<int>({0, 4, 4});
    QTest::newRow("terminal-cr") << "abc\r" << std::vector<int>({0, 4, 4});
    QTest::newRow("crlf") << "a\r\nb" << std::vector<int>({0, 3, 4});
}

void CppCorrectnessTest::linePositions()
{
    QFETCH(QString, data);
    QFETCH(std::vector<int>, expected);

    QCOMPARE(SearchHelpers::linePositions(data), expected);
}

QTEST_GUILESS_MAIN(CppCorrectnessTest)

#include "tst_cppcorrectness.moc"
