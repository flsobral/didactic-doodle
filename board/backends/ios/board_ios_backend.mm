/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#include "../../src/board_internal.h"
#include <board/board_ios.h>
#import <UIKit/UIKit.h>

typedef struct BoardIosState { BoardBackend *backend; void *view; } BoardIosState;

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

@interface BoardIosView : UIView {
@public
    BoardBackend *_board_backend;
    CADisplayLink *_display_link;
}
- (instancetype)initWithBackend:(BoardBackend *)backend frame:(CGRect)frame;
- (void)startFrames;
- (void)stopFrames;
@end

@implementation BoardIosView
- (instancetype)initWithBackend:(BoardBackend *)backend frame:(CGRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        _board_backend = backend;
        self.opaque = YES;
        self.contentScaleFactor = UIScreen.mainScreen.scale;
        self.multipleTouchEnabled = YES;
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
}
@end

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
    backend->surface.cpu = (BoardSurfaceCpuInterface){sizeof(BoardSurfaceCpuInterface), BOARD_ABI_VERSION, state, board_ios_map, board_ios_present};
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
