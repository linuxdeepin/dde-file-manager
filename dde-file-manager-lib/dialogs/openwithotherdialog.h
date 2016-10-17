#ifndef OPENWITHOTHERDIALOG_H
#define OPENWITHOTHERDIALOG_H

#include <ddialog.h>
#include <QListWidget>
#include "durl.h"
#include "dabstractfileinfo.h"

DWIDGET_USE_NAMESPACE

class OpenWithOtherDialog : public DDialog
{
    Q_OBJECT
public:
    explicit OpenWithOtherDialog(const DUrl& url, QWidget *parent = 0);
    ~OpenWithOtherDialog();

    void initUI();
    void initConnect();

    QListWidget *createOpenWithListWidget(const AbstractFileInfoPointer &info);

signals:

public slots:
    void handleButtonClicked(int index, QString text);

private:
    DUrl m_url;
    QListWidget* m_appListWidget = NULL;
    QButtonGroup* m_OpenWithButtonGroup = NULL;

};

#endif // OPENWITHOTHERDIALOG_H
