#include "WlCallJava.h"

WlCallJava::WlCallJava(_JavaVM *javaVM, JNIEnv *env, jobject *obj) {
    this->javaVM = javaVM;
    this->jniEnv = env;
    this->jobj = *obj;
    this->jobj = env->NewGlobalRef(jobj);

    jclass jlz = jniEnv->GetObjectClass(jobj);
    if(!jlz) {
        return;
    }

    jmid_prepared = env->GetMethodID(jlz, "onCallPrepared", "()V");
    jmid_load = env->GetMethodID(jlz, "onCallLoad", "(Z)V");
    jmid_timeinfo = env->GetMethodID(jlz, "onCallTimeInfo", "(II)V");

}

void WlCallJava::onCallPrepared(int type) {

}

void WlCallJava::onCallLoad(int type, bool load) {

}

void WlCallJava::onCallTimeInfo(int type, int curr, int total) {

}

WlCallJava::~WlCallJava() {
    
}