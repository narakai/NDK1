//
// Created by leon on 15/12/28.
//
#include "Store.h"
#include <cstdlib>
#include <cstring>
#include <unistd.h>

bool isEntryValid(JNIEnv *pEnv, StoreEntry *pEntry, StoreType pType) {
//    return ((pEntry != NULL) && (pEntry->mType == pType));
    if(pEntry == NULL){
        throwNotExistingKeyException(pEnv);
    }else if (pEntry->mType != pType) {
        throwInvalidTypeException(pEnv);
    }
    return !pEnv->ExceptionCheck();
}

StoreEntry *allocateEntry(JNIEnv *pEnv, Store *pStore, jstring pKey) {
    // If entry already exists in the store, releases its content
    // and keep its key.
    StoreEntry *entry = findEntry(pEnv, pStore, pKey);
    if (entry != NULL) {
        releaseEntryValue(pEnv, entry);
    }

        // If entry does not exist, create a new entry
        // right after the entries already stored.
    else {
        //// Checks store can accept a new entry.
        if(pStore->mLength >= STORE_MAX_CAPACITY){
            throwStoreFullException(pEnv);
            return NULL;
        }
        entry = pStore->mEntries + pStore->mLength;
        //use GetStringUTFChars() from the JNI API to get a temporary character buffer containing the converted Java string
        const char *tmpKey = pEnv->GetStringUTFChars(pKey, NULL);
        entry->mKey = new char[strlen(tmpKey) + 1];
        // Copies the new key into its final C string buffer.
        strcpy(entry->mKey, tmpKey);
        //GetStringUTFChars() must be systematically cally coupled with a call to ReleaseStringUTFChars()
        // to release the temporary buffer allocated in GetStringUTFChars()
        pEnv->ReleaseStringUTFChars(pKey, tmpKey);
        ++pStore->mLength;
    }
    return entry;
}


StoreEntry *findEntry(JNIEnv *pEnv, Store *pStore, jstring pKey) {
    //它被赋值给了entry.是StoreEntry结构体指针类型
    //pStore->mEntries entry指向的起始pStore指向StoreEntry数组的起始位置
    StoreEntry *entry = pStore->mEntries;
    //可以相加 ,entry数组后mLength的位置，即结束位置
    StoreEntry *entryEnd = entry + pStore->mLength;

    // Compare requested key with every entry key currently stored
    // until we find a matching one.
    const char *tmpKey = pEnv->GetStringUTFChars(pKey, NULL);
    //比较两个字符串设这两个字符串为str1，str2，若str1==str2，则返回零，否则指针++
    while ((entry < entryEnd) && (strcmp(entry->mKey, tmpKey) != 0)) {
        ++entry;
    }
    //第一个参数指定一个jstring变量，即是要释放的本地字符串的来源, 第二个参数就是要释放的本地字符串
    pEnv->ReleaseStringUTFChars(pKey, tmpKey);
    //entry==entryEnd表示找到了最后还没找到，所以返回空
    return (entry == entryEnd) ? NULL : entry;
}

void releaseEntryValue(JNIEnv *pEnv, StoreEntry *pEntry) {
    switch (pEntry->mType) {
        case StoreType_String:
            delete pEntry->mValue.mString;
            break;
        case StoreType_Color:
            // Unreferences the object for garbage collection.
            pEnv->DeleteGlobalRef(pEntry->mValue.mColor);
            break;
        case StoreType_IntegerArray:
            delete[] pEntry->mValue.mIntegerArray;
            break;
        case StoreType_StringArray:
            // Destroys every C string pointed by the String array before releasing it.
            for (int32_t i = 0; i < pEntry->mLength; ++i) {
                delete pEntry->mValue.mStringArray[i];
            }
            delete[] pEntry->mValue.mStringArray;
            break;
        case StoreType_ColorArray:
            // Unreferences every color before releasing the Id array.
            for (int32_t i = 0; i < pEntry->mLength; ++i) {
                pEnv->DeleteGlobalRef(pEntry->mValue.mColorArray[i]);
            }
            delete[] pEntry->mValue.mColorArray;
            break;
    }
}

void throwNotExistingKeyException(JNIEnv* pEnv){
    jclass clazz = pEnv->FindClass("com/example/leon/ndk1/NotExistingKeyException");
    if(clazz != NULL){
        pEnv->ThrowNew(clazz, "Key does not exist.");
    }
    pEnv->DeleteLocalRef(clazz);
}

void throwInvalidTypeException(JNIEnv* pEnv){
    jclass clazz = pEnv->FindClass("com/example/leon/ndk1/InvalidTypeException");
    if(clazz != NULL){
        pEnv->ThrowNew(clazz, "Invalid type.");
    }
    pEnv->DeleteLocalRef(clazz);
}

void throwStoreFullException(JNIEnv* pEnv){
    jclass clazz = pEnv->FindClass("com/example/leon/ndk1/StoreFullException");
    if(clazz != NULL){
        pEnv->ThrowNew(clazz, "Store is full.");
    }
    pEnv->DeleteLocalRef(clazz);
}

StoreWatcher* startWatcher(JavaVM* pJavaVM, Store* pStore, jobject pLock){
    StoreWatcher* watcher = new StoreWatcher();
    watcher->mJavaVM = pJavaVM;
    watcher->mStore = pStore;
    watcher->mLock = pLock;
    watcher->mRunning = true;
    //initialize and launch a native thread with the PThread POSIX API
    pthread_attr_t lAttributes;
    //initializes the necessary data structure
    if(pthread_attr_init(&lAttributes)) abort();
    //starts the thread
    if(pthread_create(&watcher->mThread, &lAttributes, runWatcher, watcher)) abort();
    return watcher;
}

void stopWatcher(StoreWatcher* pWatcher) {
    pWatcher->mRunning = false;
}

//Implement the thread's main loop on the watcher thread, not UI thread
//void型指针,表示这个指针指向的内存中的数据的类型要由用户来指定
void* runWatcher(void* pArgs){
    StoreWatcher* watcher = (StoreWatcher*) pArgs;
    Store* store = watcher->mStore;

    JavaVM* javaVM = watcher->mJavaVM;
    JavaVMAttachArgs javaVMAttachArgs;
    javaVMAttachArgs.version = JNI_VERSION_1_6;
    javaVMAttachArgs.name = "NativeThread";
    javaVMAttachArgs.group = NULL;

    JNIEnv* env;
    //attach the thread as a daemon to the Dalvik VM
    if (javaVM->AttachCurrentThreadAsDaemon(&env, &javaVMAttachArgs) != JNI_OK) abort();
    // Runs the thread loop.
    while (true) {
        sleep(5); // In seconds.
        // Critical section beginning, one thread at a time.
        // Entries cannot be added or modified.
        //上锁
        env->MonitorEnter(watcher->mLock);
        if (!watcher->mRunning) break;
        StoreEntry* entry = watcher->mStore->mEntries;
        StoreEntry* entryEnd = entry + watcher->mStore->mLength;
        while (entry < entryEnd) {
            processEntry(entry);
            ++entry;
        }
        // Critical section end.
        env->MonitorExit(watcher->mLock);
    }
    javaVM->DetachCurrentThread();
    delete watcher;
    pthread_exit(NULL);
}

void processEntry(StoreEntry* pEntry) {
    switch (pEntry->mType) {
        case StoreType_Integer:
            if (pEntry->mValue.mInteger > 100000) {
                pEntry->mValue.mInteger = 100000;
            } else if (pEntry->mValue.mInteger < -100000) {
                pEntry->mValue.mInteger = -100000;
            }
            break;
    }
}