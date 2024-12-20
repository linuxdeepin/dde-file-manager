// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "clipboardmonitor.h"

#include <QDebug>
#include <QApplication>

#include <xcb/xfixes.h>
#include <xcb/xcb.h>
//#include <xcb/xcb_event.h>


#include <string.h>

using namespace std;

static xcb_atom_t primaryAtom = XCB_ATOM_NONE;
static xcb_atom_t clipboardAtom = XCB_ATOM_NONE;
static xcb_atom_t utf8StringAtom = XCB_ATOM_NONE;
static xcb_atom_t textAtom = XCB_ATOM_NONE;
static xcb_atom_t uriListAtom = XCB_ATOM_NONE;
static xcb_atom_t targetsAtom = XCB_ATOM_NONE;
static xcb_atom_t wlSelectionAtom = XCB_ATOM_NONE;
static xcb_atom_t timestampAtom = XCB_ATOM_NONE;
static xcb_atom_t incrAtom = XCB_ATOM_NONE;
static xcb_atom_t deleteAtom = XCB_ATOM_NONE;

xcb_atom_t getAtom(const char *str, xcb_connection_t *xcbConn)
{
    xcb_atom_t atom = XCB_ATOM_NONE;
    if (!strcmp(str, "PRIMARY") && primaryAtom != XCB_ATOM_NONE) {
        return primaryAtom;
    } else if (!strcmp(str, "CLIPBOARD") && clipboardAtom != XCB_ATOM_NONE) {
        return clipboardAtom;
    } else if (!strcmp(str, "UTF8_STRING") && utf8StringAtom != XCB_ATOM_NONE) {
        return utf8StringAtom;
    } else if (!strcmp(str, "TEXT") && textAtom != XCB_ATOM_NONE) {
        return textAtom;
    } else if (!strcmp(str, "text/uri-list") && uriListAtom != XCB_ATOM_NONE) {
        return uriListAtom;
    } else if (!strcmp(str, "TARGETS") && targetsAtom != XCB_ATOM_NONE) {
        return targetsAtom;
    } else if (!strcmp(str, "WL_SELECTION") && wlSelectionAtom != XCB_ATOM_NONE) {
        return wlSelectionAtom;
    } else if (!strcmp(str, "TIMESTAMP") && timestampAtom != XCB_ATOM_NONE) {
        return timestampAtom;
    } else if (!strcmp(str, "INCR") && incrAtom != XCB_ATOM_NONE) {
        return incrAtom;
    } else if (!strcmp(str, "DELETE") && deleteAtom != XCB_ATOM_NONE) {
        return deleteAtom;
    }

    if (xcbConn) {
        xcb_intern_atom_reply_t *rep = nullptr;
        uint16_t name_len = static_cast<uint16_t>(strlen(str));
        xcb_intern_atom_cookie_t cookie = xcb_intern_atom(xcbConn, 0, name_len, str);
        if (!(rep = xcb_intern_atom_reply(xcbConn, cookie, nullptr))) {
            return atom;
        }
        atom = rep->atom;
        free(rep);
    }
    // 记录atom
    if (!strcmp(str, "PRIMARY")) {
        primaryAtom = atom;
    } else if (!strcmp(str, "CLIPBOARD")) {
        clipboardAtom = atom;
    } else if (!strcmp(str, "UTF8_STRING")) {
        utf8StringAtom = atom;
    } else if (!strcmp(str, "TEXT")) {
        textAtom = atom;
    } else if (!strcmp(str, "text/uri-list")) {
        uriListAtom = atom;
    } else if (!strcmp(str, "TARGETS")) {
        targetsAtom = atom;
    } else if (!strcmp(str, "WL_SELECTION")) {
        wlSelectionAtom = atom;
    } else if (!strcmp(str, "TIMESTAMP")) {
        timestampAtom = atom;
    } else if (!strcmp(str, "INCR")) {
        incrAtom = atom;
    } else if (!strcmp(str, "DELETE")) {
        deleteAtom = atom;
    }

    return atom;
}

using namespace dfmbase;
ClipboardMonitor::ClipboardMonitor(QObject *parent)
    : QThread(parent)
{
    // 创建 XCB 连接
        connection = nullptr;
        connection = xcb_connect(nullptr, nullptr);
        if (xcb_connection_has_error(connection)) {
            for (size_t i = 0; i < 100; i++) {
                std::string displayStr(":");
                displayStr += std::to_string(i);
                // setenv("DISPLAY",displayStr.c_str(),1);
                connection = xcb_connect(displayStr.c_str(), nullptr);
                if (xcb_connection_has_error(connection) == 0) {
                    break;
                }
            }
        }

        if (xcb_connection_has_error(connection)) {
            return;
        }



    const xcb_query_extension_reply_t *queryExtension;
    queryExtension = xcb_get_extension_data(connection, &xcb_xfixes_id);
    if (!queryExtension) {
        xcb_disconnect(connection);
    }

    m_queryExtension = queryExtension;
    xcb_discard_reply(connection, xcb_xfixes_query_version(connection, 1, 0).sequence);
    screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data;
    connect(qApp, &QApplication::aboutToQuit, this, [this](){
        stop();
        this->wait(100);
    });
}

