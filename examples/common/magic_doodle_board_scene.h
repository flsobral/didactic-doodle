/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
#ifndef MAGIC_DOODLE_BOARD_SCENE_H
#define MAGIC_DOODLE_BOARD_SCENE_H
#include <board/board_backend.h>
#include <doodle/doodle_renderer.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct MagicDoodleBoardScene {
    float width, height;
    float pointer_x, pointer_y;
    double elapsed;
    char board_backend[96];
    char magic_backend[160];
    char renderer[96];
    unsigned pointer_down : 1;
} MagicDoodleBoardScene;

void magic_doodle_board_scene_init(MagicDoodleBoardScene *scene, float width, float height);
void magic_doodle_board_scene_set_runtime(MagicDoodleBoardScene *scene, const BoardBackend *board, const MagicContext *magic, const DoodleRenderer *renderer);
void magic_doodle_board_scene_event(MagicDoodleBoardScene *scene, const BoardEvent *event);
void magic_doodle_board_scene_update(MagicDoodleBoardScene *scene, double delta_seconds);
void magic_doodle_board_scene_draw(MagicDoodleBoardScene *scene, DoodleCanvas *canvas);
#ifdef __cplusplus
}
#endif
#endif
