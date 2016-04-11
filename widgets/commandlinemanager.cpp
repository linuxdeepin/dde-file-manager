#include "commandlinemanager.h"
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QCoreApplication>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

CommandLineManager::CommandLineManager():
    m_commandParser(new QCommandLineParser)
{

}

void CommandLineManager::process(){
    m_commandParser->setApplicationDescription(QString("%1 helper").arg(QCoreApplication::applicationName()));
    m_commandParser->addHelpOption();
    m_commandParser->addVersionOption();
    m_commandParser->process(*qApp);
    parserOptions();
}

void CommandLineManager::initOptions(){
    QCommandLineOption logOption(QStringList() << "l" << "log", "enable log to destination [stdout|file].", "logDestination", "file");
    CommandLineManager::instance()->addOption(logOption);
    CommandLineManager::instance()->process();
}

void CommandLineManager::addOption(const QCommandLineOption &option){
    m_commandParser->addOption(option);
    m_options.append(option);
}

void CommandLineManager::addOptions(const QList<QCommandLineOption> &options){
    foreach (QCommandLineOption option, options) {
        addOption(option);
    }
}

QStringList CommandLineManager::positionalArguments() const
{
    return m_commandParser->positionalArguments();
}

QStringList CommandLineManager::unknownOptionNames() const
{
    return m_commandParser->unknownOptionNames();
}

void CommandLineManager::parserOptions(){
    foreach (QCommandLineOption option, m_options) {
        QJsonObject obj;
        obj.insert("names", QJsonArray::fromStringList(option.names()));
        obj.insert("valueName", option.valueName());
        obj.insert("description", option.description());
        obj.insert("isSet", m_commandParser->isSet(option));
        obj.insert("value", m_commandParser->value(option));

        foreach (QString name, option.names()) {
            m_objs.insert(name, obj);
        }
        m_objs.insert(option.valueName(), obj);
    }
}

bool CommandLineManager::isNameExisted(const QString &name) const{
    if (m_objs.contains(name))
        return true;
    return false;
}

bool CommandLineManager::isSet(const QString &name) const{
    if (isNameExisted(name))
        return m_objs.value(name).value("isSet").toBool();
    return false;
}

QString CommandLineManager::value(const QString &name) const{
    if (isNameExisted(name))
        return m_objs.value(name).value("value").toString();
    return "";
}

QString CommandLineManager::description(const QString &name) const{
    if (isNameExisted(name))
        return m_objs.value(name).value("description").toString();
    return "";
}

CommandLineManager::~CommandLineManager(){

}
