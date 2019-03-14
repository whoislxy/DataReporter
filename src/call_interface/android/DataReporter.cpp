#include <jni.h>
#include <string>
#include "JniHelper.h"
#include "AndroidUtil.h"
#include "Reporter.h"

static jlong
MakeReporter(JNIEnv *env, jobject obj, jstring uuid, jstring cachePath, jobject reportImp) {
    std::string uuidCStr = AndroidUtil::fromJavaString(env, uuid);
    std::string cachePathCstr = AndroidUtil::fromJavaString(env, cachePath);
    std::shared_ptr<JObject> jCallback(new JObject(reportImp));
    future::Reporter *reporter = new future::Reporter(uuidCStr, cachePathCstr,
                                                      [jCallback](int64_t key,
                                                                  std::list<std::shared_ptr<future::CacheItem> > &data) {
                                                          jobjectArray javaData = AndroidUtil::getEnv()->NewObjectArray(
                                                                  data.size(),
                                                                  AndroidUtil::CLASS_java_String.j(),
                                                                  NULL);
                                                          JNIEnv *currentEnv = AndroidUtil::getEnv();
                                                          int i = 0;
                                                          for (std::list<std::shared_ptr<future::CacheItem> >::iterator iter = data.begin();
                                                               iter != data.end(); iter++) {
                                                              std::string dataCstr;
                                                              dataCstr.append(
                                                                      (const char *) (*iter)->pbEncodeItem.data.GetBegin(),
                                                                      (*iter)->pbEncodeItem.data.Length());
                                                              jstring dataJstr = AndroidUtil::createJavaString(
                                                                      currentEnv, dataCstr);
                                                              currentEnv->SetObjectArrayElement(
                                                                      javaData, i, dataJstr);
                                                              i++;
                                                              currentEnv->DeleteLocalRef(dataJstr);
                                                          }
                                                          AndroidUtil::Method_upload->call(
                                                                  (jlong) key, javaData,
                                                                  jCallback->GetObj());
                                                          currentEnv->DeleteLocalRef(javaData);
                                                      });
    return (jlong) reporter;
}

static void SetReportCount(JNIEnv *env, jobject obj, jlong nativeReporter, jint count) {
    future::Reporter *reporter = reinterpret_cast<future::Reporter *>(nativeReporter);
    if (reporter == NULL) {
        return;
    }
    reporter->SetUploadItemSize((int) count);
}

static void SetFileMaxSize(JNIEnv *env, jobject obj, jlong nativeReporter, jint fileMaxSize) {
    future::Reporter *reporter = reinterpret_cast<future::Reporter *>(nativeReporter);
    if (reporter == NULL) {
        return;
    }
    reporter->SetFileMaxSize((int) fileMaxSize);
}

static void SetExpiredTime(JNIEnv *env, jobject obj, jlong nativeReporter, jlong expiredTime) {
    future::Reporter *reporter = reinterpret_cast<future::Reporter *>(nativeReporter);
    if (reporter == NULL) {
        return;
    }
    reporter->SetExpiredTime((std::int64_t) expiredTime);
}

static void
SetReportingInterval(JNIEnv *env, jobject obj, jlong nativeReporter, jlong reportingInterval) {
    future::Reporter *reporter = reinterpret_cast<future::Reporter *>(nativeReporter);
    if (reporter == NULL) {
        return;
    }
    reporter->SetReportingInterval((std::int64_t) reportingInterval);
}

static void Start(JNIEnv *env, jobject obj, jlong nativeReporter) {
    future::Reporter *reporter = reinterpret_cast<future::Reporter *>(nativeReporter);
    if (reporter == NULL) {
        return;
    }
    reporter->Start();
}

static void ReaWaken(JNIEnv *env, jobject obj, jlong nativeReporter) {
    future::Reporter *reporter = reinterpret_cast<future::Reporter *>(nativeReporter);
    if (reporter == NULL) {
        return;
    }
    reporter->ReaWaken();
}

static void Push(JNIEnv *env, jobject obj, jlong nativeReporter, jstring data) {
    future::Reporter *reporter = reinterpret_cast<future::Reporter *>(nativeReporter);
    if (reporter == NULL) {
        return;
    }
    std::string dataCstr = AndroidUtil::fromJavaString(env, data);
    reporter->Push(dataCstr);
}

static void UploadSucess(JNIEnv *env, jobject obj, jlong nativeReporter, jlong key) {
    future::Reporter *reporter = reinterpret_cast<future::Reporter *>(nativeReporter);
    if (reporter == NULL) {
        return;
    }
    reporter->UoloadSuccess(key);
}

static void UploadFailed(JNIEnv *env, jobject obj, jlong nativeReporter, jlong key) {
    future::Reporter *reporter = reinterpret_cast<future::Reporter *>(nativeReporter);
    if (reporter == NULL) {
        return;
    }
    reporter->UploadFailed(key);
}

static void ReleaseReporter(JNIEnv *env, jobject obj, jlong nativeReporter) {
    future::Reporter *reporter = reinterpret_cast<future::Reporter *>(nativeReporter);
    if (reporter == NULL) {
        return;
    }
    future::Reporter::Destroy(reporter);
}

static JNINativeMethod gJavaDataReporterMethods[] = {
        {"makeReporter",         "(Ljava/lang/String;Ljava/lang/String;Lcom/iget/datareporter/IReport;)J", (void *) MakeReporter},
        {"setReportCount",       "(JI)V",                                                                  (void *) SetReportCount},
        {"setFileMaxSize",       "(JI)V",                                                                  (void *) SetFileMaxSize},
        {"setExpiredTime",       "(JJ)V",                                                                  (void *) SetExpiredTime},
        {"setReportingInterval", "(JJ)V",                                                                  (void *) SetReportingInterval},
        {"start",                "(J)V",                                                                   (void *) Start},
        {"reaWaken",             "(J)V",                                                                   (void *) ReaWaken},
        {"push",                 "(JLjava/lang/String;)V",                                                 (void *) Push},
        {"uploadSucess",         "(JJ)V",                                                                  (void *) UploadSucess},
        {"uploadFailed",         "(JJ)V",                                                                  (void *) UploadFailed},
        {"releaseReporter",      "(J)V",                                                                   (void *) ReleaseReporter},
};

int registerDataReporter(JNIEnv *env) {
    const char *reporterClassName = "com/iget/datareporter/DataReporter";
    return registerNativeMethods(env, reporterClassName,
                                 gJavaDataReporterMethods, NELEM(gJavaDataReporterMethods));
}
