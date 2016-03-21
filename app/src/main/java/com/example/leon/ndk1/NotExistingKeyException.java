package com.example.leon.ndk1;

/**
 * Created by leon on 15/12/30.
 */
public class NotExistingKeyException extends Exception{
    public NotExistingKeyException(String detailMessage) {
        super(detailMessage);
    }
}
