// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TAGCOLORLISTWIDGET_H
#define TAGCOLORLISTWIDGET_H

#include "dfmplugin_tag_global.h"

#include <QFrame>
#include <DLabel>

class QVBoxLayout;
class QHBoxLayout;

DWIDGET_USE_NAMESPACE
namespace dfmplugin_tag {

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

private slots:
    void initUiForSizeMode();

private:
    void setCentralLayout() noexcept;
    void initUiElement();
    void initConnect();

    QList<TagButton *> tagButtons;
    QVBoxLayout *mainLayout { nullptr };
    QHBoxLayout *buttonLayout { nullptr };
    DLabel *toolTip { nullptr };

    bool currentExclusive = false;
    QStringList currentCheckedColorList;
};

}

#endif   // TAGCOLORLISTWIDGET_H
