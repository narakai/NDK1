//
// Created by leon on 15/12/28.
//

#include "Store.h"
#include "com_example_leon_ndk1_Store.h"
#include <cstdlib>
#include <cstdint>

static Store gStore;
//Classes are accessible by JNIEnv itself, and are retrievable from the JavaVM given to JNI_OnLoad().
static jclass StringClass;
static jclass ColorClass;
static jmethodID MethodOnSuccessInt;
static jmethodID MethodOnSuccessString;
static jmethodID MethodOnSuccessColor;
//create a new static variable gLock that is going to hold the Java synchronization object
static jobject gLock;

JNIEXPORT jint
JNI_OnLoad(JavaVM *pVM, void *reserved) {
    JNIEnv *env;
    if (pVM->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        abort();
    }
    // If returned class is null, an exception is raised by the VM.
    //We need to check whether the found classes are null in case they cannot be loaded.
    //If that happens, an exception is raised by the VM so that we can return immediately
    jclass StringClassTmp = env->FindClass("java/lang/String");
    if (StringClassTmp == NULL) abort();
    StringClass = (jclass) env->NewGlobalRef(StringClassTmp);
    env->DeleteLocalRef(StringClassTmp);

    jclass ColorClassTmp = env->FindClass("com/example/leon/ndk1/Color");
    if (ColorClassTmp == NULL) abort();
    ColorClass = (jclass) env->NewGlobalRef(ColorClassTmp);
    env->DeleteLocalRef(ColorClassTmp);

    // Caches methods.
    //1.Getting a Class descriptor with the JNI method FindClass()
    jclass StoreClass = env->FindClass("com/example/leon/ndk1/Store");
    if (StoreClass == NULL) abort();
    //2.Retrieving a method descriptor from the class descriptor with GetMethodID()
    //(<Parameter 1 Type Code>[<Parameter 1 Class>];...)<Return Type Code>
    MethodOnSuccessInt = env->GetMethodID(StoreClass, "onSuccess", "(I)V");
    if (MethodOnSuccessInt == NULL) abort();
    MethodOnSuccessString = env->GetMethodID(StoreClass, "onSuccess", "(Ljava/lang/String;)V");
    if (MethodOnSuccessString == NULL) abort();
    MethodOnSuccessColor = env->GetMethodID(StoreClass, "onSuccess", "(Lcom/example/leon/ndk1/Color;)V");
    if (MethodOnSuccessColor == NULL) abort();
    env->DeleteLocalRef(StoreClass);

    // Store initialization
    gStore.mLength = 0;

    //Create an instance of the Object class in JNI_OnLoad() using the JNI Re ec on API
    //First, find its Object constructor with GetMethodID(). Constructors in JNI are named <init> and have no result.
    jclass ObjectClass = env->FindClass("java/lang/Object");
    if(ObjectClass == NULL) abort();
    //Then, invoke the constructor to create an instance and make it global.
    jmethodID ObjectConstructor = env->GetMethodID(ObjectClass, "<init>", "()V");
    if(ObjectConstructor == NULL) abort();
    jobject lockTmp = env->NewObject(ObjectClass, ObjectConstructor);
    //remove local references when they become useless
    env->DeleteLocalRef(ObjectClass);
    gLock = env->NewGlobalRef(lockTmp);
    env->DeleteLocalRef(lockTmp);

    //Save the created Object instance in the StoreThreadSafe.LOCK field
    //First, retrieve the StoreThreadSafe class and its LOCK field using the JNI Reflection methods FindClass() and GetStaticFieldId()
    jclass StoreThreadSafeClass = env->FindClass("com/example/leon/ndk1/StoreThreadSafe");
    if(StoreThreadSafeClass == NULL) abort();
    //拿到static object
    jfieldID lockField = env->GetStaticFieldID(StoreThreadSafeClass, "LOCK", "Ljava/lang/Object;");
    if (lockField == NULL) abort();
    //Then, save the value into the LOCK static field with the JNI method SetStaticObjectField(), which requires a field signature (such as methods)
    env->SetStaticObjectField(StoreThreadSafeClass, lockField, gLock);
    //Finally, remove the local reference to the StoreThreadSafe class when it becomes useless
    env->DeleteLocalRef(StoreThreadSafeClass);

    return JNI_VERSION_1_6;
}

