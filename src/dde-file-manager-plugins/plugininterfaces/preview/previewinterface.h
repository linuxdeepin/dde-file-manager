/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             lixiang<lixianga@uniontech.com>
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

#ifndef PREVIEWINTERFACE_H
#define PREVIEWINTERFACE_H

#include <QStringList>
#include <QWidget>
#include <QIcon>

#define DEFALT_PREVIEW_MIN_SIZE QSize(600, 300)

class PreviewInterface : public QObject
{
public:
    explicit PreviewInterface(QObject *parent = 0)
        : QObject(parent) {}

    virtual void init(const QString& uri){
        Q_UNUSED(uri)
    }

    virtual QWidget* previewWidget(){
        return NULL;
    }

    virtual QSize previewWidgetMinSize() const {
        return DEFALT_PREVIEW_MIN_SIZE;
    }

    virtual bool canPreview() const{
        return false;
    }

    virtual QWidget* toolBarItem(){
        return NULL;
    }

    virtual QString pluginName() const{
        return "";
    }

    virtual QIcon pluginLogo() const{
        return QIcon();
    }

    virtual QString pluginDescription() const{
        return "";
    }
};

#define PreviewInterface_iid "com.deepin.dde-file-manager.PreviewInterface"

Q_DECLARE_INTERFACE(PreviewInterface, PreviewInterface_iid)

#endif // PREVIEWINTERFACE_H
