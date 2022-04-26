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
#ifndef BASICWIDGET_H
#define BASICWIDGET_H

#include "dfmplugin_propertydialog_global.h"
#include "services/common/propertydialog/property_defines.h"

#include "dfm-base/widgets/dfmkeyvaluelabel/keyvaluelabel.h"
#include "dfm-base/utils/filestatisticsjob.h"

#include <DArrowLineDrawer>

#include <QCheckBox>

DPPROPERTYDIALOG_BEGIN_NAMESPACE
class BasicWidget : public DTK_WIDGET_NAMESPACE::DArrowLineDrawer
{
    Q_OBJECT
public:
    explicit BasicWidget(QWidget *parent = nullptr);
    virtual ~BasicWidget() override;

private:
    void initUI();

    void initFileMap();

    void basicExpand(const QUrl &url);

    void basicFieldFilter(const QUrl &url);

    void basicFill(const QUrl &url);

public:
    void selectFileUrl(const QUrl &url);

    qint64 getFileSize();

    int getFileCount();

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
    DFMBASE_NAMESPACE::FileStatisticsJob *fileCalculationUtils { nullptr };
    qint64 fSize { 0 };
    int fCount { 0 };
    QMultiMap<DSC_NAMESPACE::CPY_NAMESPACE::BasicFieldExpandEnum, DFMBASE_NAMESPACE::KeyValueLabel *> fieldMap;
    QFrame *frameMain { nullptr };
    QUrl currentUrl;
};
DPPROPERTYDIALOG_END_NAMESPACE
#endif   // BASICWIDGET_H
