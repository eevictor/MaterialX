//
// Copyright Contributors to the MaterialX Project
// SPDX-License-Identifier: Apache-2.0
//

#if defined (__APPLE__)

#ifdef TARGET_OS_IOS

#import <GLKit/GLKit.h>
#import <MaterialXRenderHw/WindowCocoaWrappers.h>
//Only make using this *.m for objective C code.  Methods using iOS UIKit instead of Cocoa.
void* NSUtilGetView(void* view)
{
	return (void*)view;
}

// for iOS target, this is actually creating a GLES backed view.
void* NSUtilCreateWindow(unsigned int width, unsigned int height, const char* title, bool batchMode)
{
	// In batch mode, batchMode ignored

	// Create local autorelease pool for any objects that need to be autoreleased.
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	EAGLContext * supportedContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
	
	if (!supportedContext) {
		NSLog(@"MaterialXRenderHw Failed to create ES context");
	} else {
		NSLog(@"MaterialXRenderHw Construct ES context : %lu", (unsigned long)supportedContext.API);
	}
	
	GLKView * view = [[GLKView alloc] initWithFrame: CGRectMake(0, 0, width, height) context: supportedContext];
	view.drawableColorFormat = GLKViewDrawableColorFormatRGBA8888;
	view.drawableDepthFormat = GLKViewDrawableDepthFormat24;
	view.drawableStencilFormat = GLKViewDrawableStencilFormat8;
	// this sequence of settings makes the glkview transparent
	view.opaque = NO;
	view.backgroundColor = [UIColor clearColor];
	CAEAGLLayer *eaglLayer = (CAEAGLLayer *)view.layer;
	eaglLayer.opaque = NO;

	// Free up memory
	[pool release];
	
	return (void*)view;
}
void* GetContext(void* pView) {
	GLKView * view = (GLKView*)pView;
	return [view context];
}
void* GetCurrentContext()
{
	return [EAGLContext currentContext];
}
void* CreateContext(void* pView, void* sharedWithContext)
{
	GLKView * view = (GLKView*)pView;
	EAGLContext * supportedContext = 0;
	
	if ( sharedWithContext != 0 ) {
		EAGLSharegroup * group = [((EAGLContext*)sharedWithContext) sharegroup];
		supportedContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3 sharegroup:group];
	} else {
		supportedContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
	}
	if (!supportedContext) {
		NSLog(@"MaterialXRenderHw Failed to create ES context");
	} else {
		NSLog(@"MaterialXRenderHw CreateContext ES context : %lu", (unsigned long)supportedContext.API);
	}
	if ( supportedContext ) {
		[view setContext: supportedContext];
		
		view.drawableColorFormat = GLKViewDrawableColorFormatRGBA8888;
		view.drawableDepthFormat = GLKViewDrawableDepthFormat24;
		view.drawableStencilFormat = GLKViewDrawableStencilFormat8;
		// this sequence of settings makes the glkview transparent
		view.opaque = NO;
		view.backgroundColor = [UIColor clearColor];
		CAEAGLLayer *eaglLayer = (CAEAGLLayer *)view.layer;
		eaglLayer.opaque = NO;
		
		return supportedContext;
	}
	return nullptr;
}
void MakeCurrent(void* pContext) {
	EAGLContext * c = (EAGLContext*)pContext;
	[EAGLContext setCurrentContext:c];
}
void DestroyCurrentContext(void** pContext)
{
	//EAGLContext * c = (EAGLContext*)*pContext;
	[EAGLContext setCurrentContext:nil];
}
void NSUtilShowWindow(void* pWindow)
{
}

void NSUtilHideWindow(void* pWindow)
{
}

void NSUtilSetFocus(void* pWindow)
{
}

void NSUtilDisposeWindow(void* pView)
{
	// Create local autorelease pool for any objects that need to be autoreleased.
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	UIView* view = (UIView*)pView;
	[view removeFromSuperview];
	view = nil;

	// Free up memory
	[pool release];
}

#else

#import <Cocoa/Cocoa.h>
#import <AppKit/NSApplication.h>
#import <MaterialXRenderHw/WindowCocoaWrappers.h>

void* NSUtilGetView(void* pWindow)
{
    NSWindow* window = (NSWindow*)pWindow;
	NSView* view =  [window contentView];
	return (void*)view;
}

void* NSUtilCreateWindow(unsigned int width, unsigned int height, const char* title, bool batchMode)
{
	// In batch mode, ensure that Cocoa is initialized
	if (batchMode)
	{
		NSApplicationLoad();
	}

	// Create local autorelease pool for any objects that need to be autoreleased.
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	NSWindow *window = [[NSWindow alloc] initWithContentRect:NSMakeRect(0, 0, width, height)
		styleMask:NSWindowStyleMaskTitled  | NSWindowStyleMaskClosable	| NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable
		backing:NSBackingStoreBuffered defer:NO];
	NSString *string = [NSString stringWithUTF8String:title];

	[window setTitle:string];
	[window setAlphaValue:0.0];

	// Free up memory
	[pool release];

	return (void*)window;
}

void NSUtilShowWindow(void* pWindow)
{
    NSWindow* window = (NSWindow*) pWindow;
	[window orderFront:window];
}

void NSUtilHideWindow(void* pWindow)
{
    NSWindow* window = (NSWindow*)pWindow;
	[window orderOut:window];
}

void NSUtilSetFocus(void* pWindow)
{
    NSWindow* window = (NSWindow*)pWindow;
	[window makeKeyAndOrderFront:window];
}

void NSUtilDisposeWindow(void* pWindow)
{
	// Create local autorelease pool for any objects that need to be autoreleased.
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

	NSWindow* window = (NSWindow*)pWindow;
    [window close];

	// Free up memory
	[pool release];
}

#endif

#endif
