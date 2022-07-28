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
#ifndef FILEBASEINFOVIEW_H
#define FILEBASEINFOVIEW_H

#include "dfmplugin_detailspace_global.h"

#include "dfm-base/widgets/dfmkeyvaluelabel/keyvaluelabel.h"
#include "dfm-base/interfaces/abstractfileinfo.h"

#include <QUrl>
#include <QFrame>

namespace dfmplugin_detailspace {
class FileBaseInfoView : public QFrame
{
    Q_OBJECT
public:
    explicit FileBaseInfoView(QWidget *parent);
    virtual ~FileBaseInfoView();

private:
    void initUI();

    void initFileMap();

    void basicExpand(const QUrl &url);

    void basicFieldFilter(const QUrl &url);

    void basicFill(const QUrl &url);

    void clearField();

    void connectEvent();

    void connectInit();

    void imageExtenInfoReceiver(const QStringList &properties);

    void videoExtenInfoReceiver(const QStringList &properties);

    void audioExtenInfoReceiver(const QStringList &properties);

signals:
    void sigImageExtenInfo(const QStringList &properties);

    void sigVideoExtenInfo(const QStringList &properties);

    void sigAudioExtenInfo(const QStringList &properties);

public slots:
    void slotImageExtenInfo(const QStringList &properties);

    void slotVideoExtenInfo(const QStringList &properties);

    void slotAudioExtenInfo(const QStringList &properties);

public:
    static void imageExtenInfo(bool flg, QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> properties);

    static void videoExtenInfo(bool flg, QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> properties);

    static void audioExtenInfo(bool flg, QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> properties);

public:
    void setFileUrl(const QUrl &url);

private:
    DFMBASE_NAMESPACE::KeyValueLabel *fileName { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *fileSize { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *fileViewSize { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *fileDuration { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *fileType { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *fileInterviewTime { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *fileChangeTime { nullptr };

    QMultiMap<BasicFieldExpandEnum, DFMBASE_NAMESPACE::KeyValueLabel *> fieldMap;

    QUrl currentUrl;
};

}
#endif   // FILEBASEINFOVIEW_H
