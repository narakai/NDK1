package com.example.leon.ndk1;

/**
 * Created by leon on 15/12/30.
 */
public interface StoreListener {
    void onSuccess(int pValue);
    void onSuccess(String pValue);
    void onSuccess(Color pValue);
}
