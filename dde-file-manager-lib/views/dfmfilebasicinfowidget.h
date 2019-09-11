/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Mike Chen <kegechen@gmail.com>
 *
 * Maintainer: Mike Chen <chenke_cm@deepin.com>
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
#ifndef DFMFILEBASICINFOWIDGET_H
#define DFMFILEBASICINFOWIDGET_H

#include <QFrame>
#include <QLabel>
#include <dfmglobal.h>
#include "durl.h"

DFM_BEGIN_NAMESPACE


class SectionKeyLabel: public QLabel
{
    Q_OBJECT

public:
    explicit SectionKeyLabel(const QString &text="", QWidget *parent = nullptr, Qt::WindowFlags f = {});
};

class SectionValueLabel: public QLabel
{
    Q_OBJECT

public:
    explicit SectionValueLabel(const QString &text="", QWidget *parent = nullptr, Qt::WindowFlags f = {});
};

class LinkSectionValueLabel: public SectionValueLabel
{
    Q_OBJECT

public:
    explicit LinkSectionValueLabel(const QString &text="", QWidget *parent = nullptr, Qt::WindowFlags f = {});

    DUrl linkTargetUrl() const;
    void setLinkTargetUrl(const DUrl &linkTargetUrl);

protected:
    void mouseReleaseEvent(QMouseEvent* event) Q_DECL_OVERRIDE;

private:
    DUrl m_linkTargetUrl;
};


class DFMFileBasicInfoWidgetPrivate;
class DFMFileBasicInfoWidget : public QFrame
{
    Q_OBJECT
public:
    explicit DFMFileBasicInfoWidget(QWidget *parent = nullptr);
    virtual ~DFMFileBasicInfoWidget();

    void setUrl(const DUrl &url);

    bool showFileName();
    void setShowFileName(bool visible);
    bool showPicturePixel();
    void setShowPicturePixel(bool visible);
    bool showVideoInfo();
    void setShowVideoInfo(bool visible);


private:
    QScopedPointer<DFMFileBasicInfoWidgetPrivate> d_private;
    Q_DECLARE_PRIVATE_D(d_private, DFMFileBasicInfoWidget)
};

DFM_END_NAMESPACE

#endif // DFMFILEBASICINFOWIDGET_H
