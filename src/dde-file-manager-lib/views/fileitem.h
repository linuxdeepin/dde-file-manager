// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    explicit FileIconItem(QWidget *parent = nullptr, bool desktop = false);
    ~FileIconItem() override;
    void setMaxHeight(int h);
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
    int m_maxHeight = -1;
    int m_iconSpace = 0;
    friend class DIconItemDelegate;
    friend class DFileView;
    friend class DFileViewHelper;

    QScopedPointer<FileIconItemPrivate> d_ptr;

    Q_DECLARE_PRIVATE(FileIconItem)
    Q_DISABLE_COPY(FileIconItem)
};

#endif // FILEITEM_H