JNIEXPORT jint JNICALL
Java_com_example_leon_ndk1_Store_getCount(JNIEnv *env, jobject instance) {
    return gStore.mLength;
}

JNIEXPORT jstring JNICALL
Java_com_example_leon_ndk1_Store_getString(JNIEnv *pEnv, jobject pThis, jstring pKey) {
    //据结构体成员指针找到结构体指针 : &gStore
    StoreEntry *entry = findEntry(pEnv, &gStore, pKey);
    if (isEntryValid(pEnv, entry, StoreType_String)) {
        // Converts a C string into a Java String.
        return pEnv->NewStringUTF(entry->mValue.mString);
    } else {
        return NULL;
    }
}

JNIEXPORT void JNICALL
Java_com_example_leon_ndk1_Store_setString(JNIEnv *pEnv, jobject pThis, jstring pKey,
                                           jstring pString) {
    // Turns the Java string into a temporary C string.
    //每次执行mLength加1
    StoreEntry *entry = allocateEntry(pEnv, &gStore, pKey);
    if (entry != NULL) {
        entry->mType = StoreType_String;
// Copy the temporary C string into its dynamically allocated
// final location. Then releases the temporary string.
        jsize stringLength = pEnv->GetStringUTFLength(pString);
        entry->mValue.mString = new char[stringLength + 1];
// Directly copies the Java String into our new C buffer.
        pEnv->GetStringUTFRegion(pString, 0, stringLength,
                                 entry->mValue.mString);
// Append the null character for string termination.
        entry->mValue.mString[stringLength] = '\0';
        pEnv->CallVoidMethod(pThis, MethodOnSuccessString,
                             (jstring) pEnv->NewStringUTF(entry->mValue.mString));
    }
}

JNIEXPORT jint JNICALL
Java_com_example_leon_ndk1_Store_getInteger(JNIEnv *pEnv, jobject pThis, jstring pKey) {
    StoreEntry *entry = findEntry(pEnv, &gStore, pKey);
    if (isEntryValid(pEnv, entry, StoreType_Integer)) {
        return entry->mValue.mInteger;
    } else {
        return 0;
    }
}

JNIEXPORT void JNICALL
Java_com_example_leon_ndk1_Store_setInteger(JNIEnv *pEnv, jobject pThis, jstring pKey,
                                            jint pInteger) {
    StoreEntry *entry = allocateEntry(pEnv, &gStore, pKey);
    if (entry != NULL) {
        entry->mType = StoreType_Integer;
        entry->mValue.mInteger = pInteger;
        //打印success信息
        pEnv->CallVoidMethod(pThis, MethodOnSuccessInt, (jint) entry->mValue.mInteger);
    }
}

JNIEXPORT jboolean JNICALL
Java_com_example_leon_ndk1_Store_getBoolean(JNIEnv *pEnv, jobject pThis, jstring pKey) {
    //parse c value to java
    StoreEntry *entry = findEntry(pEnv, &gStore, pKey);
    if (isEntryValid(pEnv, entry, StoreType_Boolean)) {
        return entry->mValue.mBoolean;
    } else {
        return 0;
    }
}

JNIEXPORT void JNICALL
Java_com_example_leon_ndk1_Store_setBoolean(JNIEnv *pEnv, jobject pThis, jstring pKey,
                                            jboolean pBoolean) {
    //parse java value to c
    StoreEntry *entry = allocateEntry(pEnv, &gStore, pKey);
    if (entry != NULL) {
        entry->mType = StoreType_Boolean;
        entry->mValue.mBoolean = pBoolean;
    }
}

JNIEXPORT jobject JNICALL
Java_com_example_leon_ndk1_Store_getColor(JNIEnv *pEnv, jobject pThis, jstring pKey) {
    StoreEntry *entry = findEntry(pEnv, &gStore, pKey);
    if (isEntryValid(pEnv, entry, StoreType_Color)) {
        return entry->mValue.mColor;
    } else {
        return NULL;
    }
}

