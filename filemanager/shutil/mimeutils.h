#ifndef MIMEUTILS_H
#define MIMEUTILS_H

#include <QObject>

class MimeUtils : public QObject
{
    Q_OBJECT
public:
    explicit MimeUtils(QObject *parent = 0);
    ~MimeUtils();

signals:

public slots:
};

#endif // MIMEUTILS_H
