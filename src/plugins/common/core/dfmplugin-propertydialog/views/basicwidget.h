// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASICWIDGET_H
#define BASICWIDGET_H

#include "dfmplugin_propertydialog_global.h"

#include <dfm-base/widgets/dfmkeyvaluelabel/keyvaluelabel.h>
#include <dfm-base/utils/filestatisticsjob.h>

#include <DArrowLineDrawer>
#include <DCheckBox>

namespace dfmplugin_propertydialog {
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

    void slotFileCountAndSizeChange(qint64 size, int filesCount, int directoryCount);

    void slotFileHide(int state);

protected:
    virtual void closeEvent(QCloseEvent *event) override;

private:
    DFMBASE_NAMESPACE::KeyValueLabel *fileSize { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *fileCount { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *fileType { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *filePosition { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *fileCreated { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *fileModified { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *fileAccessed { nullptr };
    QCheckBox *hideFile { nullptr };
    bool hideCheckBox { false };
    DFMBASE_NAMESPACE::FileStatisticsJob *fileCalculationUtils { nullptr };
    qint64 fSize { 0 };
    int fCount { 0 };
    QMultiMap<BasicFieldExpandEnum, DFMBASE_NAMESPACE::KeyValueLabel *> fieldMap;
    QFrame *frameMain { nullptr };
    QGridLayout *layoutMain { nullptr };
    QUrl currentUrl;
};
}
#endif   // BASICWIDGET_H
