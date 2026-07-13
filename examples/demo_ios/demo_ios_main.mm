/*
 * SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda.
 * SPDX-License-Identifier: LGPL-2.1-only
 */

#include "tc_internal.h"
#include "demo_scene.h"
#import <UIKit/UIKit.h>

int tc_ios_cpu_context_create(void*, int, int, float, TcGraphicsContext**);
int tc_ios_cpu_context_resize(TcGraphicsContext*, int, int, float);
void tc_ios_cpu_context_present(TcGraphicsContext*);
void tc_ios_cpu_context_destroy(TcGraphicsContext*);
#if TC_IOS_GRAPHICS_OPENGL
int tc_ios_gl_context_create(void*, int, int, float, TcGraphicsContext**);
int tc_ios_gl_context_resize(TcGraphicsContext*, int, int, float);
void tc_ios_gl_context_present(TcGraphicsContext*);
void tc_ios_gl_context_destroy(TcGraphicsContext*);
void tc_graphics_context_present(TcGraphicsContext* context) { tc_ios_gl_context_present(context); }
#else
void tc_graphics_context_present(TcGraphicsContext* context) { tc_ios_cpu_context_present(context); }
#endif

@interface TcDemoView : UIView { @public DemoScene scene; TcGraphicsContext* graphics; TcRenderer2D* renderer; CADisplayLink* displayLink; }
@end
@implementation TcDemoView
+ (Class)layerClass {
#if TC_IOS_GRAPHICS_OPENGL
    return CAEAGLLayer.class;
#else
    return CALayer.class;
#endif
}
- (instancetype)initWithFrame:(CGRect)frame { if ((self = [super initWithFrame:frame])) { self.contentScaleFactor = UIScreen.mainScreen.scale; self.opaque = YES;
#if TC_IOS_GRAPHICS_OPENGL
    ((CAEAGLLayer*)self.layer).drawableProperties = @{kEAGLDrawablePropertyColorFormat: kEAGLColorFormatRGBA8, kEAGLDrawablePropertyRetainedBacking: @NO};
#endif
} return self; }
- (void)startRuntimeIfReady {
    if (!self.window || graphics || renderer || self.bounds.size.width <= 0.0 || self.bounds.size.height <= 0.0) return;
    int w = (int)(self.bounds.size.width * self.contentScaleFactor), h = (int)(self.bounds.size.height * self.contentScaleFactor);
#if TC_IOS_GRAPHICS_OPENGL
    int created = tc_ios_gl_context_create((__bridge void*)self, w, h, self.contentScaleFactor, &graphics);
#else
    int created = tc_ios_cpu_context_create((__bridge void*)self, w, h, self.contentScaleFactor, &graphics);
#endif
    if (created != TC_OK) { NSLog(@"tc_runtime: graphics context creation failed: %d", created); return; }
    int renderer_result = tc_renderer_create(TC_RENDERER_SKIA, graphics, &renderer);
    if (renderer_result != TC_OK) { NSLog(@"tc_runtime: Skia renderer creation failed: %d", renderer_result); return; }
    demo_scene_init(&scene, w, h); displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(frame:)]; [displayLink addToRunLoop:NSRunLoop.mainRunLoop forMode:NSRunLoopCommonModes]; }
- (void)didMoveToWindow { [super didMoveToWindow]; [self startRuntimeIfReady]; }
- (void)layoutSubviews { [super layoutSubviews]; [self startRuntimeIfReady]; if (!renderer) return; int w = (int)(self.bounds.size.width * self.contentScaleFactor), h = (int)(self.bounds.size.height * self.contentScaleFactor);
#if TC_IOS_GRAPHICS_OPENGL
    tc_ios_gl_context_resize(graphics, w, h, self.contentScaleFactor);
#else
    tc_ios_cpu_context_resize(graphics, w, h, self.contentScaleFactor);
#endif
    tc_renderer_resize(renderer, w, h, self.contentScaleFactor); TcEvent event = {.type = TC_EVENT_RESIZE}; event.data.resize.width = w; event.data.resize.height = h; event.data.resize.scale = self.contentScaleFactor; demo_scene_on_event(&scene, &event); }
- (void)frame:(CADisplayLink*)link { demo_scene_on_update(&scene, link.targetTimestamp - link.timestamp); TcCanvas2D* canvas = tc_renderer_begin_frame(renderer); demo_scene_on_draw(&scene, canvas); tc_renderer_end_frame(renderer); }
- (void)drawRect:(CGRect)rect { (void)rect;
#if !TC_IOS_GRAPHICS_OPENGL
    if (!graphics) return; CGColorSpaceRef colors = CGColorSpaceCreateDeviceRGB(); CGDataProviderRef data = CGDataProviderCreateWithData(NULL, graphics->pixels, (size_t)graphics->pitch * graphics->height, NULL); CGImageRef image = CGImageCreate(graphics->width, graphics->height, 8, 32, graphics->pitch, colors, kCGBitmapByteOrder32Big | kCGImageAlphaPremultipliedLast, data, NULL, false, kCGRenderingIntentDefault); CGContextRef context = UIGraphicsGetCurrentContext(); CGContextSaveGState(context); CGContextTranslateCTM(context, 0, CGRectGetHeight(self.bounds)); CGContextScaleCTM(context, 1, -1); CGContextDrawImage(context, CGRectMake(0, 0, CGRectGetWidth(self.bounds), CGRectGetHeight(self.bounds)), image); CGContextRestoreGState(context); CGImageRelease(image); CGDataProviderRelease(data); CGColorSpaceRelease(colors);
#endif
}
- (void)dealloc { [displayLink invalidate]; tc_renderer_destroy(renderer);
#if TC_IOS_GRAPHICS_OPENGL
    tc_ios_gl_context_destroy(graphics);
#else
    tc_ios_cpu_context_destroy(graphics);
#endif
    [super dealloc];
}
@end
@interface TcAppDelegate : UIResponder <UIApplicationDelegate> @property(nonatomic, retain) UIWindow* window; @end
@implementation TcAppDelegate
- (BOOL)application:(UIApplication*)application didFinishLaunchingWithOptions:(NSDictionary*)options { (void)application; (void)options; self.window = [[UIWindow alloc] initWithFrame:UIScreen.mainScreen.bounds]; self.window.rootViewController = [UIViewController new]; self.window.rootViewController.view = [[TcDemoView alloc] initWithFrame:self.window.bounds]; [self.window makeKeyAndVisible]; return YES; }
@end
int main(int argc, char* argv[]) { @autoreleasepool { return UIApplicationMain(argc, argv, nil, NSStringFromClass(TcAppDelegate.class)); } }
