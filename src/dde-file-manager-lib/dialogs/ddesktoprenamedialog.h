// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DDESKTOPRENAMEDIALOG_H
#define DDESKTOPRENAMEDIALOG_H


#include <QSharedPointer>

#include "ddialog.h"
#include "interfaces/dfileservices.h"

using namespace Dtk::Widget;

class DDesktopRenameDialogPrivate;
class DDesktopRenameDialog : public DDialog
{
    Q_OBJECT

public:
    explicit DDesktopRenameDialog(QWidget* const parent = nullptr);
    virtual ~DDesktopRenameDialog()=default;

    DDesktopRenameDialog(const DDesktopRenameDialog& other)=delete;
    DDesktopRenameDialog& operator=(const DDesktopRenameDialog& other)=delete;


    std::size_t getCurrentModeIndex()const noexcept;
    DFileService::AddTextFlags getAddMode()const noexcept;

    QPair<QString, QString> getModeOneContent()const noexcept;
    QPair<QString, DFileService::AddTextFlags> getModeTwoContent()const noexcept;
    QPair<QString, QString> getModeThreeContent()const noexcept;

    ///###: when is invoking DDesktopRenameDialog::show invoke this function actually.
    virtual void setVisible(bool visible) override;
    void setDialogTitle(const QString& tile)noexcept;

signals:
    void visibleChanged(bool visible);
    void clickCancelButton();
    void clickRenameButton();

private slots:
    void onCurrentModeChanged(const std::size_t& index)noexcept;
    void onCurrentAddModeChanged(const std::size_t& index)noexcept;

    void onContentChangedForFinding(const QString& content);
    void onContentChangedForAdding(const QString& content);
    void onContentChangedForCustomzedSN(const QString& content);
    void onVisibleChanged(bool visible)noexcept;
    void onReplaceTextChanged()noexcept;

private:
    using DDialog::setWindowTitle;

    void initUi();
    void initConnect();
    void setRenameButtonStatus(const bool& enabled);

    QSharedPointer<DDesktopRenameDialogPrivate> d_ptr{ nullptr };
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), DDesktopRenameDialog)
};



#endif // DDESKTOPRENAMEDIALOG_H
