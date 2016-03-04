#ifndef DSEARCHBAR_H
#define DSEARCHBAR_H

#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QHBoxLayout>

class DSearchBar : public QLineEdit
{
    Q_OBJECT
public:
    explicit DSearchBar(QWidget *parent = 0);
    ~DSearchBar();
    QListWidget * getList();
private:
    QListWidget * m_list;
    QMenu * m_menu;
    QHBoxLayout * m_menuLayout;
public slots:
    void openDropDown();
    void closeDropDown();
};

#endif // DSEARCHBAR_H
