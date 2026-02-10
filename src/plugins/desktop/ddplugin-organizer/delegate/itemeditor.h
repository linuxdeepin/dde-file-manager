// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ITEMEDITOR_H
#define ITEMEDITOR_H

#include "ddplugin_organizer_global.h"

#include <DTextEdit>

#include <QFrame>
#include <QTextEdit>
#include <QStack>

class QGraphicsOpacityEffect;

DWIDGET_BEGIN_NAMESPACE
class DArrowRectangle;
DWIDGET_END_NAMESPACE

namespace ddplugin_organizer {

class RenameEdit : public DTK_WIDGET_NAMESPACE::DTextEdit
{
    Q_OBJECT
    friend class ItemEditor;

public:
    explicit RenameEdit(QWidget *parent = nullptr);
public slots:
    void undo();
    void redo();

protected:
    void pushStatck(const QString &item);
    QString stackCurrent() const;
    QString stackBack();
    QString stackAdvance();
    void adjustStyle();

protected:
    bool eventFilter(QObject *, QEvent *) override;
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
    explicit ItemEditor(QWidget *parent = nullptr);
    ~ItemEditor();
    void setBaseGeometry(const QRect &base, const QSize &itemSize, const QMargins &margin);
    QString text() const;
    void setText(const QString &text);
    void select(const QString &part);
    void setOpacity(qreal opacity);
    inline void setMaxHeight(int h) { maxHeight = h; }
    inline RenameEdit *editor() const { return textEditor; }
    inline void setMaximumLength(int l)
    {
        if (l > 0)
            maxTextLength = l;
    }

    inline int maximumLength() const
    {
        return maxTextLength;
    }
    inline void setCharCountLimit()
    {
        useCharCount = true;
    }
public slots:
    void updateGeometry();
    void showAlertMessage(const QString &text, int duration = 3000);
signals:
    void inputFocusOut();

protected:
    static RenameEdit *createEditor();
    static DTK_WIDGET_NAMESPACE::DArrowRectangle *createTooltip();
private slots:
    void textChanged();

private:
    void init();

protected:
    int maxHeight = -1;
    int maxTextLength = INT_MAX;
    bool useCharCount = false;
    RenameEdit *textEditor = nullptr;
    QSize itemSizeHint;
    QGraphicsOpacityEffect *opacityEffect = nullptr;
    DTK_WIDGET_NAMESPACE::DArrowRectangle *tooltip = nullptr;
};

}

#endif   // ITEMEDITOR_H
