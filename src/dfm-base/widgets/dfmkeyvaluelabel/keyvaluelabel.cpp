// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "keyvaluelabel.h"

#include <QFontMetrics>
#include <QMenu>
#include <QClipboard>
#include <QApplication>

#include <dtkwidget_global.h>
#ifdef DTKWIDGET_CLASS_DSizeMode
#    include <DSizeMode>
#endif

static constexpr int kMaxShowRowNum { 4 };
static constexpr char kSelectAllName[] { "select-all" };

DWIDGET_USE_NAMESPACE
using namespace dfmbase;

KeyValueLabel::KeyValueLabel(QWidget *parent)
    : QFrame(parent)
{
    initUI();
    initPropertyMap();
    initFont();
    initUiForSizeMode();
#ifdef DTKWIDGET_CLASS_DSizeMode
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, &KeyValueLabel::initUiForSizeMode);
#endif
}

KeyValueLabel::~KeyValueLabel()
{
}

void KeyValueLabel::initUI()
{
    leftValueLabel = new DLabel(this);
    rightValueEdit = new RightValueWidget(this);
    connect(rightValueEdit, &RightValueWidget::clicked, this, &KeyValueLabel::valueAreaClicked);
    rightValueEdit->setMinimumWidth(130);
    glayout = new QGridLayout;
    glayout->setContentsMargins(0, 0, 0, 0);
    glayout->addWidget(leftValueLabel, 0, 0);
    glayout->addWidget(rightValueEdit, 0, 1);
    glayout->setColumnStretch(0, 1);
    glayout->setColumnStretch(1, 3);
    setLayout(glayout);
}

void KeyValueLabel::initUiForSizeMode()
{
#ifdef DTKWIDGET_CLASS_DSizeMode
    leftValueLabel->setMaximumWidth(DSizeModeHelper::element(65, 100));
#else
    leftValueLabel->setMaximumWidth(100);
#endif
}

void KeyValueLabel::initPropertyMap()
{
    propertyMap.insert(kLeftValue, -1);
    propertyMap.insert(kLeftElideMode, -1);
    propertyMap.insert(kLeftTipVisible, false);
    propertyMap.insert(kLeftAlignment, -1);
    propertyMap.insert(kLeftWordWrap, -1);
    propertyMap.insert(kRightValue, {});
    propertyMap.insert(kRightElideMode, -1);
    propertyMap.insert(kRightTipVisible, false);
    propertyMap.insert(kRightAlignment, -1);
    propertyMap.insert(kRightWordWrap, true);
}

void KeyValueLabel::initFont()
{
    setLeftFontSizeWeight(DFontSizeManager::T9);
    setRightFontSizeWeight(DFontSizeManager::T9);
}

/*!
 * \brief                       设置左值以及相应的显示方式
 * \param value                 需要显示的信息
 * \param elideMode             省略显示的方式
 * \param aligment              信息显示对齐方式
 * \param toolTipVisibility     是否设置toolTip
 */
void KeyValueLabel::setLeftValue(QString value, Qt::TextElideMode elideMode, Qt::Alignment aligment, bool toolTipVisibility, int fontMinWidth)
{
    QString elideNote = value;
    leftValueLabel->setAlignment(aligment);
    leftValueLabel->setContentsMargins(0, 4, 0, 0);
    QFontMetrics fontWidth(leftValueLabel->font());
    int fontW = leftValueLabel->width();
    if (fontW > fontMinWidth)
        fontW = fontMinWidth;
    elideNote = fontWidth.elidedText(value, elideMode, fontW);
    leftValueLabel->setText(elideNote);
    if (toolTipVisibility) {
        if (elideNote != value)
            leftValueLabel->setToolTip(value);
    }
    propertyMap[kLeftValue] = QVariant::fromValue(value);
    propertyMap[kLeftElideMode] = QVariant::fromValue(elideMode);
    propertyMap[kLeftTipVisible] = QVariant::fromValue(toolTipVisibility);
    propertyMap[kLeftAlignment] = QVariant::fromValue(aligment);
    propertyMap[kLeftFontWidth] = QVariant::fromValue(fontW);
}

/*!
 * \brief                       设置右值以及相应的显示方式
 * \param value                 需要显示的信息
 * \param elideMode             省略显示的方式
 * \param aligment              信息显示对齐方式
 * \param toolTipVisibility     是否设置toolTip
 */
void KeyValueLabel::setRightValue(QString value, Qt::TextElideMode elideMode, Qt::Alignment aligment, bool toolTipVisibility, int fontMinWidth)
{
    rightValueEdit->setAlignment(aligment);
    QFontMetrics fontM(rightValueEdit->font());
    int fontW = rightValueEdit->width() * kMaxShowRowNum - fontM.averageCharWidth() * (kMaxShowRowNum + 1);
    if (fontW < fontMinWidth)
        fontW = fontMinWidth;
    QString elideNote = fontM.elidedText(value, elideMode, fontW);
    rightValueEdit->setCompleteText(value);
    rightValueEdit->setText(elideNote);
    if (toolTipVisibility) {
        if (elideNote != value)
            rightValueEdit->setToolTip(value);
    }

    propertyMap[kRightValue] = QVariant::fromValue(value);
    propertyMap[kRightElideMode] = QVariant::fromValue(elideMode);
    propertyMap[kRightTipVisible] = QVariant::fromValue(toolTipVisibility);
    propertyMap[kRightAlignment] = QVariant::fromValue(aligment);
    propertyMap[kRightFontWidth] = QVariant::fromValue(fontW);
}

