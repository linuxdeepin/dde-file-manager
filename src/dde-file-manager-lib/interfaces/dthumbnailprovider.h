// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFM_DFILETHUMBNAILPROVIDER_H
#define DFM_DFILETHUMBNAILPROVIDER_H

#include <QThread>
#include <QFileInfo>

#include "dfmglobal.h"

#include <functional>

QT_BEGIN_NAMESPACE
class QMimeType;
class QLibrary;
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

    typedef std::function<void(const QString &)> CallBack;
    QString createThumbnail(const QFileInfo &info, Size size);
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
    ~DThumbnailProvider() override;

    void run() override;

private:
    QScopedPointer<DThumbnailProviderPrivate> d_ptr;
    QLibrary *m_libMovieViewer = nullptr;
    Q_DECLARE_PRIVATE(DThumbnailProvider)
};

DFM_END_NAMESPACE

#endif // DFM_DFILETHUMBNAILPROVIDER_H
