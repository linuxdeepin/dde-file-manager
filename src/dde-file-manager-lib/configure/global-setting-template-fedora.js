{
    "groups": [
        {
            "key": "base",
            "name": qsTranslate("GenerateSettingTranslate", "Basic"),
            "groups": [
                {
                    "key": "open_action",
                    "name": qsTranslate("GenerateSettingTranslate", "Open behavior"),
                    "options": [
                        {
                            "key": "allways_open_on_new_window",
                            "type": "checkbox",
                            "text": qsTranslate("GenerateSettingTranslate", "Always open folder in new window"),
                            "default": false
                        },
                        {
                            "key": "open_file_action",
                            "name": qsTranslate("GenerateSettingTranslate", "Open file:"),
                            "type": "combobox",
                            "items": [
                                qsTranslate("GenerateSettingTranslate", "Click"),
                                qsTranslate("GenerateSettingTranslate", "Double click")
                                ],
                            "default": 1
                        }
                    ]
                },
                {
                    "key": "new_tab_windows",
                    "name": qsTranslate("GenerateSettingTranslate", "New window and tab"),
                    "options": [
                        {
                            "key": "default_window_path",
                            "name": qsTranslate("GenerateSettingTranslate", "Open from default window:"),
                            "type": "combobox",
                            "items": {
                                "values": [
                                    qsTranslate("GenerateSettingTranslate", "Computer"),
                                    qsTranslate("GenerateSettingTranslate", "Home"),
                                    qsTranslate("GenerateSettingTranslate", "Desktop"),
                                    qsTranslate("GenerateSettingTranslate", "Videos"),
                                    qsTranslate("GenerateSettingTranslate", "Music"),
                                    qsTranslate("GenerateSettingTranslate", "Pictures"),
                                    qsTranslate("GenerateSettingTranslate", "Documents"),
                                    qsTranslate("GenerateSettingTranslate", "Downloads")
                                ],
                                "keys": [
                                    "computer:///",
                                    "standard://home",
                                    "standard://desktop",
                                    "standard://videos",
                                    "standard://music",
                                    "standard://pictures",
                                    "standard://documents",
                                    "standard://downloads"
                                ]
                            },
                            "default": "computer:///"
                        },
                        {
                            "key": "new_tab_path",
                            "name": qsTranslate("GenerateSettingTranslate", "Open in new tab:"),
                            "type": "combobox",
                            "items": {
                                "values": [
                                    qsTranslate("GenerateSettingTranslate", "Current Directory"),
                                    qsTranslate("GenerateSettingTranslate", "Computer"),
                                    qsTranslate("GenerateSettingTranslate", "Home"),
                                    qsTranslate("GenerateSettingTranslate", "Desktop"),
                                    qsTranslate("GenerateSettingTranslate", "Videos"),
                                    qsTranslate("GenerateSettingTranslate", "Music"),
                                    qsTranslate("GenerateSettingTranslate", "Pictures"),
                                    qsTranslate("GenerateSettingTranslate", "Documents"),
                                    qsTranslate("GenerateSettingTranslate", "Downloads")
                                ],
                                "keys": [
                                    "",
                                    "computer:///",
                                    "standard://home",
                                    "standard://desktop",
                                    "standard://videos",
                                    "standard://music",
                                    "standard://pictures",
                                    "standard://documents",
                                    "standard://downloads"
                                ]
                            },
                            "default": ""
                        }
                    ]
                },
                {
                    "key": "default_view",
                    "name": qsTranslate("GenerateSettingTranslate", "View"),
                    "options": [
                        {
                            "key": "icon_size",
                            "name": qsTranslate("GenerateSettingTranslate", "Default size:"),
                            "type": "combobox",
                            "items": [
                                qsTranslate("GenerateSettingTranslate", "Extra small"),
                                qsTranslate("GenerateSettingTranslate", "Small"),
                                qsTranslate("GenerateSettingTranslate", "Medium"),
                                qsTranslate("GenerateSettingTranslate", "Large"),
                                qsTranslate("GenerateSettingTranslate", "Extra large")
                            ],
                            "default": 1
                        },
                        {
                            "key": "view_mode",
                            "name": qsTranslate("GenerateSettingTranslate", "Default view:"),
                            "type": "combobox",
                            "items": {
                                "values": [
                                    qsTranslate("GenerateSettingTranslate", "Icon"),
                                    qsTranslate("GenerateSettingTranslate", "List")
                                ],
                                "keys": [
                                    1, 2
                                ]
                            },
                            "default": 1
                        }
                    ]
                },
                {
                    "key": "hidden_files",
                    "name": qsTranslate("GenerateSettingTranslate", "Hidden files"),
                    "options": [
                        {
                            "key": "show_hidden",
                            "text": qsTranslate("GenerateSettingTranslate", "Show hidden files"),
                            "type": "checkbox",
                            "default": false
                        },
                        {
                            "key": "hide_suffix",
                            "text": qsTranslate("GenerateSettingTranslate", "Hide file extension when rename"),
                            "type": "checkbox",
                            "default": true
                        },
                        {
                            "key": "show_recent",
                            "text": qsTranslate("GenerateSettingTranslate", "Display recent file entry in left panel"),
                            "type": "checkbox",
                            "default": true
                        }
                    ]
                }

            ]
        },
        {
            "key": "advance",
            "name": qsTranslate("GenerateSettingTranslate", "Advanced"),
            "groups": [
                {
                  "key": "index",
                  "name": qsTranslate("GenerateSettingTranslate", "Index"),
                  "options": [
                    {
                            "key": "index_internal",
                            "text": qsTranslate("GenerateSettingTranslate", "Auto index internal disk"),
                            "type": "checkbox",
                            "default": true
                    },
                    {
                            "key": "index_external",
                            "text": qsTranslate("GenerateSettingTranslate", "Index external storage device after connected to computer"),
                            "type": "checkbox",
                            "default": false
                    },
                    {
                            "key": "index_search",
                            "text": qsTranslate("GenerateSettingTranslate", "Full-Text search"),
                            "type": "checkbox",
                            "default": false
                    }
                    ]
                },
                {
                    "key": "search",
                    "name": "",
                    "options": [
                        {
                            "key": "show_hidden",
                            "text": qsTranslate("GenerateSettingTranslate", "Show hidden files in search results"),
                            "type": "checkbox",
                            "default": false,
                            "hide": true
                        }
                    ]
                },
                {
                    "key": "preview",
                    "name": qsTranslate("GenerateSettingTranslate", "Preview"),
                    "options": [
                        {
                            "key": "compress_file_preview",
                            "text": qsTranslate("GenerateSettingTranslate", "Compress file preview"),
                            "type": "checkbox",
                            "default": false,
                            "hide": true
                        },
                        {
                            "key": "text_file_preview",
                            "text": qsTranslate("GenerateSettingTranslate", "Text preview"),
                            "type": "checkbox",
                            "default": true
                        },
                        {
                            "key": "document_file_preview",
                            "text": qsTranslate("GenerateSettingTranslate", "Document preview"),
                            "type": "checkbox",
                            "default": true
                        },
                        {
                            "key": "image_file_preview",
                            "text": qsTranslate("GenerateSettingTranslate", "Image preview"),
                            "type": "checkbox",
                            "default": true
                        },
                        {
                            "key": "video_file_preview",
                            "text": qsTranslate("GenerateSettingTranslate", "Video preview"),
                            "type": "checkbox",
                            "default": false,
                            "hide": true
                        }
                    ]
                },
                {
                    "key": "mount",
                    "name": qsTranslate("GenerateSettingTranslate", "Mount"),
                    "options": [
                        {
                            "key": "auto_mount",
                            "text": qsTranslate("GenerateSettingTranslate", "Auto mount"),
                            "type": "mountCheckBox",
                            "default": true
                        },
                        {
                            "key": "auto_mount_and_open",
                            "text": qsTranslate("GenerateSettingTranslate", "Open after auto mount"),
                            "type": "openCheckBox",
                            "default": false
                        },
                        {
                            "key": "mtp_show_bottom_info",
                            "text": qsTranslate("GenerateSettingTranslate", "Show item counts and sizes in the path of mounted MTP devices"),
                            "type": "checkbox",
                            "default": false
                        },
                        {
                            "key": "always_show_offline_remote_connection",
                            "text": qsTranslate("GenerateSettingTranslate", "Keep showing the mounted Samba shares"),
                            "type": "checkbox",
                            "default": true
                        }
                    ]
                },
                {
                    "key": "dialog",
                    "name": qsTranslate("GenerateSettingTranslate", "Dialog"),
                    "options": [
                        {
                            "key": "default_chooser_dialog",
                            "text": qsTranslate("GenerateSettingTranslate", "Use the file chooser dialog of File Manager"),
                            "type": "checkbox",
                            "default": true
                        },
                        {
                            "key": "delete_confirmation_dialog",
                            "text": qsTranslate("GenerateSettingTranslate", "Ask for my confirmation when deleting files"),
                            "type": "checkbox",
                            "default": false
                        }
                    ]
                },
                {
                    "key": "other",
                    "name": qsTranslate("GenerateSettingTranslate", "Other"),
                    "options": [
                        {
                            "key": "hide_system_partition",
                            "text": qsTranslate("GenerateSettingTranslate", "Hide system disk"),
                            "type": "checkbox",
                            "default": false
                        },
                        {
                            "key": "show_crumbbar_clickable_area",
                            "text": "Show crumb bar clickable area",
                            "type": "checkbox",
                            "default": false,
                            "hide": true
                        },
                        {
                            "key": "show_filesystemtag_on_diskicon",
                            "text": qsTranslate("GenerateSettingTranslate", "Show file system on disk icon"),
                            "type": "checkbox",
                            "default": false
                        }
                    ]
                }
            ]
        }

      ]
}
