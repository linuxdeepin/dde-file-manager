#ifndef COMMANDLINEMANAGER_H
#define COMMANDLINEMANAGER_H

#include <QCommandLineOption>
#include <QJsonObject>
#include <QMap>
#include <QList>
class QCommandLineParser;
class QCoreApplication;

class CommandLineManager
{

public:

    static CommandLineManager* instance(){
        static CommandLineManager instance;
        return &instance;
    }

    void initOptions();

    void parserOptions();
    bool isNameExisted(const QString& name) const;
    bool isSet(const QString& name) const;
    QString value(const QString& name) const;
    QString description(const QString& name) const;
    void process();

    void addOption(const QCommandLineOption& option);
    void addOptions(const QList<QCommandLineOption> & options);
    QStringList positionalArguments() const;
    QStringList unknownOptionNames() const;

private:
    explicit CommandLineManager();
   ~CommandLineManager();
   CommandLineManager(const CommandLineManager &);
   CommandLineManager & operator = (const CommandLineManager &);
    QCommandLineParser* m_commandParser;
    QList<QCommandLineOption> m_options;
    QMap<QString, QJsonObject> m_objs;
};

#endif // COMMANDLINEMANAGER_H