JNIEXPORT void JNICALL
Java_com_example_leon_ndk1_Store_setColor(JNIEnv *pEnv, jobject pThis, jstring pKey,
                                          jobject pColor) {
    StoreEntry *entry = allocateEntry(pEnv, &gStore, pKey);
    if (entry != NULL) {
        entry->mType = StoreType_Color;
        //To be allowed to keep a Java object reference in na ve code a er native method returns,
        // they must be turned into Global references in order to inform the Dalvik VM
        // that they must not be garbage collected.
        //不能直接用pColor：
        //Objects passed in parameters or created inside a JNI method are Local references.
        // Local references cannot be kept in na ve code outside of the native method scope (such as for strings).
        //to keep Java objects on the na ve side, we must keep references to them using the JNI API
        //All objects coming from Java are represented by jobject,
        //even jstring (which is in fact a typedef over jobject internally).

        //Never forget to pair New<Reference Type>Ref() with Delete<Reference Type>Ref().
        // Failure to do so results in a memory leak.
        entry->mValue.mColor = pEnv->NewGlobalRef(pColor);
        pEnv->CallVoidMethod(pThis, MethodOnSuccessColor,
                             (jstring) entry->mValue.mColor);
    }
}

JNIEXPORT jintArray JNICALL
Java_com_example_leon_ndk1_Store_getIntegerArray(JNIEnv *pEnv, jobject pThis, jstring pKey) {
    StoreEntry *entry = findEntry(pEnv, &gStore, pKey);
    if (isEntryValid(pEnv, entry, StoreType_IntegerArray)) {
        jintArray javaArray = pEnv->NewIntArray(entry->mLength);
        //use SetIntArrayRegion() to copy the native int buffer content into jintArray
        pEnv->SetIntArrayRegion(javaArray, 0, entry->mLength, entry->mValue.mIntegerArray);
        return javaArray;
    } else {
        return NULL;
    }
}

JNIEXPORT void JNICALL
Java_com_example_leon_ndk1_Store_setIntegerArray(JNIEnv *pEnv, jobject pThis, jstring pKey,
                                                 jintArray pIntegerArray) {
    StoreEntry *entry = allocateEntry(pEnv, &gStore, pKey);
    if (entry != NULL) {
        jsize length = pEnv->GetArrayLength(pIntegerArray);
        int32_t *array = new int32_t[length];
        pEnv->GetIntArrayRegion(pIntegerArray, 0, length, array);
        entry->mType = StoreType_IntegerArray;
        entry->mLength = length;
        entry->mValue.mIntegerArray = array;
    }
}

//Unlike primitive arrays, it is not possible to work on Java object array's all elements at the same time.
// Instead, objects are set one by one with SetObjectArrayElement(). Here, the native array is filled with String objects
// stored on the native side, which keeps Global references to them.
// So there is no need to delete or create any reference here except the reference to the newly allocated string.

JNIEXPORT jobjectArray JNICALL
Java_com_example_leon_ndk1_Store_getStringArray(JNIEnv *pEnv, jobject pThis, jstring pKey) {
    StoreEntry *entry = findEntry(pEnv, &gStore, pKey);
    if (isEntryValid(pEnv, entry, StoreType_StringArray)) {
        //// An array of String in Java is in fact an array of object.
        jobjectArray javaArray = pEnv->NewObjectArray(entry->mLength, StringClass, NULL);
        // Creates a new Java String object for each C string stored.
        // Reference to the String can be removed right after it is
        // added to the Java array, as the latter holds a reference
        // to the String object.
        for (int32_t i = 0; i < entry->mLength; ++i) {
            jstring string = pEnv->NewStringUTF(entry->mValue.mStringArray[i]);
            // Puts the new string in the array
            pEnv->SetObjectArrayElement(javaArray, i, string);
            // Do it here to avoid holding many useless local refs.
            pEnv->DeleteLocalRef(string);
        }
        return javaArray;
    } else {
        return NULL;
    }
}

