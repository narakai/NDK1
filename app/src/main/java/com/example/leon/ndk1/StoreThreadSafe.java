package com.example.leon.ndk1;

/**
 * Created by leon on 15/12/31.
 */
public class StoreThreadSafe extends Store {
    //not Objects!!
    protected static Object LOCK;

    public StoreThreadSafe(StoreListener mListener) {
        super(mListener);
    }

    @Override
    public int getCount() throws NotExistingKeyException, InvalidTypeException {
        synchronized (LOCK) {
            return super.getCount();
        }
    }

    @Override
    public String getString(String pKey) throws NotExistingKeyException, InvalidTypeException {
        synchronized (LOCK) {
            return super.getString(pKey);
        }
    }

    @Override
    public int getInteger(String pKey) throws NotExistingKeyException, InvalidTypeException {
        synchronized (LOCK) {
            return super.getInteger(pKey);
        }
    }

    @Override
    public boolean getBoolean(String pKey) throws NotExistingKeyException, InvalidTypeException {
        synchronized (LOCK) {
            return super.getBoolean(pKey);
        }
    }

    @Override
    public Color getColor(String pKey) throws NotExistingKeyException, InvalidTypeException {
        synchronized (LOCK) {
            return super.getColor(pKey);
        }
    }

    @Override
    public void setString(String pKey, String pString) {
        synchronized (LOCK) {
            super.setString(pKey, pString);
        }
    }

    @Override
    public void setInteger(String pKey, int pInt) {
        synchronized (LOCK) {
            super.setInteger(pKey, pInt);
        }
    }

    @Override
    public void setBoolean(String pKey, boolean pBoolean) {
        synchronized (LOCK) {
            super.setBoolean(pKey, pBoolean);
        }
    }

    @Override
    public void setColor(String pKey, Color pColor) {
        synchronized (LOCK) {
            super.setColor(pKey, pColor);
        }
    }

    @Override
    public int[] getIntegerArray(String pKey) throws NotExistingKeyException, InvalidTypeException {
        synchronized (LOCK) {
            return super.getIntegerArray(pKey);
        }
    }

    @Override
    public void setIntegerArray(String pKey, int[] pIntArray) {
        synchronized (LOCK) {
            super.setIntegerArray(pKey, pIntArray);
        }
    }

    @Override
    public String[] getStringArray(String pKey) throws NotExistingKeyException, InvalidTypeException {
        synchronized (LOCK) {
            return super.getStringArray(pKey);
        }
    }

    @Override
    public void setStringArray(String pKey, String[] pStringArray) {
        synchronized (LOCK) {
            super.setStringArray(pKey, pStringArray);
        }
    }

    @Override
    public Color[] getColorArray(String pKey) throws NotExistingKeyException, InvalidTypeException {
        synchronized (LOCK) {
            return super.getColorArray(pKey);
        }
    }

    @Override
    public void setColorArray(String pKey, Color[] pColorArray) {
        synchronized (LOCK) {
            super.setColorArray(pKey, pColorArray);
        }
    }

    @Override
    public void stopWatcher(long pPointer) {
        synchronized (LOCK) {
            super.stopWatcher(pPointer);
        }
    }

}
