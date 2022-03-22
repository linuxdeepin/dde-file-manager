/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
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
#ifndef TAGCOLORLISTWIDGET_H
#define TAGCOLORLISTWIDGET_H

#include "dfmplugin_tag_global.h"

#include <QFrame>

class QVBoxLayout;
class QHBoxLayout;
class QLabel;

DPTAG_BEGIN_NAMESPACE

class TagButton;
class TagColorListWidget final : public QFrame
{
    Q_OBJECT

    Q_PROPERTY(QList<QColor> checkedColorList READ checkedColorList WRITE setCheckedColorList)
    Q_PROPERTY(bool exclusive READ exclusive WRITE setExclusive)
public:
    explicit TagColorListWidget(QWidget *parent = nullptr);

    QList<QColor> checkedColorList() const;
    void setCheckedColorList(const QList<QColor> &colorNames);

    bool exclusive() const;
    void setExclusive(bool exclusive);

    void setToolTipVisible(bool visible);
    void setToolTipText(const QString &text);
    void clearToolTipText();

signals:
    void hoverColorChanged(const QColor &color);
    void checkedColorChanged(const QColor &color);

private:
    void setCentralLayout() noexcept;
    void initUiElement();
    void initConnect();

    QList<TagButton *> tagButtons;
    QVBoxLayout *mainLayout { nullptr };
    QHBoxLayout *buttonLayout { nullptr };
    QLabel *toolTip { nullptr };

    bool currentExclusive = false;
    QStringList currentCheckedColorList;
};

DPTAG_END_NAMESPACE

#endif   // TAGCOLORLISTWIDGET_H
