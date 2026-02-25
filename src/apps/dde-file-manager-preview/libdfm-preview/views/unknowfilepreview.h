// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UNKNOWFILEPREVIEW_H
#define UNKNOWFILEPREVIEW_H
#include "dfmplugin_filepreview_global.h"
#include <dfm-base/interfaces/abstractbasepreview.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/utils/filescanner.h>

#include <QLabel>
#include <QUrl>
#include <QPointer>

namespace dfmplugin_filepreview {
class UnknowFilePreview : public DFMBASE_NAMESPACE::AbstractBasePreview
{
    Q_OBJECT
public:
    explicit UnknowFilePreview(QObject *parent = nullptr);
    ~UnknowFilePreview() override;

    bool setFileUrl(const QUrl &url) override;
    QUrl fileUrl() const override;

    QWidget *contentWidget() const override;

private:
    void setFileInfo(const FileInfoPointer &info);

signals:
    void requestStartFolderSize();

public slots:
    void updateFolderSizeCount(const DFMBASE_NAMESPACE::FileScanner::ScanResult &result);

private:
    QUrl url;
    QWidget *contentView { nullptr };
    QLabel *iconLabel { nullptr };
    QLabel *nameLabel { nullptr };
    QLabel *sizeLabel { nullptr };
    QLabel *typeLabel { nullptr };
    DFMBASE_NAMESPACE::FileScanner *fileCalculationUtils { nullptr };
};
}
#endif   // UNKNOWFILEPREVIEW_H
