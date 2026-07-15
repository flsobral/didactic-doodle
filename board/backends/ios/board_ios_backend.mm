/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#include "../../src/board_internal.h"
#include <board/board_ios.h>
#import <UIKit/UIKit.h>
#if BOARD_BUILD_IOS_OPENGL
#import <QuartzCore/CAEAGLLayer.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES3/gl.h>
#endif
#if BOARD_BUILD_IOS_METAL
#import <QuartzCore/CAMetalLayer.h>
#endif

typedef struct BoardIosState { BoardBackend *backend; void *view;
#if BOARD_BUILD_IOS_OPENGL
    void *active_gl;
#endif
} BoardIosState;
typedef struct BoardIosNativeViewSlot { void *container; } BoardIosNativeViewSlot;
#if BOARD_BUILD_IOS_OPENGL
typedef struct BoardIosOpenGLContext { void *eagl; GLuint framebuffer, colorbuffer; uint32_t width, height; float scale; } BoardIosOpenGLContext;
static BoardResult board_ios_gl_rebuild(BoardIosState *state, BoardIosOpenGLContext *gl, uint32_t width, uint32_t height, float scale);
#endif

static uint64_t board_ios_timestamp(CFTimeInterval seconds) {
    return (uint64_t)(seconds * 1000000000.0);
}

static BoardResult board_ios_resize(BoardBackend *backend, uint32_t width, uint32_t height, float scale) {
    uint8_t *pixels;
    BoardEvent event = {sizeof(BoardEvent), BOARD_ABI_VERSION, BOARD_EVENT_RESIZE, 0, {{0}}};
    if (!backend || !width || !height) return BOARD_ERROR_INVALID_ARGUMENT;
    if (backend->width == width && backend->height == height && backend->scale == scale) return BOARD_OK;
    pixels = (uint8_t *)realloc(backend->pixels, (size_t)width * height * 4u);
    if (!pixels) return BOARD_ERROR_OUT_OF_MEMORY;
    backend->pixels = pixels;
    backend->width = width;
    backend->height = height;
    backend->stride = width * 4u;
    backend->scale = scale > 0 ? scale : 1.0f;
#if BOARD_BUILD_IOS_OPENGL
    { BoardIosState *state = (BoardIosState *)backend->implementation; if (state && state->active_gl) { BoardResult result = board_ios_gl_rebuild(state, (BoardIosOpenGLContext *)state->active_gl, width, height, backend->scale); if (result != BOARD_OK) return result; } }
#endif
#if BOARD_BUILD_IOS_METAL
    backend->surface.metal.width = width;
    backend->surface.metal.height = height;
    backend->surface.metal.scale = backend->scale;
#endif
    event.timestamp_ns = board_ios_timestamp(CACurrentMediaTime());
    event.data.resize.width = width;
    event.data.resize.height = height;
    event.data.resize.scale = backend->scale;
    return board_backend_post_event(backend, &event);
}

static BoardResult board_ios_map(void *data, void **pixels, uint32_t *width, uint32_t *height, uint32_t *stride, BoardPixelFormat *format, float *scale) {
    BoardIosState *state = (BoardIosState *)data;
    BoardBackend *backend = state ? state->backend : NULL;
    if (!backend || !pixels || !width || !height || !stride || !format || !scale) return BOARD_ERROR_INVALID_ARGUMENT;
    *pixels = backend->pixels;
    *width = backend->width;
    *height = backend->height;
    *stride = backend->stride;
    *format = BOARD_PIXEL_FORMAT_RGBA8888;
    *scale = backend->scale;
    return BOARD_OK;
}

@interface BoardIosOverlayView : UIView
@end

@implementation BoardIosOverlayView
- (UIView *)hitTest:(CGPoint)point withEvent:(UIEvent *)event {
    UIView *hit = [super hitTest:point withEvent:event];
    return hit == self ? nil : hit;
}
@end

@interface BoardIosView : UIView {
@public
    BoardBackend *_board_backend;
    CADisplayLink *_display_link;
    BoardIosOverlayView *_native_overlay;
}
- (instancetype)initWithBackend:(BoardBackend *)backend frame:(CGRect)frame;
- (void)startFrames;
- (void)stopFrames;
@end

