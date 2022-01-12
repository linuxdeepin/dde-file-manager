/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#include "views/private/addressbar_p.h"
#include "views/addressbar.h"

#include <QCompleter>

DPTITLEBAR_USE_NAMESPACE

AddressBar::AddressBar(QWidget *parent)
    : QLineEdit(parent), d(new AddressBarPrivate(this))
{
}

bool AddressBar::completerViewVisible()
{
    return d->completerView->isVisible();
}

bool AddressBar::event(QEvent *e)
{
    // blumia: When window lost focus and then get activated, we should hide
    //         addressbar if it's visiable.
    if (e->type() == QEvent::WindowActivate) {
        if (!hasFocus() && isVisible()) {
            Q_EMIT lostFocus();
        }
    }

    if (e->type() == QEvent::KeyPress) {
        keyPressEvent(static_cast<QKeyEvent *>(e));
        return true;
    }

    return QLineEdit::event(e);
}

void AddressBar::focusInEvent(QFocusEvent *e)
{
    if (completer())
        completer()->setWidget(this);
    return QLineEdit::focusInEvent(e);
}

void AddressBar::focusOutEvent(QFocusEvent *e)
{
    // blumia: Sometimes completion will trigger weird Qt::ActiveWindowFocusReason event,
    //         right click context menu will trigger Qt::PopupFocusReason event. It will
    //         cause focusOutEvent. So we simply ignore it here.
    // blumia: 2019/12/01: seems now based on current 5.11.3.2+c1-1+deepin version of Qt,
    //         completion will no longer trigger Qt::ActiveWindowFocusReason reason focus
    //         out event, so we comment out this case for now and see if it still happens.
    // fix bug#38455 文管启动后第一次点击搜索，再点击筛选按钮，会导致搜索框隐藏
    // 第一次点击筛选按钮，会发出Qt::OtherFocusReason信号导致搜索框隐藏，所以将其屏蔽
    if (/*e->reason() == Qt::ActiveWindowFocusReason || */ e->reason() == Qt::PopupFocusReason || e->reason() == Qt::OtherFocusReason) {
        e->accept();
        setFocus();
        return;
    }
    d->completerView->hide();
    emit lostFocus();
    return QLineEdit::focusOutEvent(e);
}

void AddressBar::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_Escape:
        emit escKeyPressed();
        e->accept();
        return;
    default:
        break;
    }

    if (completer() && completer()->popup()->isVisible()) {
        // The following keys are forwarded by the completer to the widget
        switch (e->key()) {

            // TODO(zhangs):isHistoryInCompleterModel

        case Qt::Key_Backtab:
            e->ignore();
            return;
        case Qt::Key_Enter:
        case Qt::Key_Return:
            e->accept();
            d->completerView->hide();
            emit returnPressed();
            return;
        case Qt::Key_Tab:
            if (completer()->completionCount() > 0) {
                if (selectedText().isEmpty()) {
                    QString completeResult = completer()->completionModel()->index(0, 0).data().toString();
                    // TODO(zhangs): insertCompletion(completeResult);
                }
                if (QUrl::fromUserInput(text()).isLocalFile()) {
                    setText(text() + '/');
                    emit textEdited(text());
                }
            }
            e->accept();
            return;
        //解决bug19609文件管理器中，文件夹搜索功能中输入法在输入过程中忽然失效然后恢复
        case Qt::Key_Up:
        case Qt::Key_Down:
            d->completerView->keyPressEvent(e);
            break;
        default:
            break;
        }
        setFocus();
    } else {
        // If no compiler
        switch (e->key()) {
        case Qt::Key_Tab:
            e->accept();
            return;
        case Qt::Key_Enter:
        case Qt::Key_Return:
            emit returnPressed();
            e->accept();
            return;
        default:
            break;
        }
    }

    return QLineEdit::keyPressEvent(e);
}

void AddressBar::paintEvent(QPaintEvent *e)
{
    QLineEdit::paintEvent(e);

    // addressbar animation
    QPainter painter(this);

    //设置提示text
    if (text().isEmpty()) {
        QPen oldpen = painter.pen();
        QColor phColor = palette().text().color();
        const int flags = static_cast<int>(QStyle::visualAlignment(Qt::LeftToRight, QFlag(Qt::AlignCenter)));

        phColor.setAlpha(128);
        painter.setPen(phColor);

        painter.drawText(rect(), flags, placeholderText());

        painter.setPen(oldpen);
    }
    //绘制波纹效果
    if (d->animation.state() != QAbstractAnimation::Stopped) {

        QIcon icon = QIcon::fromTheme("dfm_addressbar_glowing");
        if (icon.availableSizes().isEmpty())
            return;
        const QSize size = icon.availableSizes().first();
        QPixmap glowingImg = icon.pixmap(size);
        float curValue = d->animation.currentValue().toFloat();
        float xPos = (width() + glowingImg.width()) * curValue - glowingImg.width();

        painter.drawPixmap(static_cast<int>(xPos), 0, glowingImg);
    }
}

void AddressBar::showEvent(QShowEvent *event)
{
    d->timer.start();
    stopSpinner();

    return QLineEdit::showEvent(event);
}

// 解决bug19609文件管理器中，文件夹搜索功能中输入法在输入过程中忽然失效然后恢复
void AddressBar::inputMethodEvent(QInputMethodEvent *e)
{
    if (hasSelectedText()) {
        // TODO(zhangs): imple me
        // fix bug#31692 搜索框输入中文后,全选已输入的,再次输入未覆盖之前的内容
        //        int pos = selectPosStart;
        //        setText(lastEditedString.remove(selectPosStart, selectLength));
        //        // 设置光标到修改处
        //        setCursorPosition(pos);
    }
    QLineEdit::inputMethodEvent(e);
}

void AddressBar::enterEvent(QEvent *e)
{
    // TODO(zhangs): imple me

    //    if (indicatorType == Search && d->spinner && d->spinner->isPlaying()) {
    //        animationSpinner->hide();
    //        pauseButton->setVisible(true);
    //    }

    QLineEdit::enterEvent(e);
}

void AddressBar::leaveEvent(QEvent *e)
{
    // TODO(zhangs): imple me

    //    if (indicatorType == Search && animationSpinner && animationSpinner->isPlaying()) {
    //        pauseButton->setVisible(false);
    //        animationSpinner->show();
    //    }

    QLineEdit::leaveEvent(e);
}

void AddressBar::startSpinner()
{
    d->startSpinner();
}

void AddressBar::stopSpinner()
{
    d->stopSpinner();
}
