// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ICONITEMEDITOR_H
#define ICONITEMEDITOR_H

#include "dfmplugin_workspace_global.h"

#include <DArrowRectangle>

#include <QLabel>
#include <QStack>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE
class QTextEdit;
class QGraphicsOpacityEffect;
QT_END_NAMESPACE

namespace dfmplugin_workspace {

class IconItemEditorPrivate;

class IconItemEditor : public QFrame
{
    Q_OBJECT

    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)

public:
    explicit IconItemEditor(QWidget *parent = nullptr);
    ~IconItemEditor() override;

    QString text() const;
    void setText(const QString &text);
    void select(const QString &part);

    qreal opacity() const;
    void setOpacity(qreal opacity);
    void setMaxCharSize(int maxSize);
    int maxCharSize() const;

    void setMaxHeight(int h);

    QSize sizeHint() const override;
    QLabel *getIconLabel() const;
    QTextEdit *getTextEdit() const;
    bool isEditReadOnly() const;
    void setCharCountLimit();

public slots:
    void showAlertMessage(const QString &text, int duration = 3000);

signals:
    void inputFocusOut();

private slots:
    void popupEditContentMenu();
    void editUndo();
    void editRedo();
    void onEditTextChanged();
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
    static DTK_WIDGET_NAMESPACE::DArrowRectangle *createTooltip();

    friend class IconItemDelegate;
    friend class FileView;
    friend class FileViewHelper;

    QScopedPointer<IconItemEditorPrivate> dd;

    Q_DECLARE_PRIVATE_D(dd, IconItemEditor)
    Q_DISABLE_COPY(IconItemEditor)
};
}

#endif   // ICONITEMEDITOR_H
