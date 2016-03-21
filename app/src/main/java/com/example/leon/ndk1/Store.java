package com.example.leon.ndk1;

/**
 * Created by leon on 15/12/28.
 */
public class Store implements StoreListener{
    private StoreListener mListener;

    public Store(StoreListener mListener) {
        this.mListener = mListener;
    }

    static {
        System.loadLibrary("com_example_leon_ndk1_Store");
    }

    public native int getCount() throws NotExistingKeyException, InvalidTypeException;
//    public native String getString();
    public native String getString(String pKey) throws NotExistingKeyException, InvalidTypeException;
    public native int getInteger(String pKey) throws NotExistingKeyException, InvalidTypeException;
    //not Boolean!
    public native boolean getBoolean(String pKey) throws NotExistingKeyException, InvalidTypeException;
    public native Color getColor(String pKey) throws NotExistingKeyException, InvalidTypeException;
    public native void setString(String pKey, String pString);
    public native void setInteger(String pKey, int pInt);
    public native void setBoolean(String pKey, boolean pBoolean);
    public native void setColor(String pKey, Color pColor);

    public native int[] getIntegerArray(String pKey) throws NotExistingKeyException, InvalidTypeException;
    public native void setIntegerArray(String pKey, int[] pIntArray);

    public native String[] getStringArray(String pKey) throws NotExistingKeyException, InvalidTypeException;
    public native void setStringArray(String pKey, String[] pStringArray);

    public native Color[] getColorArray(String pKey) throws NotExistingKeyException, InvalidTypeException;
    public native void setColorArray(String pKey, Color[] pColorArray);

    //The use of a long value (represented on 64-bit) to save a native pointer
    public native long startWatcher();
    public native void stopWatcher(long pPointer);

    @Override
    public void onSuccess(int pValue) {
        mListener.onSuccess(pValue);
    }

    @Override
    public void onSuccess(String pValue) {
        mListener.onSuccess(pValue);
    }

    @Override
    public void onSuccess(Color pValue) {
        mListener.onSuccess(pValue);
    }
}
