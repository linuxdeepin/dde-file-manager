// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEBASEINFOVIEW_H
#define FILEBASEINFOVIEW_H

#include "dfmplugin_detailspace_global.h"

#include <dfm-base/widgets/dfmkeyvaluelabel/keyvaluelabel.h>
#include <dfm-base/interfaces/fileinfo.h>

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
    void connectInit();
    void imageExtenInfoReceiver(const QStringList &properties);
    void videoExtenInfoReceiver(const QStringList &properties);
    void audioExtenInfoReceiver(const QStringList &properties);

signals:
    void sigImageExtenInfo(const QStringList &properties);
    void sigVideoExtenInfo(const QStringList &properties);
    void sigAudioExtenInfo(const QStringList &properties);

public slots:
    void imageExtenInfo(const QUrl &url, QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> properties);
    void videoExtenInfo(const QUrl &url, QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> properties);
    void audioExtenInfo(const QUrl &url, QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> properties);
    void slotImageExtenInfo(const QStringList &properties);
    void slotVideoExtenInfo(const QStringList &properties);
    void slotAudioExtenInfo(const QStringList &properties);

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
