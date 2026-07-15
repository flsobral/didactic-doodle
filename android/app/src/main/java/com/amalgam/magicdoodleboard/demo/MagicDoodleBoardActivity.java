/* SPDX-FileCopyrightText: 2026 Amalgam Solucoes em TI Ltda. */
/* SPDX-License-Identifier: LGPL-2.1-only */
package com.amalgam.magicdoodleboard.demo;

import android.app.Activity;
import android.os.Bundle;
import android.view.Gravity;
import android.widget.Button;
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
        host.addView(board, new LinearLayout.LayoutParams(-1, 0, 1));
        Button button = new Button(this);
        button.setText("Native control below BoardView");
        host.addView(button, new LinearLayout.LayoutParams(-1, 56));
        setContentView(host);
    }
}
