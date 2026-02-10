// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "collectiontitlebar_p.h"

#include <DFontSizeManager>
#include <DStyle>

#include <QEvent>
#include <QFont>
#include <QFontMetrics>
#include <QPalette>
#include <QPaintEvent>
#include <QPainterPath>

static constexpr int kNameMaxLength = 255;
static constexpr int kMenuBtnWidth = 20;
static constexpr int kMenuBtnHeight = 20;

using namespace ddplugin_organizer;
DWIDGET_USE_NAMESPACE

CollectionTitleBarPrivate::CollectionTitleBarPrivate(const QString &uuid, CollectionTitleBar *qq)
    : q(qq), id(uuid)
{
    nameLabel = new DLabel(q);
    nameLabel->setWordWrap(false);

    nameLineEdit = new DLineEdit(q);
    nameLineEdit->lineEdit()->setMaxLength(kNameMaxLength);
    nameLineEdit->setClearButtonEnabled(false);
    DStyle::setFocusRectVisible(nameLineEdit->lineEdit(), false);
    QMargins margins = nameLineEdit->lineEdit()->textMargins();
    margins.setLeft(0);
    nameLineEdit->lineEdit()->setTextMargins(margins);

    QFont nameFont = nameLineEdit->font();
    nameFont = DFontSizeManager::instance()->get(DFontSizeManager::T8, nameFont);
    nameFont.setWeight(QFont::Medium);
    nameLabel->setFont(nameFont);
    nameLineEdit->setFont(nameFont);

    QPalette palette(nameLineEdit->palette());
    palette.setColor(QPalette::WindowText, Qt::white);
    nameLabel->setPalette(palette);

    palette.setColor(QPalette::Button, Qt::transparent);
    palette.setColor(QPalette::Text, Qt::black);
    palette.setColor(QPalette::Highlight, QColor(0x00, 0x61, 0xf7));
    palette.setColor(QPalette::HighlightedText, Qt::black);
    nameLineEdit->setPalette(palette);

    nameWidget = new DStackedWidget(q);
    nameWidget->layout()->setContentsMargins(0, 0, 0, 0);

    nameWidget->addWidget(nameLabel);
    nameWidget->addWidget(nameLineEdit);
    nameWidget->setCurrentWidget(nameLabel);

    menuBtn = new OptionButton(q);
    menuBtn->setFixedSize(kMenuBtnWidth, kMenuBtnHeight);
    menuBtn->setCursor(Qt::ArrowCursor);
    menuBtn->setToolTip(tr("Collection size"));

    mainLayout = new QHBoxLayout(q);
    mainLayout->setContentsMargins(8, 2, 8, 2);
    mainLayout->setSpacing(12);
    mainLayout->addWidget(nameWidget);
    mainLayout->addWidget(menuBtn);
    q->setLayout(mainLayout);

    connect(nameLineEdit, &DLineEdit::editingFinished, this, &CollectionTitleBarPrivate::titleNameModified);
    connect(menuBtn, &OptionButton::clicked, this, &CollectionTitleBarPrivate::showMenu);

    menu = new DMenu(q);
    connect(menu, &DMenu::aboutToHide, this, [=]() {
        if (needHidden) {
            needHidden = false;
            q->setVisible(false);
        }
    });
}

CollectionTitleBarPrivate::~CollectionTitleBarPrivate()
{
}

void CollectionTitleBarPrivate::modifyTitleName()
{
    if (!renamable)
        return;

    if (q->isHidden())
        q->setHidden(false);

    nameWidget->setCurrentWidget(nameLineEdit);
    nameLineEdit->setText(titleName);
    nameLineEdit->setFocus();
    nameLineEdit->lineEdit()->setSelection(0, nameLineEdit->lineEdit()->maxLength());
}

void CollectionTitleBarPrivate::titleNameModified()
{
    if (nameLineEdit->text().trimmed().isEmpty())
        return;
    titleName = nameLineEdit->text().trimmed();

    updateDisplayName();
}