@implementation BoardIosView
+ (Class)layerClass {
#if BOARD_BUILD_IOS_OPENGL
    return CAEAGLLayer.class;
#elif BOARD_BUILD_IOS_METAL
    return CAMetalLayer.class;
#else
    return CALayer.class;
#endif
}
- (instancetype)initWithBackend:(BoardBackend *)backend frame:(CGRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        _board_backend = backend;
        self.opaque = YES;
        self.contentScaleFactor = UIScreen.mainScreen.scale;
        self.multipleTouchEnabled = YES;
        _native_overlay = [[BoardIosOverlayView alloc] initWithFrame:self.bounds];
        _native_overlay.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
        [self addSubview:_native_overlay];
#if BOARD_BUILD_IOS_OPENGL
        ((CAEAGLLayer *)self.layer).opaque = YES;
        ((CAEAGLLayer *)self.layer).drawableProperties = @{kEAGLDrawablePropertyColorFormat: kEAGLColorFormatRGBA8, kEAGLDrawablePropertyRetainedBacking: @NO};
#elif BOARD_BUILD_IOS_METAL
        ((CAMetalLayer *)self.layer).opaque = YES;
        ((CAMetalLayer *)self.layer).contentsScale = self.contentScaleFactor;
#endif
    }
    return self;
}
- (void)startFrames {
    if (_display_link) return;
    _display_link = [CADisplayLink displayLinkWithTarget:self selector:@selector(onFrame:)];
    [_display_link addToRunLoop:NSRunLoop.mainRunLoop forMode:NSRunLoopCommonModes];
}
- (void)stopFrames {
    [_display_link invalidate];
    _display_link = nil;
}
- (void)layoutSubviews {
    [super layoutSubviews];
    uint32_t width = (uint32_t)(self.bounds.size.width * self.contentScaleFactor);
    uint32_t height = (uint32_t)(self.bounds.size.height * self.contentScaleFactor);
    board_ios_resize(_board_backend, width, height, self.contentScaleFactor);
}
- (void)onFrame:(CADisplayLink *)link {
    if (!_board_backend || !_board_backend->event_sink) return;
    board_backend_dispatch_events(_board_backend, _board_backend->event_sink, _board_backend->event_data);
    board_scheduler_request_frame(&_board_backend->scheduler);
    board_backend_step(_board_backend, board_ios_timestamp(link.timestamp));
}
- (void)emitTouch:(UITouch *)touch type:(BoardEventType)type {
    CGPoint point = [touch locationInView:self];
    BoardEvent event = {sizeof(BoardEvent), BOARD_ABI_VERSION, type, board_ios_timestamp(CACurrentMediaTime()), {{0}}};
    event.data.pointer.x = point.x * self.contentScaleFactor;
    event.data.pointer.y = point.y * self.contentScaleFactor;
    event.data.pointer.button = BOARD_POINTER_BUTTON_LEFT;
    event.data.pointer.pointer_id = (uint32_t)(uintptr_t)touch;
    board_backend_post_event(_board_backend, &event);
}
- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event { (void)event; for (UITouch *touch in touches) [self emitTouch:touch type:BOARD_EVENT_POINTER_DOWN]; }
- (void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event { (void)event; for (UITouch *touch in touches) [self emitTouch:touch type:BOARD_EVENT_POINTER_MOVE]; }
- (void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event { (void)event; for (UITouch *touch in touches) [self emitTouch:touch type:BOARD_EVENT_POINTER_UP]; }
- (void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event { (void)event; for (UITouch *touch in touches) [self emitTouch:touch type:BOARD_EVENT_POINTER_CANCEL]; }
- (void)drawRect:(CGRect)rect {
#if BOARD_BUILD_IOS_OPENGL || BOARD_BUILD_IOS_METAL
    (void)rect;
    return;
#else
    BoardBackend *backend = _board_backend;
    CGColorSpaceRef color_space;
    CGDataProviderRef provider;
    CGImageRef image;
    CGContextRef context;
    (void)rect;
    if (!backend || !backend->pixels) return;
    color_space = CGColorSpaceCreateDeviceRGB();
    provider = CGDataProviderCreateWithData(NULL, backend->pixels, (size_t)backend->stride * backend->height, NULL);
    image = CGImageCreate(backend->width, backend->height, 8, 32, backend->stride, color_space, kCGBitmapByteOrder32Big | kCGImageAlphaPremultipliedLast, provider, NULL, false, kCGRenderingIntentDefault);
    context = UIGraphicsGetCurrentContext();
    CGContextSaveGState(context);
    CGContextTranslateCTM(context, 0, CGRectGetHeight(self.bounds));
    CGContextScaleCTM(context, 1, -1);
    CGContextDrawImage(context, self.bounds, image);
    CGContextRestoreGState(context);
    CGImageRelease(image);
    CGDataProviderRelease(provider);
    CGColorSpaceRelease(color_space);
#endif
}
@end

static BoardIosNativeViewSlot *board_ios_slot(BoardNativeViewSlot *slot) { return slot ? (BoardIosNativeViewSlot *)slot->implementation : NULL; }

static BoardResult board_ios_slot_apply(BoardNativeViewSlot *slot) {
    BoardIosNativeViewSlot *native_slot = board_ios_slot(slot);
    UIView *container;
    UIView *view;
    CGRect frame;
    if (!slot || !slot->backend || !native_slot || !native_slot->container || slot->z_order != BOARD_NATIVE_VIEW_ABOVE_RENDERER || ![NSThread isMainThread]) return BOARD_ERROR_UNAVAILABLE;
    container = (__bridge UIView *)native_slot->container;
    view = container.subviews.firstObject;
    if (!view) return BOARD_ERROR_PLATFORM;
    frame = CGRectMake(slot->frame.x, slot->frame.y, slot->frame.width, slot->frame.height);
    container.hidden = !slot->visible;
    if (slot->clip_enabled) {
        container.frame = CGRectMake(slot->clip.x, slot->clip.y, slot->clip.width, slot->clip.height);
        container.clipsToBounds = YES;
        view.frame = CGRectMake(frame.origin.x - container.frame.origin.x, frame.origin.y - container.frame.origin.y, frame.size.width, frame.size.height);
    } else {
        container.frame = frame;
        container.clipsToBounds = NO;
        view.frame = container.bounds;
    }
    [container.superview bringSubviewToFront:container];
    return BOARD_OK;
}

static BoardResult board_ios_slot_create(BoardBackend *backend, const BoardNativeViewSlotConfig *config, BoardNativeViewSlot *slot) {
    BoardIosState *state = backend ? (BoardIosState *)backend->implementation : NULL;
    BoardIosNativeViewSlot *native_slot;
    BoardIosView *host;
    UIView *view;
    UIView *container;
    if (!state || !state->view || !config || !slot || ![NSThread isMainThread]) return BOARD_ERROR_INVALID_ARGUMENT;
    if (config->z_order != BOARD_NATIVE_VIEW_ABOVE_RENDERER) return BOARD_ERROR_UNAVAILABLE;
    view = (__bridge UIView *)config->native_view;
    if (![view isKindOfClass:UIView.class]) return BOARD_ERROR_INVALID_ARGUMENT;
    host = (__bridge BoardIosView *)state->view;
    native_slot = (BoardIosNativeViewSlot *)calloc(1, sizeof(*native_slot));
    if (!native_slot) return BOARD_ERROR_OUT_OF_MEMORY;
    container = [[UIView alloc] initWithFrame:CGRectZero];
    [container addSubview:view];
    [host->_native_overlay addSubview:container];
    native_slot->container = (void *)CFBridgingRetain(container);
    slot->implementation = native_slot;
    return board_ios_slot_apply(slot);
}

static void board_ios_slot_destroy(BoardNativeViewSlot *slot) {
    BoardIosNativeViewSlot *native_slot = board_ios_slot(slot);
    if (!native_slot) return;
    if (native_slot->container) {
        UIView *container = (__bridge_transfer UIView *)native_slot->container;
        [container removeFromSuperview];
    }
    free(native_slot);
    slot->implementation = NULL;
}

#if BOARD_BUILD_IOS_OPENGL
static EAGLContext *board_ios_gl_eagl(BoardIosOpenGLContext *gl) { return (__bridge EAGLContext *)gl->eagl; }
static BoardResult board_ios_gl_rebuild(BoardIosState *state, BoardIosOpenGLContext *gl, uint32_t width, uint32_t height, float scale) {
    EAGLContext *eagl = board_ios_gl_eagl(gl);
    CAEAGLLayer *layer = (CAEAGLLayer *)((__bridge BoardIosView *)state->view).layer;
    GLint drawable_width = 0, drawable_height = 0;
    if (![EAGLContext setCurrentContext:eagl]) return BOARD_ERROR_PLATFORM;
    layer.contentsScale = scale;
    if (gl->framebuffer) glDeleteFramebuffers(1, &gl->framebuffer);
    if (gl->colorbuffer) glDeleteRenderbuffers(1, &gl->colorbuffer);
    glGenFramebuffers(1, &gl->framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gl->framebuffer);
    glGenRenderbuffers(1, &gl->colorbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, gl->colorbuffer);
    if (![eagl renderbufferStorage:GL_RENDERBUFFER fromDrawable:layer]) return BOARD_ERROR_PLATFORM;
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, gl->colorbuffer);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) return BOARD_ERROR_PLATFORM;
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &drawable_width);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &drawable_height);
    gl->width = drawable_width > 0 ? (uint32_t)drawable_width : width;
    gl->height = drawable_height > 0 ? (uint32_t)drawable_height : height;
    gl->scale = scale;
    state->backend->width = gl->width;
    state->backend->height = gl->height;
    state->backend->scale = scale;
    glViewport(0, 0, gl->width, gl->height);
    return BOARD_OK;
}
static BoardResult board_ios_gl_create(void *data, void **out_context) {
    BoardIosState *state = (BoardIosState *)data;
    BoardIosOpenGLContext *gl;
    EAGLContext *eagl;
    if (!state || !out_context || state->active_gl) return BOARD_ERROR_INVALID_ARGUMENT;
    gl = (BoardIosOpenGLContext *)calloc(1, sizeof(*gl));
    if (!gl) return BOARD_ERROR_OUT_OF_MEMORY;
    eagl = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
    if (!eagl) { free(gl); return BOARD_ERROR_UNAVAILABLE; }
    gl->eagl = (void *)CFBridgingRetain(eagl);
    state->active_gl = gl;
    if (board_ios_gl_rebuild(state, gl, state->backend->width, state->backend->height, state->backend->scale) != BOARD_OK) { CFBridgingRelease(gl->eagl); state->active_gl = NULL; free(gl); return BOARD_ERROR_PLATFORM; }
    *out_context = gl;
    return BOARD_OK;
}
static void board_ios_gl_destroy(void *data, void *context) {
    BoardIosState *state = (BoardIosState *)data;
    BoardIosOpenGLContext *gl = (BoardIosOpenGLContext *)context;
    if (!gl) return;
    [EAGLContext setCurrentContext:board_ios_gl_eagl(gl)];
    if (gl->framebuffer) glDeleteFramebuffers(1, &gl->framebuffer);
    if (gl->colorbuffer) glDeleteRenderbuffers(1, &gl->colorbuffer);
    [EAGLContext setCurrentContext:nil];
    CFBridgingRelease(gl->eagl);
    if (state && state->active_gl == gl) state->active_gl = NULL;
    free(gl);
}
static BoardResult board_ios_gl_make_current(void *data, void *context) { BoardIosState *state = (BoardIosState *)data; BoardIosOpenGLContext *gl = (BoardIosOpenGLContext *)context; if (!state || !gl || state->active_gl != gl) return BOARD_ERROR_INVALID_ARGUMENT; return [EAGLContext setCurrentContext:board_ios_gl_eagl(gl)] ? BOARD_OK : BOARD_ERROR_PLATFORM; }
static void *board_ios_gl_get_proc(void *data, const char *name) { (void)data; (void)name; return NULL; }
static BoardResult board_ios_gl_drawable_size(void *data, uint32_t *width, uint32_t *height, float *scale) { BoardIosState *state = (BoardIosState *)data; BoardIosOpenGLContext *gl = state ? (BoardIosOpenGLContext *)state->active_gl : NULL; if (!gl || !width || !height || !scale) return BOARD_ERROR_INVALID_ARGUMENT; *width = gl->width; *height = gl->height; *scale = gl->scale; return BOARD_OK; }
static BoardResult board_ios_gl_swap(void *data) { BoardIosState *state = (BoardIosState *)data; BoardIosOpenGLContext *gl = state ? (BoardIosOpenGLContext *)state->active_gl : NULL; if (!gl || ![EAGLContext setCurrentContext:board_ios_gl_eagl(gl)]) return BOARD_ERROR_PLATFORM; glBindRenderbuffer(GL_RENDERBUFFER, gl->colorbuffer); return [board_ios_gl_eagl(gl) presentRenderbuffer:GL_RENDERBUFFER] ? BOARD_OK : BOARD_ERROR_PLATFORM; }
#endif

