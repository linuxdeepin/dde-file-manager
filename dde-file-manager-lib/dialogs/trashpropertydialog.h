#ifndef TRASHPROPERTYDIALOG_H
#define TRASHPROPERTYDIALOG_H

#include "basedialog.h"
#include "durl.h"
#include <QLabel>
#include "propertydialog.h"

class TrashPropertyDialog : public BaseDialog
{
    Q_OBJECT
public:
    explicit TrashPropertyDialog(const DUrl& url, QWidget *parent = 0);
    ~TrashPropertyDialog();

    void initUI();
    void startComputerFolderSize(const DUrl& url);

signals:
    void requestStartComputerFolderSize();

public slots:
    void updateFolderSize(qint64 size);

private:
    DUrl m_url;
    QLabel* m_iconLabel;
    QLabel* m_nameLable;
    QLabel* m_countLabel;
    QLabel* m_sizeLabel;
};

#endif // TRASHPROPERTYDIALOG_H
