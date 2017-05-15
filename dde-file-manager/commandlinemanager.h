#ifndef COMMANDLINEMANAGER_H
#define COMMANDLINEMANAGER_H

#include <QCommandLineOption>
#include <QJsonObject>
#include <QMap>
#include <QList>

QT_BEGIN_NAMESPACE
class QCommandLineParser;
class QCoreApplication;
QT_END_NAMESPACE

class DUrl;

class CommandLineManager
{
public:
    static CommandLineManager* instance();

    bool isSet(const QString& name) const;
    QString value(const QString& name) const;

    void process();
    void process(const QStringList &arguments);

    void addOption(const QCommandLineOption& option);
    void addOptions(const QList<QCommandLineOption> & options);
    QStringList positionalArguments() const;
    QStringList unknownOptionNames() const;

    void processCommand();

private:
    explicit CommandLineManager();
    void initOptions();

    ~CommandLineManager();
    QCommandLineParser* m_commandParser;
};

#endif // COMMANDLINEMANAGER_H
