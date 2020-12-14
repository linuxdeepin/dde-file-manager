#ifndef DDESKTOPRENAMEDIALOG_P_H
#define DDESKTOPRENAMEDIALOG_P_H

#include <QString>
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>
#include <QHBoxLayout>
#include <QStackedLayout>

#include "interfaces/dfileservices.h"

class DDesktopRenameDialog;

class DDesktopRenameDialogPrivate
{
public:
    explicit DDesktopRenameDialogPrivate(DDesktopRenameDialog *const qPtr);
    ~DDesktopRenameDialogPrivate() = default;
    DDesktopRenameDialogPrivate(const DDesktopRenameDialogPrivate &other) = delete;
    DDesktopRenameDialogPrivate &operator=(const DDesktopRenameDialogPrivate &other) = delete;


    void initUi();
    void initUiParameters();
    void initUiLayout();

    QString filteringText(const QString &text); //bug 26533
    void updateLineEditText(QLineEdit *lineEdit, const QString &defaultValue = ""); //bug 26533

    DDesktopRenameDialog *q_ptr{ nullptr };

    QLabel *m_titleLabel{ nullptr };

    std::tuple<QLabel *, QComboBox *, QHBoxLayout *, QFrame *> m_itemsForSelecting{};
    QStackedLayout *m_stackedLayout{ nullptr };

    std::tuple<QLabel *, QLineEdit *, QHBoxLayout *> m_modeOneItemsForFinding{};
    std::tuple<QLabel *, QLineEdit *, QHBoxLayout *> m_modeOneItemsForReplacing{};
    QPair<QVBoxLayout *, QFrame *> m_modeOneLayout{};


    std::tuple<QLabel *, QLineEdit *, QHBoxLayout *> m_modeTwoItemsForAdding{};
    std::tuple<QLabel *, QComboBox *, QHBoxLayout *> m_modeTwoItemsForLocating{};
    QPair<QVBoxLayout *, QFrame *> m_modeTwoLayout{};


    std::tuple<QLabel *, QLineEdit *, QHBoxLayout *> m_modeThreeItemsForFileName{};
    std::tuple<QLabel *, QLineEdit *, QHBoxLayout *> m_modeThreeItemsForSNNumber{};
    QPair<QVBoxLayout *, QFrame *> m_modeThreeLayout{};
    QRegExpValidator *m_validator{ nullptr };

    QVBoxLayout *m_mainLayout{ nullptr };
    QFrame *m_mainFrame{ nullptr };
    std::size_t m_currentmode{ 0 };      //###: reocord current mode.
    std::array<bool, 3>  m_currentEnabled{{ false }}; //###: this array record current status of renamebutton at current mode.
    DFileService::AddTextFlags m_flagForAdding{ DFileService::AddTextFlags::Before }; //###: this flag is useful in mode 2. It record what to append str to name.
};

#endif // DDESKTOPRENAMEDIALOG_P_H
