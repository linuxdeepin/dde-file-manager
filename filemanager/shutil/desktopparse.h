#ifndef DESKTOPPARSE_H
#define DESKTOPPARSE_H

#include <QObject>

class DesktopParse : public QObject
{
    Q_OBJECT
public:
    explicit DesktopParse(QObject *parent = 0);
    ~DesktopParse();

signals:

public slots:
};

#endif // DESKTOPPARSE_H
