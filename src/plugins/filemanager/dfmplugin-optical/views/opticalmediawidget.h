// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPTICALMEDIAWIDGET_H
#define OPTICALMEDIAWIDGET_H

#include "dfmplugin_optical_global.h"

#include <dfm-base/widgets/filemanagerwindow.h>
#include <dfm-base/utils/filestatisticsjob.h>

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QtSvgWidgets/QSvgWidget>
#include <DPushButton>

namespace dfmplugin_optical {

class OpticalMediaWidget : public QWidget
{
    Q_OBJECT

public:
    explicit OpticalMediaWidget(QWidget *parent = nullptr);
    bool updateDiscInfo(const QUrl &url, bool retry = false);

private:
    void initializeUi();
    void initConnect();
    void updateUi();
    void handleErrorMount();
    bool isSupportedUDF();

private slots:
    void onBurnButtonClicked();
    void onDumpButtonClicked();
    void onStagingFileStatisticsFinished();
    void onDiscUnmounted(const QUrl &url);

private:
    QHBoxLayout *layout { nullptr };
    QLabel *lbMediatype { nullptr };
    QLabel *lbAvailable { nullptr };
    QLabel *lbUDFSupport { nullptr };
    DTK_WIDGET_NAMESPACE::DPushButton *pbBurn { nullptr };
    DTK_WIDGET_NAMESPACE::DPushButton *pbDump { nullptr };
    QSvgWidget *iconCaution { nullptr };
    bool disableNotify { false };

    QUrl curUrl;
    bool isBlank { false };
    QString devId;
    QString curFS;
    QString curFSVersion;
    QString curDev;
    QString curMnt;
    QString curDiscName;
    qint64 curAvial;
    int curMediaType;
    QString curMediaTypeStr;

    DFMBASE_NAMESPACE::FileStatisticsJob *statisticWorker { nullptr };
};

}

#endif   // OPTICALMEDIAWIDGET_H
