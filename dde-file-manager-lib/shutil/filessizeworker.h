#ifndef FILESSIZEWORKER_H
#define FILESSIZEWORKER_H

#include <QObject>
#include "durl.h"

class FilesSizeWorker : public QObject
{
    Q_OBJECT
public:
    explicit FilesSizeWorker(const DUrlList& urls={}, QObject *parent = 0);
    ~FilesSizeWorker();

    DUrlList urls() const;
    void setUrls(const DUrlList &urls);

    bool stopped() const;
    void setStopped(bool stopped);

    qint64 size() const;
    void setSize(const qint64 &size);

signals:
    void sizeUpdated(qint64 size);

public slots:
    void coumpueteSize();
    void updateSize();
    void stop();

private:
    DUrlList m_urls = {};
    qint64 m_size = 0;
    bool m_stopped = false;

};

#endif // FILESSIZEWORKER_H
