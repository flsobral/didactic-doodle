/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#include <board/board_app.h>
#include <board/board_ios.h>
#include <board/board_native_view.h>
#include <doodle/doodle_renderer.h>
#include <magic/magic_context.h>
#include "../common/magic_doodle_board_scene.h"
#import <UIKit/UIKit.h>

typedef struct IosDemo {
    BoardBackend *backend;
    BoardApp *app;
    MagicContext *magic;
    DoodleRenderer *renderer;
    BoardNativeViewSlot *overlay_slot;
    MagicDoodleBoardScene scene;
} IosDemo;

static void ios_demo_event(void *data, const BoardEvent *event) {
    IosDemo *demo = (IosDemo *)data;
    magic_doodle_board_scene_event(&demo->scene, event);
    if (event->type == BOARD_EVENT_RESIZE) magic_context_resize(demo->magic, event->data.resize.width, event->data.resize.height, event->data.resize.scale);
}
static void ios_demo_update(void *data, double delta_seconds) { magic_doodle_board_scene_update(&((IosDemo *)data)->scene, delta_seconds); }
static void ios_demo_frame(void *data, uint64_t timestamp_ns, double delta_seconds) {
    IosDemo *demo = (IosDemo *)data;
    MagicFrame *frame = NULL;
    DoodleCanvas *canvas = NULL;
    (void)timestamp_ns;
    (void)delta_seconds;
    if (magic_context_begin_frame(demo->magic, &frame) != MAGIC_OK) return;
    if (doodle_renderer_begin_frame(demo->renderer, frame, &canvas) == DOODLE_OK) {
        magic_doodle_board_scene_draw(&demo->scene, canvas);
        doodle_renderer_end_frame(demo->renderer, canvas);
    }
    magic_context_end_frame(demo->magic, frame);
}

@interface MagicDoodleBoardViewController : UIViewController { @public IosDemo _demo; }
@end
@implementation MagicDoodleBoardViewController
- (void)nativeOverlayTapped:(id)sender { (void)sender; NSLog(@"Magic Doodle Board native overlay tapped"); }
- (void)viewDidLoad {
    BoardBackendConfig backend_config;
#if MDB_IOS_OPENGL
    MagicConfig magic_config = {sizeof(MagicConfig), MAGIC_ABI_VERSION, MAGIC_BACKEND_OPENGL, 1};
#elif MDB_IOS_METAL
    MagicConfig magic_config = {sizeof(MagicConfig), MAGIC_ABI_VERSION, MAGIC_BACKEND_METAL, 1};
#else
    MagicConfig magic_config = {sizeof(MagicConfig), MAGIC_ABI_VERSION, MAGIC_BACKEND_CPU, 1};
#endif
    DoodleRendererConfig renderer_config = {sizeof(DoodleRendererConfig), DOODLE_ABI_VERSION, 0};
    BoardAppCallbacks callbacks = {sizeof(BoardAppCallbacks), BOARD_ABI_VERSION, NULL, ios_demo_event, ios_demo_update, ios_demo_frame, NULL};
    BoardAppConfig app_config;
    CGRect bounds = UIScreen.mainScreen.bounds;
    CGFloat scale = UIScreen.mainScreen.scale;
    void *view = NULL;
    UIView *board_view;
    UIView *container;
    UILabel *title;
    UIButton *outside_button;
    UIButton *overlay_button;
    BoardNativeViewSlotConfig slot_config;
    [super viewDidLoad];
    backend_config = (BoardBackendConfig){sizeof(BoardBackendConfig), BOARD_ABI_VERSION, BOARD_BACKEND_IOS, "Magic Doodle Board", (uint32_t)(bounds.size.width * scale), (uint32_t)(bounds.size.height * scale), (float)scale, 0, BOARD_HOST_MODE_HYBRID_OVERLAY};
    if (board_backend_create(&backend_config, &_demo.backend) != BOARD_OK || magic_context_create(board_backend_surface(_demo.backend), &magic_config, &_demo.magic) != MAGIC_OK || doodle_renderer_create(doodle_skia_provider(), _demo.magic, &renderer_config, &_demo.renderer) != DOODLE_OK || board_ios_view_get(_demo.backend, &view) != BOARD_OK) { NSLog(@"Magic Doodle Board: initialization failed"); return; }
    magic_doodle_board_scene_init(&_demo.scene, backend_config.width, backend_config.height);
    container = [[UIView alloc] initWithFrame:bounds];
    container.backgroundColor = UIColor.systemBackgroundColor;
    title = [[UILabel alloc] initWithFrame:CGRectMake(16, 16, bounds.size.width - 32, 32)];
    title.text = @"Native controls around a BoardView";
    title.textAlignment = NSTextAlignmentCenter;
    [container addSubview:title];
    board_view = (__bridge UIView *)view;
    board_view.frame = CGRectMake(16, 64, bounds.size.width - 32, bounds.size.height - 136);
    board_view.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    [container addSubview:board_view];
    outside_button = [UIButton buttonWithType:UIButtonTypeSystem];
    outside_button.frame = CGRectMake(16, bounds.size.height - 56, bounds.size.width - 32, 40);
    outside_button.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleTopMargin;
    [outside_button setTitle:@"Native control below BoardView" forState:UIControlStateNormal];
    [container addSubview:outside_button];
    overlay_button = [UIButton buttonWithType:UIButtonTypeSystem];
    [overlay_button setTitle:@"Native overlay" forState:UIControlStateNormal];
    [overlay_button addTarget:self action:@selector(nativeOverlayTapped:) forControlEvents:UIControlEventTouchUpInside];
    slot_config = (BoardNativeViewSlotConfig){sizeof(BoardNativeViewSlotConfig), BOARD_ABI_VERSION, (__bridge void *)overlay_button, {16, 16, 132, 36}, {0, 0, 160, 56}, 1, 1, BOARD_NATIVE_VIEW_ABOVE_RENDERER};
    if (board_native_view_slot_create(_demo.backend, &slot_config, &_demo.overlay_slot) != BOARD_OK) { NSLog(@"Magic Doodle Board: native overlay initialization failed"); return; }
    self.view = container;
    app_config = (BoardAppConfig){sizeof(BoardAppConfig), BOARD_ABI_VERSION, _demo.backend, callbacks, &_demo};
    if (board_app_create(&app_config, &_demo.app) != BOARD_OK || board_app_start(_demo.app) != BOARD_OK) NSLog(@"Magic Doodle Board: application start failed");
}
- (void)dealloc {
    board_native_view_slot_destroy(_demo.overlay_slot);
    board_app_destroy(_demo.app);
    doodle_renderer_destroy(_demo.renderer);
    magic_context_destroy(_demo.magic);
    board_backend_destroy(_demo.backend);
}
@end

@interface MagicDoodleBoardAppDelegate : UIResponder <UIApplicationDelegate>
@property(nonatomic, strong) UIWindow *window;
@end
@implementation MagicDoodleBoardAppDelegate
- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)options {
    (void)application;
    (void)options;
    self.window = [[UIWindow alloc] initWithFrame:UIScreen.mainScreen.bounds];
    self.window.rootViewController = [MagicDoodleBoardViewController new];
    [self.window makeKeyAndVisible];
    return YES;
}
@end
int main(int argc, char *argv[]) { @autoreleasepool { return UIApplicationMain(argc, argv, nil, NSStringFromClass(MagicDoodleBoardAppDelegate.class)); } }
