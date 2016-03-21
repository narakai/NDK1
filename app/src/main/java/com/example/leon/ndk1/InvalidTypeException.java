package com.example.leon.ndk1;

/**
 * Created by leon on 15/12/30.
 */
public class InvalidTypeException extends Exception{
    public InvalidTypeException(String detailMessage) {
        super(detailMessage);
    }
}
