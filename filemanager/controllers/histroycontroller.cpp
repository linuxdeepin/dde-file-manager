#include "histroycontroller.h"
#include "utils/historyrecord.h"
#include "utils/qobjecthelper.h"

HistroyController::HistroyController(QObject *parent) : QObject(parent)
{
    initData();
}

HistroyController::~HistroyController()
{

}

void HistroyController::initData()
{
    m_histroyUrlRecords = new HistoryRecord(this);
    m_histroySearchKeywordRecords = new HistoryRecord(this);

    HistoryRecord historyRecord, historyRecord2;
    historyRecord.push("/home");
    historyRecord.push("/home/djf");
    historyRecord.push("/home/djf/dde-workspace");
    historyRecord.push("/home/djf/dde-workspace/dde-desktop");
    historyRecord.push("/home/djf/dde-workspace/dde-desktop/skin");

    qDebug() << historyRecord.first();
    qDebug() << historyRecord.end();
    qDebug() << historyRecord.takeAt(2);

    historyRecord.back();
    historyRecord.back();
    qDebug() << historyRecord.currentUrl() << historyRecord.currentIndex();

    qDebug() << historyRecord << historyRecord2;
    QString  result = QObjectHelper::qobject2json(&historyRecord);
    QObjectHelper::json2qobject(result, &historyRecord2);


    qDebug() << result;
    qDebug() << historyRecord2;
}