void CollectionTitleBarPrivate::updateDisplayName()
{
    nameWidget->setCurrentWidget(nameLabel);

    QFontMetrics fontMetrices(nameLabel->font());
    QString showName = fontMetrices.elidedText(titleName, Qt::ElideRight, nameLabel->width());
    nameLabel->setText(showName);
    nameLabel->setToolTip(titleName);
}

void CollectionTitleBarPrivate::showMenu()
{
    QAction *action = nullptr;
    if (adjustable) {
        action = new QAction(menu);
        action->setText(tr("Collection size"));
        menu->addAction(action);

        DMenu *subMenu = new DMenu(menu);
        action->setMenu(subMenu);

        static const char kPropertySize[] { "collection_size" };
        static const QMap<CollectionFrameSize, QString> kSizeName {
            { CollectionFrameSize::kSmall, tr("Small area") },
            { CollectionFrameSize::kMiddle, tr("Middle area") },
            { CollectionFrameSize::kLarge, tr("Large area") }
        };

        auto addAction = [=](CollectionFrameSize size) {
            QAction *action = new QAction(subMenu);
            action->setText(kSizeName.value(size));
            action->setCheckable(true);
            action->setProperty(kPropertySize, size);
            if (size == this->size)
                action->setChecked(true);
            subMenu->addAction(action);
            connect(action, &QAction::triggered,
                    this, [=]() {
                        auto size = static_cast<CollectionFrameSize>(action->property(kPropertySize).toInt());
                        emit q->sigRequestAdjustSizeMode(size);
                    });
        };

        addAction(kLarge);
        addAction(kMiddle);
        addAction(kSmall);
    }

    if (renamable) {
        action = new QAction(menu);
        action->setText(tr("Rename"));
        menu->addAction(action);
        connect(action, &QAction::triggered, this, &CollectionTitleBarPrivate::modifyTitleName);
    }

    if (closable) {
        menu->addSeparator();

        action = new QAction(menu);
        action->setText(tr("Delete"));
        menu->addAction(action);
        connect(action, &QAction::triggered, this, &CollectionTitleBarPrivate::sendRequestClose);
    }

    if (menu->actions().isEmpty())
        return;

    menu->exec(QCursor::pos());
    menu->clear();
}

void CollectionTitleBarPrivate::sendRequestClose()
{
    emit q->sigRequestClose(id);
}

CollectionTitleBar::CollectionTitleBar(const QString &uuid, QWidget *parent)
    : DBlurEffectWidget(parent), d(new CollectionTitleBarPrivate(uuid, this))
{
    setObjectName("titleBar");
    setBlendMode(DBlurEffectWidget::InWindowBlend);
    setMaskColor(QColor(0, 0, 0, static_cast<int>(0.1 * 255)));

    d->nameWidget->installEventFilter(this);

    setBlurRectXRadius(0);
    setBlurRectYRadius(0);
}

CollectionTitleBar::~CollectionTitleBar()
{
}

bool CollectionTitleBar::setTitleBarVisible(const bool &visible)
{
    // todo 显隐逻辑待根据场景细化(需求：当鼠标移出集合时，隐藏该集合的标题栏)
    // 标题栏操作过程中（正在重命名、弹出菜单选项），移出鼠标是否隐藏标题栏？
    if (!visible && d->menu->isVisible()) {
        d->needHidden = true;
        return false;
    }

    d->needHidden = false;
    this->setVisible(visible);
    return true;
}

bool CollectionTitleBar::titleBarVisible() const
{
    return this->isVisible();
}

void CollectionTitleBar::setRenamable(const bool renamable)
{
    d->renamable = renamable;
}

bool CollectionTitleBar::renamable() const
{
    return d->renamable;
}

void CollectionTitleBar::setClosable(const bool closable)
{
    d->closable = closable;
}

