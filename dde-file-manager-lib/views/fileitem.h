/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#ifndef FILEITEM_H
#define FILEITEM_H

#include <QLabel>
#include <QStack>

QT_BEGIN_NAMESPACE
class QTextEdit;
class QGraphicsOpacityEffect;
QT_END_NAMESPACE

class FileIconItemEdit;
class FileIconItem : public QFrame
{
    Q_OBJECT

    Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor NOTIFY borderColorChanged)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)

public:
    explicit FileIconItem(QWidget *parent = 0);

    qreal opacity() const;
    void setOpacity(qreal opacity);
    void setMaxCharSize(int maxSize);

    QSize sizeHint() const Q_DECL_OVERRIDE;

    inline QLabel* getIconLabel() const
    { return icon; }
    inline QTextEdit* getTextEdit() const
    { return edit; }

    QColor borderColor() const;

public slots:
    void setBorderColor(QColor borderColor);

signals:
    void inputFocusOut();
    void borderColorChanged(QColor borderColor);

private slots:
    void popupEditContentMenu();
    void editUndo();
    void editRedo();

protected:
    void updateEditorGeometry();
    bool event(QEvent *ee) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *obj, QEvent *ee) Q_DECL_OVERRIDE;

private:
    void updateStyleSheet();

    QString editTextStackCurrentItem() const;
    QString editTextStackBack();
    QString editTextStackAdvance();
    void pushItemToEditTextStack(const QString &item);

    bool canDeferredDelete = true;
    QLabel *icon;
    QTextEdit *edit;
    int editTextStackCurrentIndex = -1;
    bool disableEditTextStack = false;
    QStack<QString> editTextStack;
    QGraphicsOpacityEffect *opacityEffect = Q_NULLPTR;
    QColor m_borderColor;
    int maxCharSize = INT_MAX;

    friend class DIconItemDelegate;
    friend class DFileView;
    friend class DFileViewHelper;
};

#endif // FILEITEM_H
