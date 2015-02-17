#include "include/Search/replaceinfilesworker.h"
#include "include/Search/searchinfilesworker.h"
#include <QThread>

ReplaceInFilesWorker::ReplaceInFilesWorker(FileSearchResult::SearchResult searchResult, QString replacement)
    : m_searchResult(searchResult),
      m_replacement(replacement)
{

}

ReplaceInFilesWorker::~ReplaceInFilesWorker()
{

}

void ReplaceInFilesWorker::run()
{
    emit progress("test");
    QThread::sleep(2);
    emit finished(false);
}

void ReplaceInFilesWorker::stop()
{
    m_stopMutex.lock();
    m_stop = true;
    m_stopMutex.unlock();
}
