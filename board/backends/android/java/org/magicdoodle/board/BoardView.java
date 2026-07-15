/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
package org.magicdoodle.board;

import android.content.Context;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

/** Embeddable native Board host. Its application handle is private to JNI. */
public final class BoardView extends SurfaceView implements SurfaceHolder.Callback {
    private long applicationHandle;

    public BoardView(Context context) {
        super(context);
        getHolder().addCallback(this);
    }

    @Override public void surfaceCreated(SurfaceHolder holder) {
        applicationHandle = nativeCreate(holder.getSurface(), getWidth(), getHeight());
        if (applicationHandle != 0) nativeStart(applicationHandle);
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
    private static native long nativeCreate(android.view.Surface surface, int width, int height);
    private static native void nativeStart(long applicationHandle);
    private static native void nativeResize(long applicationHandle);
    private static native void nativeDestroy(long applicationHandle);
}
