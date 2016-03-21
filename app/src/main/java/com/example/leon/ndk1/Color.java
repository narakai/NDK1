package com.example.leon.ndk1;

import android.text.TextUtils;

/**
 * Created by leon on 15/12/30.
 */
public class Color {
    private int mColor;

    public Color(String pColor) {
        if(TextUtils.isEmpty(pColor)){
            throw new IllegalArgumentException();
        }
        mColor = android.graphics.Color.parseColor(pColor);
    }

    @Override
    public String toString() {
        return String.format("#%06X", mColor);
    }
}