bool CollectionTitleBar::closable() const
{
    return d->closable;
}

void CollectionTitleBar::setAdjustable(const bool adjustable)
{
    d->adjustable = adjustable;
}

bool CollectionTitleBar::adjustable() const
{
    return d->adjustable;
}

void CollectionTitleBar::setTitleName(const QString &name)
{
    if (d->titleName == name)
        return;

    d->titleName = name;
    d->updateDisplayName();
}

QString CollectionTitleBar::titleName() const
{
    return d->titleName;
}

void CollectionTitleBar::setCollectionSize(const CollectionFrameSize &size)
{
    d->size = size;
}

CollectionFrameSize CollectionTitleBar::collectionSize() const
{
    return d->size;
}

bool CollectionTitleBar::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == d->nameWidget && QEvent::MouseButtonDblClick == event->type()) {
        d->modifyTitleName();
        return true;
    }

    return DBlurEffectWidget::eventFilter(obj, event);
}

void CollectionTitleBar::resizeEvent(QResizeEvent *e)
{
    DBlurEffectWidget::resizeEvent(e);
    rounded();
}

void CollectionTitleBar::contextMenuEvent(QContextMenuEvent *)
{
    return;
}

void CollectionTitleBar::rounded()
{
    QPainterPath path;
    const qreal radius = 8;
    QRect rect(0, 0, width() + 1, height());   // Correct 1px for width

    path.moveTo(rect.topLeft().x() + radius, rect.topLeft().y() + radius);
    path.arcTo(QRect(rect.topLeft(), QSize(radius * 2, radius * 2)), 90, 90);
    path.lineTo(rect.bottomLeft());
    path.lineTo(rect.bottomRight());
    path.lineTo(rect.topRight().x(), rect.topRight().y() - radius * 2);
    path.arcTo(QRect(QPoint(rect.topRight().x() - (radius * 2), rect.topRight().y()), QSize(radius * 2, radius * 2)), 0, 90);
    path.lineTo(rect.topLeft().x() + radius, rect.topLeft().y());

    setMaskPath(path);
}

OptionButton::OptionButton(QWidget *parent)
    : DIconButton(parent)
{
    DStyle::setFrameRadius(this, 4);

    // the icon is in resources/images.qrc and is loaded by dtk
    setIcon(QIcon::fromTheme("ddp_organizer_morebtn"));
    setIconSize(QSize(16, 16));

    // draw background by self.
    setFlat(true);

    auto pa = palette();
    pa.setColor(QPalette::ButtonText, Qt::white);   // icon is white
    setPalette(pa);
}

void OptionButton::paintEvent(QPaintEvent *event)
{
    DStylePainter p(this);
    DStyleOptionButton opt;
    initStyleOption(&opt);

    QColor background(QColor::Invalid);
    // draw backgtound on mouse over or on pressed.
    if (opt.state.testFlag(QStyle::State_MouseOver)) {
        if (opt.state.testFlag(QStyle::State_Sunken))
            background = QColor(0, 0, 0, 255 * 0.15);
        else
            background = QColor(0, 0, 0, 255 * 0.1);
    }

    if (background.isValid()) {
        int radius = DStyle::pixelMetric(style(), DStyle::PM_FrameRadius, nullptr, this);
        p.save();
        p.setRenderHint(QPainter::Antialiasing);
        p.setBrush(background);
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(QRect(QPoint(0, 0), size()), radius, radius);
        p.restore();
    }

    p.drawControl(DStyle::CE_IconButton, opt);
    event->accept();
}

void OptionButton::initStyleOption(DStyleOptionButton *option) const
{
    DIconButton::initStyleOption(option);
    // let the color of icon to like normal state on mouse hover.
    // if do not set it, the color will change to gray on hover state.
    option->features |= QStyleOptionButton::ButtonFeature(DStyleOptionButton::TitleBarButton);
}
