#ifndef DFM_DFILETHUMBNAILPROVIDER_H
#define DFM_DFILETHUMBNAILPROVIDER_H

#include <QThread>
#include <QFileInfo>

#include "dfmglobal.h"

#include <functional>

QT_BEGIN_NAMESPACE
class QMimeType;
QT_END_NAMESPACE

DFM_BEGIN_NAMESPACE

class DThumbnailProviderPrivate;
class DThumbnailProvider : public QThread
{
    Q_OBJECT

public:
    enum Size {
        Small = 64,
        Normal = 128,
        Large = 256,
    };

    static DThumbnailProvider *instance();

    bool hasThumbnail(const QFileInfo &info) const;
    bool hasThumbnail(const QMimeType &mimeType) const;

    QString thumbnailFilePath(const QFileInfo &info, Size size) const;

    QString createThumbnail(const QFileInfo &info, Size size);
    typedef std::function<void(const QString&)> CallBack;
    void appendToProduceQueue(const QFileInfo &info, Size size, CallBack callback = 0);
    void removeInProduceQueue(const QFileInfo &info, Size size);

    QString errorString() const;

    qint64 defaultSizeLimit() const;
    void setDefaultSizeLimit(qint64 size);

    qint64 sizeLimit(const QMimeType &mimeType) const;
    void setSizeLimit(const QMimeType &mimeType, qint64 size);

signals:
    void thumbnailChanged(const QString &sourceFilePath, const QString &thumbnailPath) const;
    void createThumbnailFinished(const QString &sourceFilePath, const QString &thumbnailPath) const;
    void createThumbnailFailed(const QString &sourceFilePath) const;

protected:
    explicit DThumbnailProvider(QObject *parent = 0);
    ~DThumbnailProvider();

    void run() Q_DECL_OVERRIDE;

private:
    QScopedPointer<DThumbnailProviderPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DThumbnailProvider)
};

DFM_END_NAMESPACE

#endif // DFM_DFILETHUMBNAILPROVIDER_H
