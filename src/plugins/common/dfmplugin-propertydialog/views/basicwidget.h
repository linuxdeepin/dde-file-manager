// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASICWIDGET_H
#define BASICWIDGET_H

#include "dfmplugin_propertydialog_global.h"

#include <dfm-base/widgets/dfmkeyvaluelabel/keyvaluelabel.h>
#include <dfm-base/utils/filescanner.h>

#include <dfm-io/dfileinfo.h>

#include <DArrowLineDrawer>
#include <DCheckBox>

namespace dfmplugin_propertydialog {
class MediaInfoFetchWorker;
class BasicWidget : public DTK_WIDGET_NAMESPACE::DArrowLineDrawer
{
    Q_OBJECT
public:
    explicit BasicWidget(QWidget *parent = nullptr);
    virtual ~BasicWidget() override;
    int expansionPreditHeight();

private:
    void initUI();

    void initFileMap();

    void basicExpand(const QUrl &url);

    void basicFieldFilter(const QUrl &url);

    void basicFill(const QUrl &url);

    DFMBASE_NAMESPACE::KeyValueLabel *createValueLabel(QFrame *frame, QString leftValue);

public:
    void selectFileUrl(const QUrl &url);
    qint64 getFileSize();
    int getFileCount();
    void updateFileUrl(const QUrl &url);

public slots:

    void slotFileCountAndSizeChange(const DFMBASE_NAMESPACE::FileScanner::ScanResult &result);

    void slotFileHide(Qt::CheckState state);

    void slotOpenFileLocation();

    void imageExtenInfo(const QUrl &url, QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> properties);
    void videoExtenInfo(const QUrl &url, QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> properties);
    void audioExtenInfo(const QUrl &url, QMap<DFMIO::DFileInfo::AttributeExtendID, QVariant> properties);

protected:
    virtual void closeEvent(QCloseEvent *event) override;

private:
    DFMBASE_NAMESPACE::KeyValueLabel *fileSize { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *fileCount { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *fileType { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *fileLocation { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *fileCreated { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *fileModified { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *fileAccessed { nullptr };
    QCheckBox *hideFile { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *fileMediaResolution { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *fileMediaDuration { nullptr };
    bool hideCheckBox { false };
    DFMBASE_NAMESPACE::FileScanner *fileCalculationUtils { nullptr };
    qint64 fSize { 0 };
    int fCount { 0 };
    QMultiMap<BasicFieldExpandEnum, DFMBASE_NAMESPACE::KeyValueLabel *> fieldMap;
    QFrame *frameMain { nullptr };
    QGridLayout *layoutMain { nullptr };
    QUrl currentUrl;

    QThread fetchThread;
    MediaInfoFetchWorker *infoFetchWorker { nullptr };
};
}
#endif   // BASICWIDGET_H
