#ifndef DDESKTOPRENAMEDIALOG_H
#define DDESKTOPRENAMEDIALOG_H


#include <QSharedPointer>

#include "ddialog.h"
#include "interfaces/dfileservices.h"


DTK_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

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
    QPair<QString, std::size_t> getModeThreeContent()const noexcept;


    void setDialogTitle(const QString& tile)noexcept;


private slots:
    void onCurrentModeChanged(const std::size_t& index)noexcept;
    void onCurrentAddModeChanged(const std::size_t& index)noexcept;

    void onContentChangedForFinding(const QString& content);
    void onContentChangedForReplacing(const QString& content)noexcept;
    void onContentChangedForAdding(const QString& content);
    void onContentChangedForCustomizing(const QString& content);
    void onSNNumberChanged(const QString& content);

private:
    using DDialog::setWindowTitle;

    void initUi();
    void initConnect()noexcept;
    void setRenameButtonStatus(const bool& enabled);

    QSharedPointer<DDesktopRenameDialogPrivate> d_ptr{ nullptr };
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), DDesktopRenameDialog)
};


#endif // DDESKTOPRENAMEDIALOG_H
