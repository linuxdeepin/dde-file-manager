#ifndef UNKNOWFILEPREVIEW_H
#define UNKNOWFILEPREVIEW_H
#include "dfmplugin_filepreview_global.h"
#include "dfm-base/interfaces/abstractbasepreview.h"
#include "dfm-base/interfaces/abstractfileinfo.h"
#include "utils/calculationutils.h"

#include <QLabel>
/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <QUrl>
#include <QPointer>

DPFILEPREVIEW_BEGIN_NAMESPACE
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
    void setFileInfo(const AbstractFileInfoPointer &info);

signals:
    void requestStartFolderSize();

public slots:
    void updateFolderSize(qint64 size);

    void updateFileCount(int count);

private:
    QUrl url;
    QWidget *contentView { nullptr };
    QLabel *iconLabel { nullptr };
    QLabel *nameLabel { nullptr };
    QLabel *sizeLabel { nullptr };
    QLabel *typeLabel { nullptr };
    CalculationUtils *fileCalculationUtils { nullptr };
};
DPFILEPREVIEW_END_NAMESPACE
#endif   // UNKNOWFILEPREVIEW_H
