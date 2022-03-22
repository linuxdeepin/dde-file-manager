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
#ifndef TAGBUTTON_H
#define TAGBUTTON_H

#include "dfmplugin_tag_global.h"

#include <QFrame>

DPTAG_BEGIN_NAMESPACE

class TagButton final : public QFrame
{
    Q_OBJECT

    enum class PaintStatus {
        kNormal,
        kHover,
        kPressed,
        kChecked
    };

public:
    explicit TagButton(const QColor &color,
                       QWidget *const parent = nullptr);

    void setRadiusF(const double &radius) noexcept;
    void setRadiusF(const double &radius) const noexcept = delete;
    void setRadius(const std::size_t &radius) noexcept;
    void setRadius(const std::size_t &radius) const noexcept = delete;

    void setBackGroundColor(const QColor &color) noexcept;
    void setBackGroundColor(const QColor &color) const noexcept = delete;

    void setCheckable(bool checkable) noexcept;
    void setChecked(bool checked);
    bool isChecked() const;
    bool isHovered() const;

    QColor color() const;

signals:
    void click(QColor color);
    void enter();
    void leave();
    void checkedChanged();

protected:
    virtual void enterEvent(QEvent *event) override;
    virtual void leaveEvent(QEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;

    virtual void paintEvent(QPaintEvent *paintEvent) override;

private:
    using QFrame::setBaseSize;
    using QFrame::setFixedHeight;
    using QFrame::setFixedSize;
    using QFrame::setFixedWidth;

    void setParameters();
    void setPaintStatus(PaintStatus status);

    bool checkable { true };
    PaintStatus paintStatus { PaintStatus::kNormal };

    QMarginsF margins { 1.0, 1.0, 1.0, 1.0 };
    QPair<double, std::size_t> radius { 0.0, 0 };
    QList<QPair<QColor, QColor>> allStatusColors {};

    QColor tagColor {};
    QColor backgroundColor { "#cecece" };
    QColor outlineColor { "#d8d8d8" };
};

DPTAG_END_NAMESPACE

#endif   // TAGBUTTON_H
