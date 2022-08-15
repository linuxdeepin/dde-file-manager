#include "committhread.h"

#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
CommitLog::CommitLog(QObject *parent) : QObject(parent)
{

}

CommitLog::~CommitLog()
{
    qInfo() << " - destroyed";
}

void CommitLog::commit(const QVariant &args)
{
    if(args.isNull() || !args.isValid())
        return;
    const QJsonObject& dataObj = QJsonObject::fromVariantHash(args.toHash());
    QJsonDocument doc(dataObj);
    const QByteArray& sendData = doc.toJson(QJsonDocument::Compact);
    qInfo() << "sendData = "<< sendData;
    //TODO: call external function: void WriteEventLog(const std::string &eventdata);
}

void CommitLog::init()
{
    //TODO: call external function: bool Initialize(const std::string &packagename,bool enable_sig = true);
    m_isInitialized = true;
}


