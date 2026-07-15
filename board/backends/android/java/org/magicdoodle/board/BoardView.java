/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
package org.magicdoodle.board;

import android.content.Context;
import android.graphics.Rect;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.FrameLayout;

/** Embeddable native Board host. Its application handle is private to JNI. */
public final class BoardView extends FrameLayout implements SurfaceHolder.Callback {
    private long applicationHandle;
    private final SurfaceView renderingSurface;
    private View pendingOverlay;

    public BoardView(Context context) {
        super(context);
        renderingSurface = new SurfaceView(context);
        renderingSurface.getHolder().addCallback(this);
        addView(renderingSurface, new FrameLayout.LayoutParams(-1, -1));
    }

    public void setNativeOverlay(View overlay) {
        pendingOverlay = overlay;
        if (applicationHandle != 0) nativeAttachOverlay(applicationHandle, overlay, 16, 16, 132, 36);
    }

    @Override public void surfaceCreated(SurfaceHolder holder) {
        applicationHandle = nativeCreate(this, holder.getSurface(), getWidth(), getHeight());
        if (applicationHandle != 0) {
            nativeStart(applicationHandle);
            if (pendingOverlay != null) nativeAttachOverlay(applicationHandle, pendingOverlay, 16, 16, 132, 36);
        }
    }

    @Override public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        voidFormat(format);
        if (applicationHandle != 0) nativeResize(applicationHandle);
    }

    @Override public void surfaceDestroyed(SurfaceHolder holder) {
        if (applicationHandle != 0) nativeDestroy(applicationHandle);
        applicationHandle = 0;
    }

    private static void voidFormat(int format) { }
    void addNativeOverlay(View overlay) {
        if (overlay.getParent() != this) addView(overlay);
    }
    void updateNativeOverlay(View overlay, float x, float y, float width, float height, float clipX, float clipY, float clipWidth, float clipHeight, boolean clipped, boolean visible) {
        FrameLayout.LayoutParams layout = new FrameLayout.LayoutParams((int) width, (int) height);
        layout.leftMargin = (int) x;
        layout.topMargin = (int) y;
        overlay.setLayoutParams(layout);
        overlay.setClipBounds(clipped ? new Rect((int) (clipX - x), (int) (clipY - y), (int) (clipX - x + clipWidth), (int) (clipY - y + clipHeight)) : null);
        overlay.setVisibility(visible ? VISIBLE : GONE);
    }
    void removeNativeOverlay(View overlay) { removeView(overlay); }
    private static native long nativeCreate(BoardView host, android.view.Surface surface, int width, int height);
    private static native void nativeStart(long applicationHandle);
    private static native void nativeResize(long applicationHandle);
    private static native void nativeAttachOverlay(long applicationHandle, View overlay, float x, float y, float width, float height);
    private static native void nativeDestroy(long applicationHandle);
}
