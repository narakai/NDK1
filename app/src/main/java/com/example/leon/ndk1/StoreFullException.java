package com.example.leon.ndk1;

/**
 * Created by leon on 15/12/30.
 */
public class StoreFullException extends RuntimeException {
    public StoreFullException(String detailMessage) {
        super(detailMessage);
    }
}
