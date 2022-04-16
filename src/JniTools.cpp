//
// Created by xyzjiao on 8/2/21.
//

#include "../include/JniTools.h"

extern JNIEnv* g_env;

// jstring (jni) --> c string (char *)
const char *JniTools::getCharsFromJString(jstring str, jboolean is_copy) {
    return const_cast<char *>(g_env->GetStringUTFChars(str, &is_copy));
}

// jstring (jni) --> c++ string
string *JniTools::jstringToString(jstring str, jboolean is_copy) {
    const char* s = getCharsFromJString(str, is_copy);
    return new string(s);
}

// c++ string --> c string (char *)
const char *JniTools::stringToChars(const string &str) {
    return str.c_str();
}

// c++ string --> jstring (jni)
jstring JniTools::stringToJString(const string &str) {
    const char* s = str.c_str();
    return charsToJstring(s);
}

// c++ string --> java string
jobject JniTools::stringToJavaString(const string &str) {
    // 获取java String Class对象
    jclass string_clazz = g_env->FindClass("java/lang/String");
    // 获取java String的构造方法（入参为byte[]的构造方法）
    jmethodID string_constructor = g_env->GetMethodID(string_clazz, "<init>", "([B)V");
    // 创建jbyteArray
    jbyteArray byteArray = g_env->NewByteArray(str.size());
    // 将c++ string 底层的 c string (char *) copy 到 jbyteArray 中
    g_env->SetByteArrayRegion(byteArray, 0, str.size(), reinterpret_cast<const jbyte *>(str.c_str()));
    // 使用jbyteArray作为String构造函数的参数，创建java String对象并返回
    return g_env->NewObject(string_clazz, string_constructor, byteArray);
}

// Java string --> c string
const char *JniTools::javaStringToCString(const jobject &javaString) {
    // 获取java String Class对象
    jclass string_clazz = g_env->GetObjectClass(javaString);

    jmethodID string_length_method_id = g_env->GetMethodID(string_clazz, "length", "()I");
    if (nullptr == string_length_method_id) {
        ERROR_PRINT("获取java string length方法出错");
        exit(-1);
    }

    jint string_length = g_env->CallIntMethod(javaString, string_length_method_id);

    char* stringChars = new char[string_length];

    jmethodID string_charAt_method_id = g_env->GetMethodID(string_clazz, "charAt", "(I)C");
    if (nullptr == string_charAt_method_id) {
        ERROR_PRINT("获取java string charAt方法出错");
        exit(-1);
    }

    for (int i = 0; i < string_length; ++i) {
        jchar string_char = g_env->CallIntMethod(javaString, string_charAt_method_id, i);
        stringChars[i] = string_char;
    }

    return stringChars;
}

// c string (char *) --> c++ string
string *JniTools::charsToString(const char *str) {
    return new string(str);
}

// c string (char *) --> jstring (jni)
jstring JniTools::charsToJstring(const char *str) {
    return g_env->NewStringUTF(str);
}

// c string (char *) --> java string
jobject JniTools::charsToJavaString(const char* str) {
    string* s = charsToString(str);
    return stringToJavaString(*s);
}

// jstring（jni）--> java string
jobject JniTools::jstringToJavaString(jstring string, jboolean is_copy) {
    // jstring --> c string
    const char* cstring = JniTools::getCharsFromJString(string, is_copy);
    // c string --> java string
    return JniTools::charsToJavaString(cstring);
}

jmethodID JniTools::get_method_id(const char *class_name, const char *method_name, const char *descriptor_name) {
    // 获取HashMap类元信息
    jclass _clazz = g_env->FindClass(class_name);
    if (nullptr == _clazz) {
        ERROR_PRINT("获取%s clz出错", class_name)
        exit(-1);
    }

    // 获取方法
    jmethodID _method = g_env->GetMethodID(_clazz, method_name, descriptor_name);
    if (nullptr == _method) {
        ERROR_PRINT("获取%s方法出错", method_name);
        exit(-1);
    }

    return _method;
}

jmethodID JniTools::get_method_id(jclass clz, const char *method_name, const char *descriptor_name) {

    // 获取方法
    jmethodID _method = g_env->GetMethodID(clz, method_name, descriptor_name);
    if (nullptr == _method) {
        ERROR_PRINT("获取%s: %s方法出错", method_name, descriptor_name);
        exit(-1);
    }

    return _method;
}

jfieldID JniTools::get_field_id(const char *class_name, const char *field_name, const char *descriptor_name) {
    jclass _clazz = g_env->FindClass(class_name);
    if (nullptr == _clazz) {
        ERROR_PRINT("获取%s clz出错", class_name)
        exit(-1);
    }

    jfieldID _field = g_env->GetFieldID(_clazz, field_name, descriptor_name);
    if (nullptr == _field) {
        ERROR_PRINT("获取%s字段出错", field_name);
        exit(-1);
    }

    return _field;
}

jfieldID JniTools::get_field_id(jclass clz, const char *field_name, const char *descriptor_name) {

    // 获取方法
    jfieldID _field = g_env->GetFieldID(clz, field_name, descriptor_name);
    if (nullptr == _field) {
        ERROR_PRINT("获取%s: %s字段出错", field_name, descriptor_name);
        exit(-1);
    }

    return _field;
}

jclass JniTools::find_class(const char* class_name) {
    jclass _clazz = g_env->FindClass(class_name);
    if (nullptr == _clazz) {
        ERROR_PRINT("获取Class %s出错", class_name);
        exit(-1);
    }

    return _clazz;
}

jint JniTools::get_fd(jobject netObject, const char* name) {
    jclass _clz = g_env->GetObjectClass(netObject);

    jfieldID _field = JniTools::get_field_id(_clz, name, "I");

    return g_env->GetIntField(netObject, _field);
}

jstring JniTools::jint_to_jstring(jint num) {
    char s[32];
    sprintf(s, "%d", num);
    return charsToJstring(s);
}