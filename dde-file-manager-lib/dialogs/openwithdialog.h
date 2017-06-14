#ifndef OPENWITHDIALOG_H
#define OPENWITHDIALOG_H

#include "basedialog.h"
#include "durl.h"

#include <dflowlayout.h>
#include <dlinkbutton.h>

#include <QObject>
#include <QMimeType>

DWIDGET_USE_NAMESPACE

QT_BEGIN_NAMESPACE
class QPushButton;
class QScrollArea;
class QCheckBox;
QT_END_NAMESPACE

class OpenWithDialogListItem;
class OpenWithDialog : public BaseDialog
{
    Q_OBJECT
public:
    explicit OpenWithDialog(const DUrl& url, QWidget *parent = 0);
    ~OpenWithDialog();

public slots:
    void openFileByApp();

protected:
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;

private:
    void initUI();
    void initConnect();
    void initData();
    void checkItem(OpenWithDialogListItem *item);
    void useOtherApplication();
    OpenWithDialogListItem *createItem(const QIcon &icon, const QString &name, const QString &filePath);

    QScrollArea *m_scrollArea = NULL;
    DFlowLayout *m_recommandLayout = NULL;
    DFlowLayout *m_otherLayout = NULL;

    DLinkButton *m_openFileChooseButton = NULL;
    QCheckBox *m_setToDefaultCheckBox = NULL;
    QPushButton *m_cancelButton = NULL;
    QPushButton *m_chooseButton = NULL;
    DUrl m_url;
    QMimeType m_mimeType;

    OpenWithDialogListItem *m_checkedItem = NULL;
};

#endif // OPENWITHDIALOG_H
