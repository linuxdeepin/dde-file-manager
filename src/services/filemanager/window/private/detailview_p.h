/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#ifndef DETAILVIEW_P_H
#define DETAILVIEW_P_H

#include "dfm_filemanager_service_global.h"
#include "dfm-base/widgets/dfmsplitter/splitter.h"
#include "dfm-base/widgets/dfmkeyvaluelabel/keyvaluelabel.h"

#include <QUrl>
#include <QFrame>

class QLabel;
class QScrollArea;
DSB_FM_BEGIN_NAMESPACE
DFMBASE_USE_NAMESPACE
class DetailView;
class FileBaseInfoView : public QFrame
{
    Q_OBJECT
public:
    explicit FileBaseInfoView(QWidget *parent);
    virtual ~FileBaseInfoView();

    void initUI();

    void setFileUrl(QUrl &url);

private:
    KeyValueLabel *fileName = nullptr;
    KeyValueLabel *fileSize = nullptr;
    KeyValueLabel *fileViewSize = nullptr;
    KeyValueLabel *fileDuration = nullptr;
    KeyValueLabel *fileType = nullptr;
    KeyValueLabel *fileInterviewTime = nullptr;
    KeyValueLabel *fileChangeTime = nullptr;
};

class DetailViewPrivate : public QObject
{
    Q_OBJECT
    friend class DetailView;

public:
    virtual ~DetailViewPrivate();

private:
    explicit DetailViewPrivate(DetailView *view);

    void setFileUrl(QUrl &url);

    void initUI();

    void addCustomControl(QWidget *widget);

    void insertCustomControl(int index, QWidget *widget);

private:
    QUrl url;
    QVBoxLayout *splitter = nullptr;
    QLabel *iconLabel = nullptr;
    FileBaseInfoView *baseInfoView = nullptr;
    DetailView *detailView = nullptr;
    QScrollArea *scrollArea = nullptr;
};
DSB_FM_END_NAMESPACE
#endif   // DETAILVIEW_P_H
