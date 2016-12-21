#ifndef DCRUMBBUTTON_H
#define DCRUMBBUTTON_H

#include <QPushButton>
#include <QListWidgetItem>
#include "durl.h"

class DCrumbButton : public QPushButton
{
    Q_OBJECT
public:
    DCrumbButton(int index, const QString &text, QWidget *parent = 0);
    DCrumbButton(int index, const QIcon& icon, const QString &text, QWidget *parent = 0);
    int getIndex();
    QString getName();
    void setItem(QListWidgetItem* item);
    void setListWidget(QListWidget* widget);
    QListWidgetItem* getItem();

    DUrl url() const;
    void setUrl(const DUrl& url);

protected:
    void paintEvent(QPaintEvent *e) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *e) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *e) Q_DECL_OVERRIDE;

private:
    int m_index;
    QString m_name;
    QString m_path;
    DUrl m_url;
    QListWidgetItem* m_item;
    QListWidget* m_listWidget;
    QPoint oldGlobalPos;
};

class DCrumbIconButton : public DCrumbButton
{
    Q_OBJECT
public:
    DCrumbIconButton(int index, const QIcon& normalIcon, const QIcon& hoverIcon, const QIcon& checkedIcon, const QString &text, QWidget *parent = 0);
private:
    QIcon m_normalIcon;
    QIcon m_hoverIcon;
    QIcon m_checkedIcon;
protected:
    void checkStateSet();
    void nextCheckState();
};

#endif // DCRUMBBUTTON_H
