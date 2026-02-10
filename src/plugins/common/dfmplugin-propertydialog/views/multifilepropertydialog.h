// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MULTIFILEPROPERTYDIALOG_H
#define MULTIFILEPROPERTYDIALOG_H

#include "dfmplugin_propertydialog_global.h"
#include <dfm-base/widgets/dfmkeyvaluelabel/keyvaluelabel.h>
#include <dfm-base/utils/filestatisticsjob.h>

#include <DDialog>

#include <QUrl>
#include <QList>
#include <QFrame>
#include <QPainter>
#include <QSharedPointer>
#include <QGraphicsItem>

class QWidget;
class QLabel;
class QVBoxLayout;
class QGridLayout;

namespace dfmplugin_propertydialog {

class FileCountCalculator;

class MultiFilePropertyDialog : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT

public:
    explicit MultiFilePropertyDialog(const QList<QUrl> &urls, QWidget *const parent = nullptr);
    virtual ~MultiFilePropertyDialog() override;

private:
    void initHeadUi();

    void initInfoUi();

private slots:
    void updateFolderSizeLabel(qint64 size, int filesCount, int directoryCount);

private:
    QList<QUrl> urlList {};   // ###: this list contains all the urls which are selected!
    QLabel *iconLabel { nullptr };
    QLabel *multiFileLable { nullptr };
    QLabel *basicInfoLabel { nullptr };
    QLabel *totalSizeLabel { nullptr };
    QLabel *fileCountLabel { nullptr };
    QLabel *totalSizeValueLabel { nullptr };
    QLabel *fileCountValueLabel { nullptr };
    QLabel *accessTimeLabel { nullptr };
    QLabel *accessTimeValueLabel { nullptr };
    QLabel *modifyTimeLable { nullptr };
    QLabel *modifyTimeValueLable { nullptr };
    DFMBASE_NAMESPACE::FileStatisticsJob *fileCalculationUtils { nullptr };
    FileCountCalculator *fileCountCalculator { nullptr };
};
}
#endif   // MULTIFILEPROPERTYDIALOG_H
