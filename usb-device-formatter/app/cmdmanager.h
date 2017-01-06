#ifndef CMDMANAGER_H
#define CMDMANAGER_H

#include <QObject>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QApplication>

class CMDManager : public QObject
{
    Q_OBJECT
public:
    static CMDManager* instance();
    void process(const QApplication &app);
    void init();
    bool isSet(const QString& name);
    const QString getPath();

signals:

public slots:

private:
    explicit CMDManager(QObject *parent = 0);
    QCommandLineParser parser;
};

#endif // CMDMANAGER_H
