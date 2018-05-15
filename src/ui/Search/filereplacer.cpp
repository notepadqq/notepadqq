#include "include/Search/filereplacer.h"

#include "include/docengine.h"

#include <QFile>
#include <QTextStream>

FileReplacer::FileReplacer(const SearchResult& results, const QString &replacement)
    : m_searchResult(results),
      m_replacement(replacement)
{ }

void FileReplacer::replaceAll(const DocResult& doc, QString& content, const QString& replacement)
{
    struct BackReference
    {
        int pos;
        int num;
    };

    if (doc.results.isEmpty())
        return;

    QVector<BackReference> backReferences;

    // Search the replacement string for occurences of back references.
    // Leave any references to non-existing capture groups alone.
    // Only supports numbered references of up to 9. No named or relative references.
    if (doc.regexCaptureGroupCount > 0) {
        const int alen = replacement.length();
        int idx = 0;
        while ((idx = replacement.indexOf('\\', idx)) != -1){
            if (idx==alen-1) break;

            const auto val = replacement.at(idx+1).digitValue();

            if (val > 0 && val <= doc.regexCaptureGroupCount) {
                BackReference ref;
                ref.pos = idx;
                ref.num = val;
                backReferences.append(ref);
            }

            idx += 2;
        }
    }

    // Similar to QString::replace(...)
    // Iterate on the matches. For every match, copy in chunks
    // - the part before the match
    // - the replacement string, with the proper replacements for the backreferences

    int newLength = 0; // length of the new string, with all the replacements
    int lastEnd = 0;
    QVector<QStringRef> chunks;
    const QString copy = content;

    for (const auto& result : doc.results) {

        int len = result.positionInFile - lastEnd;
        if (len > 0) {
            chunks << copy.midRef(lastEnd, len);
            newLength += len;
        }

        lastEnd = 0;

        // add the after string, with replacements for the backreferences
        for (const BackReference& backReference : backReferences) {
            // part of "after" before the backreference
            len = backReference.pos - lastEnd;
            if (len > 0) {
                chunks << replacement.midRef(lastEnd, len);
                newLength += len;
            }

            // backreference itself
            len = result.regexMatch.capturedLength(backReference.num);
            if (len > 0) {
                chunks << copy.midRef(result.regexMatch.capturedStart(backReference.num),
                                      len);
                newLength += len;
            }

            lastEnd = backReference.pos + 2; // Back reference is length 2 (e.g. "\\1")
        }

        // add the last part of the after string
        len = replacement.length() - lastEnd;
        if (len > 0) {
            chunks << replacement.midRef(lastEnd, len);
            newLength += len;
        }

        lastEnd = result.positionInFile + result.matchLength;
    }

    // 3. trailing string after the last match
    if (copy.length() > lastEnd) {
        chunks << copy.midRef(lastEnd);
        newLength += copy.length() - lastEnd;
    }

    // 4. assemble the chunks together
    content.resize(newLength);
    int i = 0;
    QChar *uc = content.data();
    for (const QStringRef &chunk : chunks) {
        int len = chunk.length();
        memcpy(uc + i, chunk.unicode(), static_cast<ulong>(len) * sizeof(QChar));
        i += len;
    }
}

void FileReplacer::run()
{
    int count = 0;

    for (const DocResult& docResult : m_searchResult.results) {
        if(m_wantToStop)
            return;

        if (++count % 10 == 0)
            emit resultProgress(count, m_searchResult.results.size());

        if (docResult.results.isEmpty())
            continue;

        QFile f(docResult.fileName);
        DocEngine::DecodedText decodedText;

        decodedText = DocEngine::readToString(&f);
        if (decodedText.error) {
            m_failedFiles.push_back(docResult.fileName);
            continue;
        }

        replaceAll(docResult, decodedText.text, m_replacement);

        if (!DocEngine::writeFromString(&f, decodedText)) {
            m_failedFiles.push_back(docResult.fileName);
            continue;
        }
    }

    emit resultReady();
}
