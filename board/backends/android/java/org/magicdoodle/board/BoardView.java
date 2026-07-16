/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
package org.magicdoodle.board;

import android.content.Context;
import android.graphics.Rect;
import android.graphics.SurfaceTexture;
import android.view.Surface;
import android.view.TextureView;
import android.view.View;
import android.view.ViewGroup;
import android.widget.FrameLayout;

/** Embeddable native Board host. Its application handle is private to JNI. */
public final class BoardView extends FrameLayout implements TextureView.SurfaceTextureListener {
    private long applicationHandle;
    private final TextureView renderingSurface;
    private Surface nativeSurface;
    private View pendingOverlay;
    private FrameLayout nativeOverlayContainer;

    public BoardView(Context context) {
        super(context);
        renderingSurface = new TextureView(context);
        renderingSurface.setSurfaceTextureListener(this);
        addView(renderingSurface, new FrameLayout.LayoutParams(-1, -1));
    }

    public void setNativeOverlay(View overlay) {
        pendingOverlay = overlay;
        if (applicationHandle != 0) nativeAttachOverlay(applicationHandle, overlay, 56, 82, 250, 94);
    }

    /** Sets the sibling container that composes native slots above the rendering surface. */
    public void setNativeOverlayContainer(FrameLayout container) {
        nativeOverlayContainer = container;
    }

    @Override public void onSurfaceTextureAvailable(SurfaceTexture texture, int width, int height) {
        nativeSurface = new Surface(texture);
        applicationHandle = nativeCreate(this, nativeSurface, width, height);
        if (applicationHandle != 0) {
            nativeStart(applicationHandle);
            if (pendingOverlay != null) nativeAttachOverlay(applicationHandle, pendingOverlay, 56, 82, 250, 94);
        }
    }

    @Override public void onSurfaceTextureSizeChanged(SurfaceTexture texture, int width, int height) {
        voidTexture(texture, width, height);
        if (applicationHandle != 0) nativeResize(applicationHandle);
    }

    @Override public boolean onSurfaceTextureDestroyed(SurfaceTexture texture) {
        if (applicationHandle != 0) nativeDestroy(applicationHandle);
        applicationHandle = 0;
        if (nativeSurface != null) nativeSurface.release();
        nativeSurface = null;
        return true;
    }

    @Override public void onSurfaceTextureUpdated(SurfaceTexture texture) { }

    private static void voidTexture(SurfaceTexture texture, int width, int height) { }
    private FrameLayout overlayContainer() {
        return nativeOverlayContainer != null ? nativeOverlayContainer : this;
    }

    private int[] overlayOffset(FrameLayout container) {
        if (container == this) return new int[] {0, 0};
        int[] boardLocation = new int[2];
        int[] containerLocation = new int[2];
        getLocationOnScreen(boardLocation);
        container.getLocationOnScreen(containerLocation);
        return new int[] {boardLocation[0] - containerLocation[0], boardLocation[1] - containerLocation[1]};
    }

    void addNativeOverlay(View overlay) {
        FrameLayout container = overlayContainer();
        if (overlay.getParent() != container) {
            if (overlay.getParent() instanceof ViewGroup) ((ViewGroup) overlay.getParent()).removeView(overlay);
            container.addView(overlay);
        }
        container.bringChildToFront(overlay);
    }
    void updateNativeOverlay(View overlay, float x, float y, float width, float height, float clipX, float clipY, float clipWidth, float clipHeight, boolean clipped, boolean visible) {
        FrameLayout container = overlayContainer();
        int[] offset = overlayOffset(container);
        FrameLayout.LayoutParams layout = new FrameLayout.LayoutParams((int) width, (int) height);
        layout.leftMargin = (int) x + offset[0];
        layout.topMargin = (int) y + offset[1];
        overlay.setLayoutParams(layout);
        container.bringChildToFront(overlay);
        overlay.setClipBounds(clipped ? new Rect((int) (clipX - x), (int) (clipY - y), (int) (clipX - x + clipWidth), (int) (clipY - y + clipHeight)) : null);
        overlay.setVisibility(visible ? VISIBLE : GONE);
    }
    void removeNativeOverlay(View overlay) {
        ViewGroup parent = overlay.getParent() instanceof ViewGroup ? (ViewGroup) overlay.getParent() : null;
        if (parent != null) parent.removeView(overlay);
    }
    private static native long nativeCreate(BoardView host, android.view.Surface surface, int width, int height);
    private static native void nativeStart(long applicationHandle);
    private static native void nativeResize(long applicationHandle);
    private static native void nativeAttachOverlay(long applicationHandle, View overlay, float x, float y, float width, float height);
    private static native void nativeDestroy(long applicationHandle);
}
