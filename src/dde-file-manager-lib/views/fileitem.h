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
#include <QScopedPointer>

QT_BEGIN_NAMESPACE
class QTextEdit;
class QGraphicsOpacityEffect;
QT_END_NAMESPACE

class FileIconItemEdit;
class FileIconItemPrivate;
class FileIconItem : public QFrame
{
    Q_OBJECT

    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)

public:
    explicit FileIconItem(QWidget *parent = nullptr);
    ~FileIconItem() override;

    qreal opacity() const;
    void setOpacity(qreal opacity);
    void setMaxCharSize(int maxSize);
    int maxCharSize();

    QSize sizeHint() const override;

    inline QLabel *getIconLabel() const
    { return icon; }
    inline QTextEdit *getTextEdit() const
    { return edit; }


public slots:
    /**
     * @brief showAlertMessage 显示提示信息
     * @param text  提示内容
     * @param duration  显示时间，默认3秒
     */
    void showAlertMessage(const QString &text, int duration = 3000);

signals:
    void inputFocusOut();

private slots:
    void popupEditContentMenu();
    void editUndo();
    void editRedo();
    void doLineEditTextChanged();
    void resizeFromEditTextChanged();

protected:
    void updateEditorGeometry();
    bool event(QEvent *ee) override;
    bool eventFilter(QObject *obj, QEvent *ee) override;

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
    int m_maxCharSize = INT_MAX;

    friend class DIconItemDelegate;
    friend class DFileView;
    friend class DFileViewHelper;

    QScopedPointer<FileIconItemPrivate> d_ptr;

    Q_DECLARE_PRIVATE(FileIconItem)
    Q_DISABLE_COPY(FileIconItem)
};

#endif // FILEITEM_H
