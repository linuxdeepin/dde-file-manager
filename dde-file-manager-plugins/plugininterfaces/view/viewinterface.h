#ifndef VIEWINTERFACE_H
#define VIEWINTERFACE_H

#include <QString>
#include <QIcon>
#include <QWidget>
/*
 * The view interface of dde-file-manager
 * 文件管理器的视图插件接口
*/
class ViewInterface : public QObject
{
public:
    explicit ViewInterface(QObject *parent = 0)
        : QObject(parent) {}

    virtual ~ViewInterface() {}

    /*!
     * \brief bookMarkText
     * \return QString: the text of leftbar bookmark which should be 16X16 size
     */
    /*!
     * \brief bookMarkText
     * \return QString:　侧边栏书签文本内容
     */\
    virtual QString bookMarkText(){
        return "";
    }

    /*!
     * \brief bookMarkNormalIcon
     * \return QIcon: the release icon of leftbar bookmark which should be 16X16 size
     */
    /*!
     * \brief bookMarkNormalIcon
     * \return QIcon: release状态-侧边栏书签Icon
     */
    virtual QIcon bookMarkNormalIcon(){
        return QIcon();
    }

    /*!
     * \brief bookMarkHoverIcon
     * \return QIcon: the hover icon of leftbar bookmark which should be 16X16 size
     */
    /*!
     * \brief bookMarkHoverIcon
     * \return QIcon: hover状态-侧边栏书签Icon
     */
    virtual QIcon bookMarkHoverIcon(){
        return QIcon();
    }

    /*!
     * \brief bookMarkPressedIcon
     * \return QIcon: the pressed hover icon of leftbar bookmark which should be 16X16 size
     */
    /*!
     * \brief bookMarkPressedIcon
     * \return QIcon: pressed状态-侧边栏书签Icon
     */
    virtual QIcon bookMarkPressedIcon(){
        return QIcon();
    }

    /*!
     * \brief bookMarkCheckedIcon
     * \return QIcon: the checked icon of leftbar bookmark which should be 16X16 size
     */
    /*!
     * \brief bookMarkCheckedIcon
     * \return QIcon: checked状态-侧边栏书签Icon
     */
    virtual QIcon bookMarkCheckedIcon(){
        return QIcon();
    }

    /*!
     * \brief crumbText
     * \return QString: text of first crumbButton in titlebar
     */
    /*!
     * \brief crumbText
     * \return QString: 第一个面包屑的显示文本
     */
    virtual QString crumbText(){
        return "";
    }

    /*!
     * \brief crumbNormalIcon
     * \return QIcon: the normal icon of first crumbButton in titlebar
     */
    /*!
     * \brief crumbNormalIcon
     * \return QIcon: 第一个面包屑的normal状态图标
     */
    virtual QIcon crumbNormalIcon(){
        return QIcon();
    }

    /*!
     * \brief crumbHoverIcon
     * \return QIcon: the hover icon of first crumbButton in titlebar
     */
    /*!
     * \brief crumbHoverIcon
     * \return QIcon: 第一个面包屑的normal状态图标
     */
    virtual QIcon crumbHoverIcon(){
        return QIcon();
    }

    /*!
     * \brief crumbPressedIcon
     * \return QIcon: the pressed icon of first crumbButton in titlebar
     */
    /*!
     * \brief crumbHoverIcon
     * \return QIcon: 第一个面包屑的pressed状态图标
     */
    virtual QIcon crumbPressedIcon(){
        return QIcon();
    }

    /*!
     * \brief crumbCheckedIcon
     * \return QIcon: the checked icon of first crumbButton in titlebar
     */
    /*!
     * \brief crumbHoverIcon
     * \return QIcon: 第一个面包屑的checked状态图标
     */
    virtual QIcon crumbCheckedIcon(){
        return QIcon();
    }


    /*!
     * \brief isAddSeparator
     * \return bool: addSeparator whether or not
     */
    /*!
     * \brief isAddSeparator
     * \return bool:是否添加书签分隔符
     */
    virtual bool isAddSeparator(){
        return false;
    }

    /*!
     * \brief scheme
     * \return QString: scheme of view which should by like computer 、trash、 usershare　and so on
     */
    /*!
     * \brief scheme
     * \return QString: 视图view对应的协议头，譬如computer 、trash、 usershare等
     */
    virtual QString scheme(){
        return "";
    }

    /*!
     * \brief createView
     * \return QWidget* : the view will be registered into dde-file-manager view
     */
    /*!
     * \brief createView
     * \return QWidget*: 注册到文件管理器视图区域的视图控件
     */
    virtual QWidget* createView(){
        return NULL;
    }
};

#define ViewInterface_iid "com.deepin.dde-file-manager.ViewInterface"

Q_DECLARE_INTERFACE(ViewInterface,ViewInterface_iid)

#endif // VIEWINTERFACE_H
