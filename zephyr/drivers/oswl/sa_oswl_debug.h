/*
 * Copyright 2021 Rockchip Electronics Co. LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __BTW_LOG_H__
#define __BTW_LOG_H__

#include <stdio.h>
#include <stdlib.h>

#define   LOG_MAX_LEN        256
#define   WIN_PRINT_ON       1
#define   WIN_PRINT_OFF      0

#endif

#ifdef __cplusplus
extern "C" {
#endif
void sa_oswl_dbg(const char *tag, const char *fmt,const char *fname, ...);
void sa_oswl_err(const char *tag, const char *fmt,const char *fname, ...);
void sa_oswl_create(const char* name,int flag);
void sa_oswl_close(void);

#ifdef __cplusplus
}
#endif

