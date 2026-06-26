#include "include/notepadqq.h"
#include "notepadqq.cpp"
#include "nqqsettings.cpp"

#include <QString>
#include <QtTest>

class NotepadqqTest : public QObject {
    Q_OBJECT

public:
    NotepadqqTest();

private Q_SLOTS:
    void editorPathIsHtml();
};

NotepadqqTest::NotepadqqTest()
{
    // QApplication a();
}

void NotepadqqTest::editorPathIsHtml()
{ QVERIFY(Notepadqq::editorPath().endsWith(".html")); }

QTEST_GUILESS_MAIN(NotepadqqTest)

#include "tst_notepadqqtest.moc"
