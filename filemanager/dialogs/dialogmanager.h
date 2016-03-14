#ifndef DIALOGMANAGER_H
#define DIALOGMANAGER_H

#include <QObject>

class DialogManager : public QObject
{
    Q_OBJECT
public:
    explicit DialogManager(QObject *parent = 0);
    ~DialogManager();

signals:

public slots:
};

#endif // DIALOGMANAGER_H
