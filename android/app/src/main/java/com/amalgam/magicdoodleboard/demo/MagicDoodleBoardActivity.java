/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
package com.amalgam.magicdoodleboard.demo;

import android.app.Activity;
import android.graphics.Color;
import android.os.Bundle;
import android.view.Gravity;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.TextView;
import org.magicdoodle.board.BoardView;

public final class MagicDoodleBoardActivity extends Activity {
    static { System.loadLibrary("magic_doodle_board_android_demo"); }

    @Override public void onCreate(Bundle state) {
        super.onCreate(state);
        LinearLayout host = new LinearLayout(this);
        host.setOrientation(LinearLayout.VERTICAL);
        TextView title = new TextView(this);
        title.setText("Native controls around a BoardView");
        title.setGravity(Gravity.CENTER);
        host.addView(title, new LinearLayout.LayoutParams(-1, 56));
        BoardView board = new BoardView(this);
        FrameLayout boardHost = new FrameLayout(this);
        boardHost.addView(board, new FrameLayout.LayoutParams(-1, -1));
        board.setNativeOverlayContainer(boardHost);
        TextView overlay = new TextView(this);
        overlay.setText("Overlay");
        overlay.setTextColor(Color.rgb(20, 31, 55));
        overlay.setTextSize(16.0f);
        overlay.setGravity(Gravity.CENTER);
        overlay.setClickable(true);
        overlay.setBackgroundColor(Color.rgb(246, 196, 69));
        overlay.setElevation(8.0f);
        overlay.setOnClickListener(view -> overlay.setText("Tapped"));
        board.setNativeOverlay(overlay);
        host.addView(boardHost, new LinearLayout.LayoutParams(-1, 0, 1));
        TextView button = new TextView(this);
        button.setText("Native control below BoardView");
        button.setTextColor(Color.rgb(20, 31, 55));
        button.setTextSize(16.0f);
        button.setGravity(Gravity.CENTER);
        button.setClickable(true);
        button.setBackgroundColor(Color.rgb(246, 196, 69));
        button.setOnClickListener(view -> button.setText("Control tapped"));
        host.addView(button, new LinearLayout.LayoutParams(-1, 56));
        setContentView(host);
    }
}
