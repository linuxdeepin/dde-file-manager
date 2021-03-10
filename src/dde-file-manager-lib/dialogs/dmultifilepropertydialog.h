/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
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

#ifndef DMULTIFILEPROPERTYDIALOG_H
#define DMULTIFILEPROPERTYDIALOG_H

#include "durl.h"
#include "ddialog.h"
#include "dfmglobal.h"

#include <tuple>
#include <memory>

#include <QList>
#include <QFrame>
#include <QPainter>
#include <QSharedPointer>
#include <QGraphicsItem>

class QWidget;
class QLabel;
class QVBoxLayout;
class QGridLayout;

class DMultiFilePropertyDialogPrivate;

DFM_BEGIN_NAMESPACE
class DFileStatisticsJob;
DFM_END_NAMESPACE

using namespace  Dtk::Widget;

class DMultiFilePropertyDialog : public DDialog
{
    Q_OBJECT

public:

    explicit DMultiFilePropertyDialog(const QList<DUrl>& urlList, QWidget* const parent = nullptr);
    virtual ~DMultiFilePropertyDialog();


    DMultiFilePropertyDialog(const DMultiFilePropertyDialog&)=delete;
    DMultiFilePropertyDialog& operator=(const DMultiFilePropertyDialog&)=delete;

    void startComputingFolderSize();

    static std::size_t getRandomNumber(const std::pair<std::size_t, std::size_t>& scope);

private slots:
   void updateFolderSizeLabel(const qint64 foldersSize)noexcept;

private:

   void initConnect();

   static std::pair<std::size_t, std::size_t> getTheQuantityOfFilesAndFolders(const QList<DUrl>& urlList)noexcept;

    QSharedPointer<DMultiFilePropertyDialogPrivate> d_ptr{ nullptr };
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), DMultiFilePropertyDialog)

};


#endif // DMULTIFILEPROPERTYDIALOG_H
