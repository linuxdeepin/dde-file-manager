/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef DFMDISPLAYDEFAULTVIEW_H
#define DFMDISPLAYDEFAULTVIEW_H

#include "displayviewlogic.h"
#include "dfm_filemanager_service_global.h"

#include <QLabel>

DSB_FM_BEGIN_NAMESPACE

class DisplayDefaultViewPrivate;
class DisplayDefaultView : public QLabel, public DisplayViewLogic
{
    Q_OBJECT
    Q_DISABLE_COPY(DisplayDefaultView)
    friend class DisplayDefaultViewPrivate;
    DisplayDefaultViewPrivate *const d;
public:
    explicit DisplayDefaultView(QWidget* parent = nullptr);
    virtual void setRootUrl(const QUrl &url) override;
    virtual QUrl rootUrl() override;
    virtual bool checkViewUrl(const QUrl& url, QString *errorString = nullptr) override;
    virtual void showBeginLogic() override;
    virtual void showEndLogic() override;
};

DSB_FM_END_NAMESPACE

#endif // DFMDISPLAYDEFAULTVIEW_H