static BoardResult board_ios_present(void *data) {
    BoardIosState *state = (BoardIosState *)data;
    if (!state || !state->view) return BOARD_ERROR_INVALID_ARGUMENT;
    [(__bridge BoardIosView *)state->view setNeedsDisplay];
    return BOARD_OK;
}
static BoardResult board_ios_start(BoardBackend *backend) {
    BoardIosState *state = backend ? (BoardIosState *)backend->implementation : NULL;
    if (!state || !state->view) return BOARD_ERROR_UNAVAILABLE;
    [(__bridge BoardIosView *)state->view startFrames];
    return BOARD_OK;
}
static void board_ios_dispose(BoardBackend *backend) {
    BoardIosState *state = backend ? (BoardIosState *)backend->implementation : NULL;
    if (!state) return;
#if BOARD_BUILD_IOS_OPENGL
    if (state->active_gl) board_ios_gl_destroy(state, state->active_gl);
#endif
    [(__bridge BoardIosView *)state->view stopFrames];
    CFBridgingRelease(state->view);
    free(state);
    free(backend->pixels);
    backend->implementation = NULL;
    backend->pixels = NULL;
}

extern "C" BoardResult board_ios_backend_init(BoardBackend *backend, const BoardBackendConfig *config) {
    BoardIosState *state;
    BoardIosView *view;
    CGRect frame;
    if (!backend || !config) return BOARD_ERROR_INVALID_ARGUMENT;
    backend->scale = config->scale > 0 ? config->scale : UIScreen.mainScreen.scale;
    backend->width = config->width;
    backend->height = config->height;
    backend->stride = backend->width * 4u;
    backend->pixels = (uint8_t *)calloc(backend->height, backend->stride);
    state = (BoardIosState *)calloc(1, sizeof(*state));
    if (!backend->pixels || !state) { free(state); free(backend->pixels); backend->pixels = NULL; return BOARD_ERROR_OUT_OF_MEMORY; }
    frame = CGRectMake(0, 0, backend->width / backend->scale, backend->height / backend->scale);
    view = [[BoardIosView alloc] initWithBackend:backend frame:frame];
    state->backend = backend;
    state->view = (void *)CFBridgingRetain(view);
    backend->implementation = state;
    backend->native_slot_create = board_ios_slot_create;
    backend->native_slot_destroy = board_ios_slot_destroy;
    backend->native_slot_apply = board_ios_slot_apply;
    backend->surface.cpu = (BoardSurfaceCpuInterface){sizeof(BoardSurfaceCpuInterface), BOARD_ABI_VERSION, state, board_ios_map, board_ios_present};
#if BOARD_BUILD_IOS_OPENGL
    backend->surface.opengl = (BoardSurfaceOpenGLInterface){sizeof(BoardSurfaceOpenGLInterface), BOARD_ABI_VERSION, state, board_ios_gl_create, board_ios_gl_destroy, board_ios_gl_make_current, board_ios_gl_get_proc, board_ios_gl_drawable_size, board_ios_gl_swap};
#endif
#if BOARD_BUILD_IOS_METAL
    backend->surface.metal = (BoardSurfaceMetalInterface){sizeof(BoardSurfaceMetalInterface), BOARD_ABI_VERSION, (__bridge void *)view.layer, backend->width, backend->height, backend->scale};
#endif
    backend->start = board_ios_start;
    backend->dispose = board_ios_dispose;
    return BOARD_OK;
}

extern "C" BoardResult board_ios_view_get(BoardBackend *backend, void **out_view) {
    BoardIosState *state = backend ? (BoardIosState *)backend->implementation : NULL;
    if (!backend || backend->kind != BOARD_BACKEND_IOS || !out_view || !state || !state->view) return BOARD_ERROR_INVALID_ARGUMENT;
    *out_view = state->view;
    return BOARD_OK;
}
