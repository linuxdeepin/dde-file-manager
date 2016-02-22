#include "histroycontroller.h"
#include "utils/historyrecord.h"
#include "utils/qobjecthelper.h"
#include "../app/global.h"

HistroyController::HistroyController(QObject *parent) : QObject(parent)
{
    initData();
    initConnect();
}

HistroyController::~HistroyController()
{

}

void HistroyController::initData()
{
    m_histroyUrlRecords = new HistoryRecord(this);
    m_histroySearchKeywordRecords = new HistoryRecord(this);
}

void HistroyController::initConnect()
{
    connect(fileSignalManager, &FileSignalManager::currentUrlChanged,
            this, &HistroyController::appendHistroyUrlRecords);
}

void HistroyController::appendHistroyUrlRecords(const QUrl &url)
{

    m_histroyUrlRecords->append(url.toLocalFile());
    qDebug() << this << url << *m_histroyUrlRecords;
}

