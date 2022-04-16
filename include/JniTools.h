//
// Created by xyzjiao on 8/2/21.
//

#ifndef JVM_C___JNITOOLS_H
#define JVM_C___JNITOOLS_H

#include <jni.h>
#include "common.h"


class JniTools {
public:
    // 字符串处理
    static const char* getCharsFromJString(jstring str, jboolean is_copy);
    static jstring charsToJstring(const char* str);
    static string* jstringToString(jstring str, jboolean is_copy);
    static jstring stringToJString(const string& str);
    static const char * javaStringToCString(const jobject &javaString);
    static jobject stringToJavaString(const string& s);
    static string *charsToString(const char *s);
    static jobject charsToJavaString(const char* s);
    static const char *stringToChars(const string &str);
    static jobject jstringToJavaString(jstring string, jboolean is_copy);

    static jmethodID get_method_id(const char* class_name, const char* method_name, const char* descriptor_name);
    static jmethodID get_method_id(jclass clz, const char *method_name, const char *descriptor_name);

    static jfieldID get_field_id(const char *class_name, const char *field_name, const char *descriptor_name);
    static jfieldID get_field_id(jclass clz, const char *field_name, const char *descriptor_name);

    static jclass find_class(const char* class_name);

    static jint get_fd(jobject netObject, const char* name);

    static jstring jint_to_jstring(jint num);
};


#endif //JVM_C___JNITOOLS_H
