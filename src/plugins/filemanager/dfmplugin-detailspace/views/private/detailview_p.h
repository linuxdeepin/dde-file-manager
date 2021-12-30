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

#include "dfmplugin_detailspace_global.h"

#include "services/filemanager/dfm_filemanager_service_global.h"
#include "dfm-base/widgets/dfmsplitter/splitter.h"
#include "dfm-base/widgets/dfmkeyvaluelabel/keyvaluelabel.h"

#include <QUrl>
#include <QFrame>

class QLabel;
class QScrollArea;

DPDETAILSPACE_BEGIN_NAMESPACE
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
    DFMBASE_NAMESPACE::KeyValueLabel *fileName { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *fileSize { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *fileViewSize { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *fileDuration { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *fileType { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *fileInterviewTime { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *fileChangeTime { nullptr };
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
    QVBoxLayout *splitter { nullptr };
    QLabel *iconLabel { nullptr };
    FileBaseInfoView *baseInfoView { nullptr };
    DetailView *detailView { nullptr };
    QScrollArea *scrollArea { nullptr };
};
DPDETAILSPACE_END_NAMESPACE

#endif   // DETAILVIEW_P_H