ClipboardMonitor::~ClipboardMonitor()
{
    if (connection) {
        xcb_disconnect(connection);
    }
}

void ClipboardMonitor::stop()
{
    stoped = true;
    if (connection) {
        xcb_disconnect(connection);
    }
}

void ClipboardMonitor::run()
{
    xcb_atom_t clipboardAtom =  getAtom("CLIPBOARD", connection);
    xcb_atom_t targetsAtom =  getAtom("TARGETS", connection);
    xcb_window_t window = xcb_generate_id(connection);

    const uint32_t clipboardValues[] = {XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY | XCB_EVENT_MASK_PROPERTY_CHANGE};
    xcb_create_window(connection,
                      XCB_COPY_FROM_PARENT,
                      window,
                      screen->root,
                      0, 0,
                      10, 10,
                      0,
                      XCB_WINDOW_CLASS_INPUT_OUTPUT,
                      XCB_COPY_FROM_PARENT,
                      XCB_CW_EVENT_MASK,
                      clipboardValues);

    const uint32_t mask = XCB_XFIXES_SELECTION_EVENT_MASK_SET_SELECTION_OWNER
                          | XCB_XFIXES_SELECTION_EVENT_MASK_SELECTION_WINDOW_DESTROY
                          | XCB_XFIXES_SELECTION_EVENT_MASK_SELECTION_CLIENT_CLOSE;


    // 创建监控剪贴板变化的事件请求
    xcb_xfixes_select_selection_input(connection, window, clipboardAtom, mask);


    xcb_flush(connection);

    while (true) {
        xcb_generic_event_t *event = xcb_wait_for_event(connection);
        if (stoped) {
            free(event);
            break;
        }
        if (event) {
            switch (event->response_type & (0x7f)) {
            case XCB_SELECTION_NOTIFY: {

                xcb_get_property_cookie_t cookie = xcb_get_property(connection,
                                                                    1,
                                                                    window,
                                                                    getAtom("WL_SELECTION", connection),
                                                                    XCB_GET_PROPERTY_TYPE_ANY,
                                                                    0,
                                                                    4096);
                if (stoped) {
                    free(event);
                    xcb_destroy_window(connection, window);
                    return;
                }
                auto *reply = xcb_get_property_reply(connection, cookie, nullptr);
                if (!reply) {
                    break;
                }
                if (reply->type != XCB_ATOM_ATOM) {

                    free(reply);
                    break;
                }
                if (stoped) {
                    free(reply);
                    free(event);
                    xcb_destroy_window(connection, window);
                    return;
                }
                xcb_atom_t *value = static_cast<xcb_atom_t *>(xcb_get_property_value(reply));
                QStringList mimeTypes;
                for (uint32_t i = 0; i < reply->value_len; i++) {
                    if (value[i] == XCB_ATOM_NONE) {
                        continue;
                    }
                    if (stoped) {
                        free(reply);
                        free(event);
                        xcb_destroy_window(connection, window);
                        return;
                    }
                    xcb_get_atom_name_cookie_t nameCookie = xcb_get_atom_name(connection, value[i]);
                    xcb_get_atom_name_reply_t *nameReply = xcb_get_atom_name_reply(connection, nameCookie, nullptr);

                    if (nameReply) {
                        mimeTypes.append(QString::fromUtf8(xcb_get_atom_name_name(nameReply), xcb_get_atom_name_name_length(nameReply)));
                        free(nameReply);
                    }
                }
                if (stoped) {
                    free(reply);
                    free(event);
                    xcb_destroy_window(connection, window);
                    return;
                }

                Q_EMIT clipboardChanged(mimeTypes);
                free(reply);
            }
            break;
            default:
                if (m_queryExtension && event->response_type == m_queryExtension->first_event + XCB_XFIXES_SELECTION_NOTIFY) {
                    xcb_xfixes_selection_notify_event_t *se = reinterpret_cast<xcb_xfixes_selection_notify_event_t *>(event);
                    if (se->selection == clipboardAtom) {
                        if (stoped) {
                            free(event);
                            xcb_destroy_window(connection, window);
                            return;
                        }
                        xcb_convert_selection(connection,
                                              window,
                                              clipboardAtom,
                                              targetsAtom,
                                              getAtom("WL_SELECTION", connection),
                                              se->timestamp);
                        xcb_flush(connection);
                        if (stoped) {
                            free(event);
                            xcb_destroy_window(connection, window);
                            return;
                        }
                    }
                }
                break;
            }
            if (stoped) {
                free(event);
                break;
            }
        }
        free(event);
    }
    xcb_destroy_window(connection, window);
}
