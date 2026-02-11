// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPENWITHDIALOG_H
#define OPENWITHDIALOG_H

#include "dfmplugin_utils_global.h"

#include <dfm-base/dialogs/basedialog/basedialog.h>

#include <dflowlayout.h>
#include <DIconButton>

#include <DCommandLinkButton>
#include <DPushButton>
#include <DSuggestButton>
#include <DLabel>
#include <QUrl>
#include <QMimeType>

QT_BEGIN_NAMESPACE
class QPushButton;
class QScrollArea;
class QCheckBox;
QT_END_NAMESPACE

DWIDGET_USE_NAMESPACE
namespace dfmplugin_utils {

class OpenWithDialogListItem : public QWidget
{
    Q_OBJECT

public:
    explicit OpenWithDialogListItem(const QString &iconName, const QString &text, QWidget *parent = nullptr);

    void setChecked(bool checked);
    QString text() const;

protected:
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    void enterEvent(QEvent *e) override;
#else
    void enterEvent(QEnterEvent *e) override;
#endif

    void leaveEvent(QEvent *e) override;
    void paintEvent(QPaintEvent *e) override;

public slots:
    void initUiForSizeMode();

private:
    void updateLabelIcon(int size);

    QString iconName;
    DIconButton *checkButton;
    DLabel *iconLabel;
    DLabel *label;
};

class OpenWithDialog : public DFMBASE_NAMESPACE::BaseDialog
{
    Q_OBJECT
public:
    explicit OpenWithDialog(const QList<QUrl> &list, QWidget *parent = nullptr);
    explicit OpenWithDialog(const QUrl &url, QWidget *parent = nullptr);
    ~OpenWithDialog() override;

public slots:
    void openFileByApp();

protected:
    void showEvent(QShowEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void initUiForSizeMode();

private:
    void initUI();
    void initConnect();
    void initData();
    void checkItem(OpenWithDialogListItem *item);
    void useOtherApplication();
    OpenWithDialogListItem *createItem(const QString &iconName, const QString &name, const QString &filePath);

    QScrollArea *scrollArea { nullptr };
    DTK_WIDGET_NAMESPACE::DFlowLayout *recommandLayout { nullptr };
    DTK_WIDGET_NAMESPACE::DFlowLayout *otherLayout { nullptr };

    DTK_WIDGET_NAMESPACE::DCommandLinkButton *openFileChooseButton { nullptr };
    QCheckBox *setToDefaultCheckBox { nullptr };
    DPushButton *cancelButton { nullptr };
    DSuggestButton *chooseButton { nullptr };
    QList<QUrl> urlList;
    QUrl curUrl;
    QMimeType mimeType;

    OpenWithDialogListItem *checkedItem { nullptr };
};
}
#endif   // OPENWITHDIALOG_H
