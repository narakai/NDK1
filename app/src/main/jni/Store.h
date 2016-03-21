//
// Created by leon on 15/12/28.
//

#ifndef NDK1_STORE_H
#define NDK1_STORE_H

#include <cstdint>
#include "jni.h"
#include <pthread.h>

#define STORE_MAX_CAPACITY 16

typedef enum {
    //integrate strings into the native StoreType enumeration
    StoreType_Color,
    StoreType_Boolean,
    StoreType_Integer,
    StoreType_IntegerArray,
    StoreType_StringArray,
    StoreType_ColorArray,
    StoreType_String
} StoreType;

typedef union {
    jobject mColor;
    int32_t mInteger;
    uint8_t mBoolean;
    //java数组在c里的定义
    int32_t*  mIntegerArray;
    char**    mStringArray;
    jobject*  mColorArray;
    char *mString;
} StoreValue;

typedef struct {
    char *mKey;
    StoreType mType;
    StoreValue mValue;
    int32_t mLength;
} StoreEntry;

typedef struct {
    StoreEntry mEntries[STORE_MAX_CAPACITY];
    //the number of allocated entries,实际数量
    int32_t mLength;
} Store;

typedef struct {
    Store* mStore;
    JavaVM* mJavaVM;
    jobject mLock;
    pthread_t mThread;
    int32_t mRunning;
} StoreWatcher;

bool isEntryValid(JNIEnv *pEnv, StoreEntry *pEntry, StoreType pType);
StoreEntry *allocateEntry(JNIEnv *pEnv, Store *pStore, jstring pKey);
StoreEntry *findEntry(JNIEnv *pEnv, Store *pStore, jstring pKey);
void releaseEntryValue(JNIEnv *pEnv, StoreEntry *pEntry);

void throwInvalidTypeException(JNIEnv* pEnv);
void throwNotExistingKeyException(JNIEnv* pEnv);
void throwStoreFullException(JNIEnv* pEnv);

StoreWatcher* startWatcher(JavaVM* pJavaVM, Store* pStore, jobject pLock);
void stopWatcher(StoreWatcher* pWatcher);
void* runWatcher(void* pArgs);
void processEntry(StoreEntry* pEntry);

#endif //NDK1_STORE_H
