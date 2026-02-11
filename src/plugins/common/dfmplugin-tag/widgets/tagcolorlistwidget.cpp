// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tagcolorlistwidget.h"
#include "tagbutton.h"
#include "utils/taghelper.h"

#include <DGuiApplicationHelper>
#ifdef DTKWIDGET_CLASS_DSizeMode
#    include <DSizeMode>
#endif

#include <QVBoxLayout>

using namespace dfmplugin_tag;
DWIDGET_USE_NAMESPACE

TagColorListWidget::TagColorListWidget(QWidget *parent, UseType type)
    : QFrame(parent), useType(type)
{
    setObjectName("tagActionWidget");
    setFocusPolicy(Qt::StrongFocus);

    initUiElement();
    initConnect();
}

QList<QColor> TagColorListWidget::checkedColorList() const
{
    QList<QColor> list;

    for (const TagButton *button : tagButtons) {
        if (button->isChecked())
            list << button->color();
    }

    return list;
}

void TagColorListWidget::setCheckedColorList(const QList<QColor> &colorNames)
{
    for (TagButton *button : tagButtons) {
        button->setChecked(colorNames.contains(button->color()));
    }
}

bool TagColorListWidget::exclusive() const
{
    return currentExclusive;
}

void TagColorListWidget::setExclusive(bool exclusive)
{
    currentExclusive = exclusive;
}

void TagColorListWidget::setToolTipVisible(bool visible)
{
    if (toolTip)
        toolTip->setVisible(visible);
}

void TagColorListWidget::setToolTipText(const QString &text)
{
    if (toolTip && toolTip->isVisible())
        toolTip->setText(text);
}

void TagColorListWidget::clearToolTipText()
{
    setToolTipText(QStringLiteral(" "));
}

void TagColorListWidget::initUiElement()
{
    QList<QColor> colors = TagHelper::instance()->defualtColors();

    for (const QColor &color : colors) {
        tagButtons << new TagButton(color, this);
    }

    buttonLayout = new QHBoxLayout;
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(0);

    if (useType == kAction)
        buttonLayout->addSpacing(21);
    for (int index = 0; index < tagButtons.length(); ++index) {
        tagButtons[index]->setContentsMargins(0, 0, 0, 0);
        if (useType == kAction)
            tagButtons[index]->setRadius(20);
        else
            tagButtons[index]->setRadius(20);

        QString objMark = QString("Color%1").arg(index + 1);
        tagButtons[index]->setObjectName(objMark);

        buttonLayout->addWidget(tagButtons[index], Qt::AlignCenter);
    }

    if (useType == kAction)
        buttonLayout->addSpacing(21);

    mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addLayout(buttonLayout);

    toolTip = new DLabel(this);
    toolTip->setText(QStringLiteral(" "));
    toolTip->setStyleSheet("color: #707070; font-size: 10px");
    toolTip->setObjectName("tool_tip");

    mainLayout->addWidget(toolTip, 0, Qt::AlignHCenter);
}

void TagColorListWidget::initUiForSizeMode()
{
#ifdef DTKWIDGET_CLASS_DSizeMode
    for (int index = 0; index < tagButtons.length(); ++index) {
        std::size_t size = static_cast<std::size_t>(DSizeModeHelper::element(18, 20));
        tagButtons[index]->setRadius(size);
    }
#else
    for (int index = 0; index < tagButtons.length(); ++index) {
        tagButtons[index]->setRadius(20);
    }
#endif
}

void TagColorListWidget::initConnect()
{
#ifdef DTKWIDGET_CLASS_DSizeMode
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, &TagColorListWidget::initUiForSizeMode);
#endif
    for (TagButton *button : tagButtons) {
        connect(button, &TagButton::enter, this, [this, button] {
            emit hoverColorChanged(button->color());
        });

        connect(button, &TagButton::leave, this, [this] {
            emit hoverColorChanged(QColor());
        });

        connect(button, &TagButton::checkedChanged, this, [this, button] {
            if (button->isChecked() && exclusive()) {
                for (TagButton *b : tagButtons) {
                    if (b != button)
                        b->setChecked(false);
                }
            }
        });

        connect(button, &TagButton::click, this, [this](QColor color) {
            emit checkedColorChanged(color);
        });
    }
}
