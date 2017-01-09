#ifndef COMMANDMANAGER_H
#define COMMANDMANAGER_H

#include <QObject>

class CommandManagerAdaptor;

class CommandManager : public QObject
{
    Q_OBJECT
public:
    static QString ObjectPath;
    explicit CommandManager(QObject *parent = 0);

signals:

public slots:
    bool process(const QString& cmd, const QStringList& args, QString &output, QString &error);
    bool startDetached(const QString &cmd, const QStringList &args);

private:
    CommandManagerAdaptor* m_commandManagerAdaptor = NULL;
};

#endif // COMMANDMANAGER_H
