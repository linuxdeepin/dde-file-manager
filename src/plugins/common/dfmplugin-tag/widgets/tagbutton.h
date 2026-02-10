// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAGBUTTON_H
#define TAGBUTTON_H

#include "dfmplugin_tag_global.h"

#include <QFrame>

namespace dfmplugin_tag {

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
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    virtual void enterEvent(QEvent *event) override;
#else
    virtual void enterEvent(QEnterEvent *event) override;
#endif
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

}

#endif   // TAGBUTTON_H
