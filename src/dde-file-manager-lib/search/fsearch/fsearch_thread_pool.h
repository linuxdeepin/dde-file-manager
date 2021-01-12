/**
 * Copyright (C) 2017 Uniontech Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <glib.h>

typedef struct _FsearchThreadPool FsearchThreadPool;
typedef GThreadFunc ThreadFunc;

typedef enum {
    THREAD_IDLE,
    THREAD_BUSY,
    THREAD_FINISHED
} FsearchThreadStatus;

FsearchThreadPool *
fsearch_thread_pool_init (void);

void
fsearch_thread_pool_free (FsearchThreadPool *pool);

GList *
fsearch_thread_pool_get_threads (FsearchThreadPool *pool);

uint32_t
fsearch_thread_pool_get_num_threads (FsearchThreadPool *pool);

bool
fsearch_thread_pool_push_data (FsearchThreadPool *pool,
                               GList *thread,
                               ThreadFunc thread_func,
                               gpointer thread_data);

bool
fsearch_thread_pool_wait_for_thread (FsearchThreadPool *pool, GList *thread);

bool
fsearch_thread_pool_task_is_busy (FsearchThreadPool *pool, GList *thread);

bool
fsearch_thread_pool_task_is_idle (FsearchThreadPool *pool, GList *thread);

gpointer
fsearch_thread_pool_get_data (FsearchThreadPool *pool, GList *thread);

bool
fsearch_thread_pool_set_task_finished (FsearchThreadPool *pool, GList *thread);
