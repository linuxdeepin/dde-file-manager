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
#ifndef MULTIFILEPROPERTYDIALOG_H
#define MULTIFILEPROPERTYDIALOG_H

#include "dfmplugin_propertydialog_global.h"
#include "dfm-base/widgets/dfmkeyvaluelabel/keyvaluelabel.h"
#include "dfm-base/utils/filestatisticsjob.h"

#include <DDialog>

#include <QUrl>
#include <QList>
#include <QFrame>
#include <QPainter>
#include <QSharedPointer>
#include <QGraphicsItem>

#include <tuple>
#include <memory>

class QWidget;
class QLabel;
class QVBoxLayout;
class QGridLayout;

DPPROPERTYDIALOG_BEGIN_NAMESPACE
class MultiFilePropertyDialog : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT

public:
    explicit MultiFilePropertyDialog(const QList<QUrl> &urls, QWidget *const parent = nullptr);
    virtual ~MultiFilePropertyDialog() override;

private:
    void initHeadUi();

    void initInfoUi();

    void calculateFileCount();

private slots:
    void updateFolderSizeLabel(qint64 size);

private:
    QList<QUrl> urlList {};   //###: this list contains all the urls which are selected!
    QLabel *iconLabel { nullptr };
    QLabel *multiFileLable { nullptr };
    QLabel *basicInfoLabel { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *totalSizeLabel { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *fileCountLabel { nullptr };
    DFMBASE_NAMESPACE::FileStatisticsJob *fileCalculationUtils { nullptr };
};
DPPROPERTYDIALOG_END_NAMESPACE
#endif   // MULTIFILEPROPERTYDIALOG_H
