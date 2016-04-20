#ifndef OPENWITHDIALOG_H
#define OPENWITHDIALOG_H

#include <QObject>
#include "basedialog.h"

#include "durl.h"

DWIDGET_USE_NAMESPACE

class QListWidget;
class QPushButton;

class OpenWithDialog : public BaseDialog
{
    Q_OBJECT
public:
    explicit OpenWithDialog(const DUrl& url, QWidget *parent = 0);
    ~OpenWithDialog();

    void initUI();
    void initConnect();
    void addItems();

signals:

public slots:
    void openFileByApp();

private:
    QListWidget* m_listWidget = NULL;
    QPushButton* m_cancelButton = NULL;
    QPushButton* m_chooseButton = NULL;
    DUrl m_url;
};

#endif // OPENWITHDIALOG_H
