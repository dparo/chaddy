// SPDX-FileCopyrightText: 2023 Davide Paro
//
// SPDX-License-Identifier: MIT

#pragma once

#if __cplusplus
extern "C" {
#endif

extern const char *const PROJECT_NAME;
extern const char *const PROJECT_DESCRIPTION;
extern const char *const PROJECT_VERSION;
extern const char *const PROJECT_VERSION_MAJOR;
extern const char *const PROJECT_VERSION_MINOR;
extern const char *const PROJECT_VERSION_PATCH;

extern const char *const BUILD_TYPE;
extern const char *const C_COMPILER_ID;
extern const char *const C_COMPILER_ABI;
extern const char *const C_COMPILER_VERSION;
extern const char *const GIT_SHA1;
extern const char *const GIT_DATE;
extern const char *const GIT_COMMIT_SUBJECT;

#if __cplusplus
}
#endif
