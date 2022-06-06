/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#ifndef ITEMEDITOR_H
#define ITEMEDITOR_H

#include "ddplugin_canvas_global.h"

#include <DTextEdit>

#include <QFrame>
#include <QTextEdit>
#include <QStack>

class QGraphicsOpacityEffect;

DWIDGET_BEGIN_NAMESPACE
class DArrowRectangle;
DWIDGET_END_NAMESPACE

DDP_CANVAS_BEGIN_NAMESPACE
class RenameEdit: public DTK_WIDGET_NAMESPACE::DTextEdit
{
    Q_OBJECT
    friend class ItemEditor;
public slots:
    void undo();
    void redo();
protected:
    void pushStatck(const QString &item);
    QString stackCurrent() const;
    QString stackBack();
    QString stackAdvance();
protected:
    using DTextEdit::DTextEdit;
    void contextMenuEvent(QContextMenuEvent *e) override;
    void focusOutEvent(QFocusEvent *e) override;
    void keyPressEvent(QKeyEvent *e) override;
    void showEvent(QShowEvent *) override;
private:
    bool enableStack = true;
    int stackCurrentIndex = -1;
    QStack<QString> textStack;
};

class ItemEditor : public QFrame
{
    Q_OBJECT
public:
    explicit ItemEditor(QWidget* parent = nullptr);
    ~ItemEditor();
    void setBaseGeometry(const QRect &base, const QSize &itemSize, const QMargins &margin);
    QString text() const;
    void setText(const QString &text);
    void setItemSizeHint(QSize size);
    void select(const QString &part);
    void setOpacity(qreal opacity);
    inline void setMaximumLength(int l) {
        if (l > 0)
            maxTextLength = l;
    }

    inline int maximumLength() const {
        return maxTextLength;
    }
public slots:
    void updateGeometry();
    void showAlertMessage(const QString &text, int duration = 3000);
signals:
    void inputFocusOut();
protected:
    static RenameEdit *createEditor();
    static DTK_WIDGET_NAMESPACE::DArrowRectangle *createTooltip();
    bool processLength(const QString &srcText, int srcPos, QString &dstText, int &dstPos);
private slots:
    void textChanged();
private:
    void init();
protected:
    int maxTextLength = INT_MAX;
    RenameEdit *textEditor = nullptr;
    QSize itemSizeHint;
    QGraphicsOpacityEffect *opacityEffect = nullptr;
    DTK_WIDGET_NAMESPACE::DArrowRectangle *tooltip = nullptr;
};

DDP_CANVAS_END_NAMESPACE

#endif // ITEMEDITOR_H
