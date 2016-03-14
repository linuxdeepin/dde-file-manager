#ifndef ICONPROVIDER_H
#define ICONPROVIDER_H

#include <QObject>

class IconProvider : public QObject
{
    Q_OBJECT
public:
    explicit IconProvider(QObject *parent = 0);
    ~IconProvider();

signals:

public slots:
};

#endif // ICONPROVIDER_H
