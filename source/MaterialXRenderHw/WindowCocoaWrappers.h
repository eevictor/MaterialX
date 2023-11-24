//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#ifndef MATERIALX_WINDOWCOCOAWRAPPERS_H
#define MATERIALX_WINDOWCOCOAWRAPPERS_H

#if defined(__APPLE__)

/// Wrappers for calling into Objective-C Cocoa routines on Mac for Windowing
#ifdef __cplusplus
extern "C" {
#endif

void* NSUtilGetView(void* pWindow);
void* NSUtilCreateWindow(unsigned int width, unsigned int height, const char* title, bool batchMode);
void NSUtilShowWindow(void* pWindow);
void NSUtilHideWindow(void* pWindow);
void NSUtilSetFocus(void* pWindow);
void NSUtilDisposeWindow(void* pWindow);

#ifdef TARGET_OS_IOS
void* GetContext(void* pView);
void* GetCurrentContext();
void MakeCurrent(void* pContext);
void* CreateContext(void* pView, void* sharedWithContext = 0);
void DestroyCurrentContext(void** pContext);
#endif

#ifdef __cplusplus
}
#endif

#endif

#endif
