{
    "groups": [
        {
            "key": "base",
            "name": "Basic",
            "groups": [
                {
                    "key": "open_action",
                    "name": "Open behavior",
                    "options": [
                        {
                            "key": "allways_open_on_new_window",
                            "type": "checkbox",
                            "text": "Always open folder in new window",
                            "default": false
                        },
                        {
                            "key": "open_file_action",
                            "name": "Open file:",
                            "type": "combobox",
                            "items": [
                                "Click",
                                "Double click"
                            ],
                            "default": 1
                        }
                    ]
                },
                {
                    "key": "new_tab_windows",
                    "name": "New window and tab",
                    "options": [
                        {
                            "key": "default_window_path",
                            "name": "Open from default window:",
                            "type": "combobox",
                            "items": {
                                "values": [
                                    "Computer",
                                    "Home",
                                    "Desktop",
                                    "Videos",
                                    "Music",
                                    "Pictures",
                                    "Documents",
                                    "Downloads"
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
                            "name": "Open in new tab:",
                            "type": "combobox",
                            "items": {
                                "values": [
                                    "Current Directory",
                                    "Computer",
                                    "Home",
                                    "Desktop",
                                    "Videos",
                                    "Music",
                                    "Pictures",
                                    "Documents",
                                    "Downloads"
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
                    "name": "View",
                    "options": [
                        {
                            "key": "icon_size",
                            "name": "Default size:",
                            "type": "combobox",
                            "items": [
                                "Extra small",
                                "Small",
                                "Medium",
                                "Large",
                                "Extra large"
                            ],
                            "default": 1
                        },
                        {
                            "key": "view_mode",
                            "name": "Default view:",
                            "type": "combobox",
                            "items": {
                                "values": [
                                    "Icon",
                                    "List"
                                ],
                                "keys": [
                                    1, 2
                                ]
                            },
                            "default": 1
                        },
                        {
                            "key": "mixed_sort",
                            "text": "Mix sorting of files and folders",
                            "type": "checkbox",
                            "default": false
                        }
                    ]
                },
                {
                    "key": "hidden_files",
                    "name": "Hidden files",
                    "options": [
                        {
                            "key": "show_hidden",
                            "text": "Show hidden files",
                            "type": "checkbox",
                            "default": false
                        },
                        {
                            "key": "show_suffix",
                            "text": "Show file extensions",
                            "type": "checkbox",
                            "default": true
                        }

                    ]
                }

            ]
        },
        {
            "key": "advance",
            "name": "Advanced",
            "groups": [
                {
                  "key": "index",
                  "name": "Index",
                  "options": [
                    {
                            "key": "index_internal",
                            "text": "Auto index internal disk",
                            "type": "checkbox",
                            "default": true
                    },
                    {
                            "key": "index_external",
                            "text": "Index external storage device after connected to computer",
                            "type": "checkbox",
                            "default": false
                    },
                    {
                            "key": "index_search",
                            "text": "Full-Text search",
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
                            "text": "Show hidden files in search results",
                            "type": "checkbox",
                            "default": false,
                            "hide": true
                        }
                    ]
                },
                {
                    "key": "preview",
                    "name": "Preview",
                    "options": [
                        {
                            "key": "compress_file_preview",
                            "text": "Compressed file preview",
                            "type": "checkbox",
                            "default": false
                        },
                        {
                            "key": "text_file_preview",
                            "text": "Text preview",
                            "type": "checkbox",
                            "default": true
                        },
                        {
                            "key": "document_file_preview",
                            "text": "Document preview",
                            "type": "checkbox",
                            "default": true
                        },
                        {
                            "key": "image_file_preview",
                            "text": "Image preview",
                            "type": "checkbox",
                            "default": true
                        },
                        {
                            "key": "video_file_preview",
                            "text": "Video preview",
                            "type": "checkbox",
                            "default": true
                        },
                        {
                            "key": "audio_file_preview",
                            "text": "Music preview",
                            "type": "checkbox",
                            "default": true
                        },
                        {
                            "key": "remote_env_file_preview",
                            "text": "The remote environment shows thumbnail previews",
                            "message":"Turning on the thumbnail preview may cause the remote directory to load slowly or the operation to freeze",
                            "type": "checkBoxWithMessage",
                            "default": false
                        }
                    ]
                },
                {
                    "key": "mount",
                    "name": "Mount",
                    "options": [
                        {
                            "key": "auto_mount",
                            "text": "Auto mount",
                            "type": "mountCheckBox",
                            "default": true
                        },
                        {
                            "key": "auto_mount_and_open",
                            "text": "Open after auto mount",
                            "type": "openCheckBox",
                            "default": false
                        },
                        {
                            "key": "always_show_offline_remote_connection",
                            "text": "Keep showing the mounted Samba shares",
                            "type": "checkbox",
                            "default": true
                        },
                        {
                            "key": "merge_the_entries_of_samba_shared_folders",
                            "text": "Merge the entries of Samba shared folders",
                            "message": "Switching the entry display may lead to failed mounting",
                            "type": "checkBoxWithMessage",
                            "default": true
                        }
                    ]
                },
                {
                    "key": "dialog",
                    "name": "Dialog",
                    "options": [
                        {
                            "key": "default_chooser_dialog",
                            "text": "Use the file chooser dialog of File Manager",
                            "type": "checkbox",
                            "default": true
                        },
                        {
                            "key": "delete_confirmation_dialog",
                            "text": "Ask for my confirmation when deleting files",
                            "type": "checkbox",
                            "default": false
                        }
                    ]
                },
                {
                    "key": "items_in_sidebar",
                    "name": "Items on sidebar pane",
                    "options": [
                        {
                            "key": "quick_access_splitter",
                            "name": "Quick access",
                            "type": "splitter"
                        },
                        {
                            "key": "recent",
                            "text": "Recent",
                            "type": "checkbox",
                            "default": true
                        },
                        {
                            "key": "home",
                            "text": "Home",
                            "type": "checkbox",
                            "default": true
                        },
                        {
                            "key": "desktop",
                            "text": "Desktop",
                            "type": "checkbox",
                            "default": true
                        },
                        {
                            "key": "videos",
                            "text": "Videos",
                            "type": "checkbox",
                            "default": true
                        },
                        {
                            "key": "music",
                            "text": "Music",
                            "type": "checkbox",
                            "default": true
                        },
                        {
                            "key": "pictures",
                            "text": "Pictures",
                            "type": "checkbox",
                            "default": true
                        },
                        {
                            "key": "documents",
                            "text": "Documents",
                            "type": "checkbox",
                            "default": true
                        },
                        {
                            "key": "downloads",
                            "text": "Downloads",
                            "type": "checkbox",
                            "default": true
                        },
                        {
                            "key": "trash",
                            "text": "Trash",
                            "type": "checkbox",
                            "default": true
                        },
                        {
                            "key": "partitions_splitter",
                            "name": "Partitions",
                            "type": "splitter"
                        },
                        {
                            "key": "computer",
                            "text": "Computer",
                            "type": "checkbox",
                            "default": true
                        },
                        {
                            "key": "vault",
                            "text": "Vault",
                            "type": "checkbox",
                            "default": true
                        },
                        {
                            "key": "builtin",
                            "text": "Built-in disks",
                            "type": "checkbox",
                            "default": true
                        },
                        {
                            "key": "loop",
                            "text": "Loop partitions",
                            "type": "checkbox",
                            "default": false
                        },
                        {
                            "key": "other_disks",
                            "text": "Mounted partitions and discs",
                            "type": "checkbox",
                            "default": true
                        },
                        {
                            "key": "network_splitters",
                            "name": "Network",
                            "type": "splitter"
                        },
                        {
                            "key": "computers_in_lan",
                            "text": "Computers in LAN",
                            "type": "checkbox",
                            "default": true
                        },
                        {
                            "key": "my_shares",
                            "text": "My shares",
                            "type": "checkbox",
                            "default": true
                        },
                        {
                            "key": "mounted_share_dirs",
                            "text": "Mounted sharing folders",
                            "type": "checkbox",
                            "default": true
                        },
                        {
                            "key": "tag_splitter",
                            "name": "Tag",
                            "type": "splitter"
                        },
                        {
                            "key": "tags",
                            "text": "Added tags",
                            "type": "checkbox",
                            "default": true
                        }
                    ]
                },
                {
                    "key": "other",
                    "name": "Other",
                    "options": [
                        {
                            "key": "hide_builtin_partition",
                            "text": "Hide built-in disks on the Computer page",
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
                            "key": "hide_loop_partitions",
                            "text": "Hide loop partitions on the Computer page",
                            "type": "checkbox",
                            "default": true
                        },
                        {
                            "key": "show_filesystemtag_on_diskicon",
                            "text": "Show file system on disk icon",
                            "type": "checkbox",
                            "default": false
                        },
                        {
                            "key": "extend_file_name",
                            "text": "Extend filename characters",
                            "message": "After opening, the file name will be expanded to support 255 Chinese characters, which may cause slow loading of the file directory or freeze the operation",
                            "type": "checkBoxWithMessage",
                            "default": false,
                            "hide": true
                        }
                    ]
                }
            ]
        }

    ]
}
