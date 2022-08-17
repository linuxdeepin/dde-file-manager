
#include "rlog.h"
#include "rlog/vaultreportdata.h"
#include "rlog/smbreportdata.h"
#include "rlog/searchreportdata.h"
#include "rlog/committhread.h"

#include <QDebug>
#include <QJsonDocument>
#include <QThread>

#include <QApplication>

RLog::RLog(QObject *parent)
    : QObject(parent)
{
}
class RLog_ : public RLog
{
};

Q_GLOBAL_STATIC(RLog_, rlGlobal)

void RLog::commit(const QString &type, const QVariantMap &args)
{
    if (!m_isInit) {
        qInfo() << "Error: RLog is not initialized.";
        return;
    }

    ReportDataInterface *interface = m_logDataObj.value(type, nullptr);
    if (!interface) {
        qInfo() << "Error: Log data object is not registed.";
        return;
    }
    QJsonObject jsonObject = interface->prepareData(args);

    const QStringList &keys = m_commonData.keys();
    foreach (const QString &key, keys) {
        jsonObject.insert(key, m_commonData.value(key));   //add common data for each log commit
    }
    emit appendArgs(jsonObject.toVariantHash());   //Send log data to the thread
}

RLog *RLog::instance()
{
    return rlGlobal;
}

RLog::~RLog()
{
    qInfo() << "RLog start destroy";
    qDeleteAll(m_logDataObj.begin(), m_logDataObj.end());
    m_logDataObj.clear();
    if (m_commitThread) {
        qInfo() << "Log thread start to quit";
        m_commitThread->quit();
        m_commitThread->wait(2000);
        qInfo() << "Log thread quited.";
    }
    qInfo() << "RLog already destroy";
}

void RLog::init()
{
    qInfo() << " - start init RLog";
    if (m_isInit) {
        qInfo() << " - RLog is already initialized, can not do this twice.";
        return;
    }

    m_commonData.insert("dfmVersion", QApplication::applicationVersion());

    ReportDataInterface *vault = new VaultReportData();
    registerLogData(vault->type(), vault);

    ReportDataInterface *smb = new SmbReportData();
    registerLogData(smb->type(), smb);

    ReportDataInterface *search = new SearchReportData();
    registerLogData(search->type(), search);

    m_commitLog = new CommitLog();
    if (!m_commitLog->init())
        return;

    m_commitThread = new QThread();
    connect(this, &RLog::appendArgs, m_commitLog, &CommitLog::commit);
    connect(m_commitThread, &QThread::finished, [&]() {
        m_commitLog->deleteLater();
    });
    m_commitLog->moveToThread(m_commitThread);
    m_commitThread->start();

    m_isInit = true;
    qInfo() << " - end init RLog, commit thread is started.";
}

bool RLog::registerLogData(const QString &type, ReportDataInterface *dataObj)
{
    if (m_logDataObj.contains(type)) {
        qInfo() << "Log type already existed.";
        return false;
    }
    m_logDataObj.insert(type, dataObj);
    return true;
}