/*!
 * \brief                       设置左右右值以及相应的显示方式
 * \param leftValue             需要显示的左值信息
 * \param rightValue            需要显示的右值信息
 * \param elideMode             省略显示的方式
 * \param aligment              信息显示对齐方式
 * \param toolTipVisibility     是否设置toolTip
 */
void KeyValueLabel::setLeftRightValue(QString leftValue, QString rightValue, Qt::TextElideMode elideMode, Qt::Alignment aligment, bool toolTipVisibility)
{
    setLeftValue(leftValue, elideMode, aligment, toolTipVisibility);
    setRightValue(rightValue, elideMode, aligment, toolTipVisibility);
}

void KeyValueLabel::adjustHeight()
{
    rightValueEdit->setFixedHeight(static_cast<int>(rightValueEdit->document()->size().height()) + 2);
}

/*!
 * \brief           设置左值是否可换行显示
 * \param wordWrap  true为可换行，反之亦然
 */
void KeyValueLabel::setLeftWordWrap(bool wordWrap)
{
    leftValueLabel->setWordWrap(wordWrap);
    propertyMap[kLeftWordWrap] = QVariant::fromValue(wordWrap);
}

/*!
 * \brief                   设置左值字体大小、风格以及颜色类型
 * \param sizeType          使用DTK DFontSizeManager类中SizeType枚举值
 * \param fontWeight        字体的风格（如粗体等）
 * \param foregroundRole    颜色类型(ColorType 参考DTK DPalette)
 */
void KeyValueLabel::setLeftFontSizeWeight(DFontSizeManager::SizeType sizeType, QFont::Weight fontWeight, DPalette::ColorType foregroundRole)
{
    DFontSizeManager::instance()->bind(leftValueLabel, sizeType, fontWeight);
    leftValueLabel->setForegroundRole(foregroundRole);
}

/*!
 * \brief                   设置右值字体大小、风格以及颜色类型
 * \param sizeType          使用DTK DFontSizeManager类中SizeType枚举值
 * \param fontWeight        字体的风格（如粗体等）
 * \param foregroundRole    颜色类型(ColorType 参考DTK DPalette)
 */
void KeyValueLabel::setRightFontSizeWeight(DFontSizeManager::SizeType sizeType, QFont::Weight fontWeight, DPalette::ColorType foregroundRole)
{
    DFontSizeManager::instance()->bind(rightValueEdit, sizeType, fontWeight);
}

QString KeyValueLabel::LeftValue()
{
    return leftValueLabel->text();
}

QString KeyValueLabel::RightValue()
{
    return rightValueEdit->toPlainText();
}

DLabel *KeyValueLabel::leftWidget()
{
    return leftValueLabel;
}

void KeyValueLabel::setLeftVauleLabelFixedWidth(int width)
{
    leftValueLabel->setFixedWidth(width);
}

RightValueWidget *KeyValueLabel::rightWidget()
{
    return rightValueEdit;
}

RightValueWidget::RightValueWidget(QWidget *parent)
    : QTextEdit(parent)
{
    setReadOnly(true);
    setFrameShape(QFrame::NoFrame);
    setWordWrapMode(QTextOption::WrapAnywhere);
    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, &RightValueWidget::customContextMenuRequested, this, &RightValueWidget::customContextMenuEvent);
}

void RightValueWidget::setCompleteText(const QString &text)
{
    completeText = text;
}

void RightValueWidget::mouseReleaseEvent(QMouseEvent *event)
{
    QTextEdit::mouseReleaseEvent(event);
    Q_EMIT clicked();
}

void RightValueWidget::showEvent(QShowEvent *event)
{
    if (!document()->toPlainText().isEmpty())
        this->setFixedHeight(static_cast<int>(document()->size().height()) + 2);

    QTextEdit::showEvent(event);
}

void RightValueWidget::focusOutEvent(QFocusEvent *e)
{
    if (!isContextMenuShow)
        moveCursor(QTextCursor::Start);
    isContextMenuShow = false;

    QTextEdit::focusOutEvent(e);
}

void RightValueWidget::customContextMenuEvent(const QPoint &pos)
{
    isContextMenuShow = true;

    const QPoint &curPos = mapToGlobal(pos);
    QMenu *menu = createStandardContextMenu(curPos);
    if (!menu)
        return;

    QList<QAction *> acts = menu->actions();
    const QString text = document()->toPlainText();
    for (int i = 0; i < acts.size(); ++i) {
        const QString &objectName = acts.at(i)->objectName();
        if (objectName == QString(kSelectAllName)) {
            if (textCursor().selectedText() == text)
                acts.at(i)->setEnabled(false);
            else
                acts.at(i)->setEnabled(true);
            break;
        }
    }

    QAction *copyComplete { nullptr };
    if (text != completeText) {
        copyComplete = new QAction(tr("Copy complete info"), menu);
        connect(copyComplete, &QAction::triggered, this, [this]() {
            QApplication::clipboard()->setText(completeText);
        });
        if (acts.size() > 0)
            menu->insertAction(acts[0], copyComplete);
        else
            menu->addAction(copyComplete);
    }

    menu->exec(curPos);

    if (copyComplete) {
        menu->removeAction(copyComplete);
        delete copyComplete;
    }
    delete menu;
}