//In setStringArray(), array elements are retrieved one by one with GetObjectArrayElement().
// Returned references are local and should be made global to store them safely on the native side.
JNIEXPORT void JNICALL
Java_com_example_leon_ndk1_Store_setStringArray(JNIEnv *pEnv, jobject pThis, jstring pKey,
                                                jobjectArray pStringArray) {
    // Creates a new entry with the new String array.
    StoreEntry *entry = allocateEntry(pEnv, &gStore, pKey);
    if (entry != NULL) {
        // Allocates an array of C string.
        jsize length = pEnv->GetArrayLength(pStringArray);
        char **array = new char *[length];
        // Fills the C array with a copy of each input Java string.
        for (int32_t i = 0; i < length; ++i) {
            // Gets the current Java String from the input Java array.
            // Object arrays can be accessed element by element only.
            jstring string = (jstring) pEnv->GetObjectArrayElement(pStringArray, i);
            jsize stringLength = pEnv->GetStringUTFLength(string);
            array[i] = new char[stringLength + 1];
            // Directly copies the Java String into our new C buffer.
            pEnv->GetStringUTFRegion(string, 0, stringLength, array[i]);
            // Append the null character for string termination.
            array[i][stringLength] = '\0';
            // No need to keep a reference to the Java string anymore.
            pEnv->DeleteLocalRef(string);
        }
        entry->mType = StoreType_StringArray;
        entry->mLength = length;
        entry->mValue.mStringArray = array;
    }
}

JNIEXPORT jobjectArray JNICALL
Java_com_example_leon_ndk1_Store_getColorArray(JNIEnv *pEnv, jobject pThis, jstring pKey) {
    StoreEntry *entry = findEntry(pEnv, &gStore, pKey);
    if (isEntryValid(pEnv, entry, StoreType_ColorArray)) {
        // Creates a new array with objects of type Id.
        jobjectArray javaArray = pEnv->NewObjectArray(entry->mLength,
                                                      ColorClass, NULL);
        // Fills the array with the Color objects stored on the native
        // side(jobject mColor), which keeps a global reference to them. So no need
        // to delete or create any reference here.
        for (int32_t i = 0; i < entry->mLength; ++i) {
            pEnv->SetObjectArrayElement(javaArray, i,
                                        entry->mValue.mColorArray[i]);
        }
        return javaArray;
    } else {
        return NULL;
    }
}

JNIEXPORT void JNICALL
Java_com_example_leon_ndk1_Store_setColorArray(JNIEnv *pEnv, jobject pThis, jstring pKey,
                                               jobjectArray pColorArray) {
    // Saves the Color array in the store.
    StoreEntry *entry = allocateEntry(pEnv, &gStore, pKey);
    if (entry != NULL) {
        // Allocates a C array of Color objects.
        jsize length = pEnv->GetArrayLength(pColorArray);
        jobject *array = new jobject[length];
        // Fills the C array with a copy of each input Java Color.
        for (int32_t i = 0; i < length; ++i) {
            // Gets the current Color object from the input Java array.
            // Object arrays can be accessed element by element only.
            //GetObjectArrayElement() retrieves one single object reference from a Java array. The returned reference is Local
            jobject localColor = pEnv->GetObjectArrayElement(
                    pColorArray, i);
            // The Java Color is going to be stored on the native side
            // Need to keep a global reference to avoid a potential
            // garbage collection after method returns.
            array[i] = pEnv->NewGlobalRef(localColor);
            // We have a global reference to the Color, so we can now
            // get rid of the local one.
            pEnv->DeleteLocalRef(localColor);
        }
        entry->mType = StoreType_ColorArray;
        entry->mLength = length;
        entry->mValue.mColorArray = array;
    }
}

JNIEXPORT jlong JNICALL Java_com_example_leon_ndk1_Store_startWatcher
        (JNIEnv * pEnv, jobject pThis){
    JavaVM * javaVM;
    // Caches the VM.
    if (pEnv->GetJavaVM(&javaVM) != JNI_OK) abort();

    // Launches the background thread.
    StoreWatcher* watcher = startWatcher(javaVM, &gStore, gLock);
    return (jlong) watcher;
}

JNIEXPORT void JNICALL Java_com_example_leon_ndk1_Store_stopWatcher
        (JNIEnv *pEnv, jobject pThis, jlong pWatcher){
        //转换jlong为指针
    stopWatcher((StoreWatcher*) pWatcher);
}


//Primitive arrays VS Object arrays
//Objects arrays are specific because unlike primitive arrays, each array element is a reference to an object
//As a consequence, a new Global reference is automatically registered each time an object is inserted in the array.
// That way, when native calls end, references do not get garbage collected. Note that object arrays cannot be converted to "native" arrays like primitives.





