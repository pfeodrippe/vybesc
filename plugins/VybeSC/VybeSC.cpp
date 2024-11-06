// PluginVybeSC.cpp
// Paulo Feodrippe (pfeodrippe@gmail.com)

#include "SC_PlugIn.hpp"
#include "VybeSC.hpp"
#include <janet.h>
#include <sstream>
#include <dlfcn.h>
#include <iostream>

#include <jni.h>

static InterfaceTable* ft;

typedef int (*plugin_func)();

static JNIEnv *vybe_jenv;
static jobject vybe_fn;
static jmethodID vybe_invoke_method;
static jobject vybe_obj_1;
static jobject vybe_obj_2;
static jclass vybe_long_class;
static jmethodID vybe_init_long;
static jmethodID vybe_long_value;
static jclass Clojure;
static jclass ClojureIFn;
static jmethodID var_method;

static bool vybe_init_called = false;

jmethodID vybe_static_method(JNIEnv *env, jclass klass, const char* method_name, const char* method_sig) {
    jmethodID method_id = env->GetStaticMethodID(klass, method_name, method_sig);
    if (method_id == 0) {
        std::cout << " === ERROR when reading var === CLASS: " << klass << " || METHOD: " << method_id << std::flush;
        return NULL;
    }
    return method_id;
}

jmethodID vybe_method(JNIEnv *env, jclass klass, const char* method_name, const char* method_sig) {
    jmethodID method_id = env->GetMethodID(klass, method_name, method_sig);
    if (method_id == 0) {
        std::cout << " === ERROR when reading var === CLASS: " << klass << " || METHOD: " << method_id << std::flush;
        return NULL;
    }
    return method_id;
}

jobject vybe_eval(const char* eval_str) {
    jstring read_string_var_name = vybe_jenv->NewStringUTF("clojure.core/read-string");
    jobject vybe_read_string = vybe_jenv->CallStaticObjectMethod(Clojure, var_method, read_string_var_name);
    jmethodID invoke_method = vybe_method(vybe_jenv, ClojureIFn, "invoke", "(Ljava/lang/Object;)Ljava/lang/Object;");

    jstring eval_jstr = vybe_jenv->NewStringUTF(eval_str);
    return vybe_jenv->CallObjectMethod(vybe_read_string, invoke_method, eval_jstr);
}

// https://stackoverflow.com/questions/992836/how-to-access-the-java-method-in-a-c-application
// https://github.com/openjdk/jdk/blob/master/test/hotspot/gtest/gtestMain.cpp#L94
// https://www.iitk.ac.in/esc101/05Aug/tutorial/native1.1/implementing/method.html
void vybe_sc_init_jvm() {
    if (vybe_init_called) {
        /////////////////////// NREPL
        jmethodID invoke_method = vybe_method(vybe_jenv, ClojureIFn, "invoke", "(Ljava/lang/Object;)Ljava/lang/Object;");

        jstring read_string_var_name = vybe_jenv->NewStringUTF("clojure.core/read-string");
        jobject vybe_read_string = vybe_jenv->CallStaticObjectMethod(Clojure, var_method, read_string_var_name);
        jstring eval_var_name = vybe_jenv->NewStringUTF("clojure.core/eval");
        jobject vybe_eval_method = vybe_jenv->CallStaticObjectMethod(Clojure, var_method, eval_var_name);

        jstring eval_jstr = vybe_jenv->NewStringUTF("((requiring-resolve 'vybe.audio/-plugin))");
        jobject read_string_ret = vybe_jenv->CallObjectMethod(vybe_read_string, invoke_method, eval_jstr);
        jobject result = vybe_jenv->CallObjectMethod(vybe_eval_method, invoke_method, read_string_ret);

        std::cout << "\nRESULT (clj): " << result << "\n" << std::flush;

        long* vybe_p = (long*)(void*)(jlong)vybe_jenv->CallObjectMethod(result, vybe_long_value);
        vybe_p[0] -= 100;

        std::cout << "\nRESULT (clj vybe_p): " << vybe_p[0] << "\n" << std::flush;

        //////////////////////// MISC
        /* result = vybe_jenv->CallObjectMethod(vybe_fn, vybe_invoke_method, */
        /*                                      vybe_obj_1, vybe_obj_2); */
        /* std::cout << "\nRESULT: " <<  (jlong)vybe_jenv->CallObjectMethod(result, vybe_long_value) << "\n" << std::flush; */
        return;
    }
    vybe_init_called = true;

    JavaVM *vm;
    JavaVMInitArgs vm_args;

    JavaVMOption* options = new JavaVMOption[10];
    vm_args.nOptions = 6;
    vm_args.options = options;
    options[0].optionString = "-Djava.class.path=.";
    options[1].optionString = "-XX:-OmitStackTraceInFastThrow";
    options[2].optionString = "--enable-native-access=ALL-UNNAMED";
    options[3].optionString = "-Djava.library.path=/Users/pfeodrippe/dev/vybe/vybe_native";
    // options[4].optionString = "-XstartOnFirstThread";
    options[4].optionString = "-DVYBE_NREPL_PORT=7889";

    /* options[7].optionString = "clojure.main"; */
    /* options[8].optionString = "-m"; */
    /* options[9].optionString = "vybe.raylib"; */

    options[5].optionString = "-Djava.class.path=test:test-resources:/Users/pfeodrippe/dev/vybe/src:/Users/pfeodrippe/dev/vybe/resources:vybe_native:/Users/pfeodrippe/dev/vybe/target/classes:/Users/pfeodrippe/.m2/repository/aleph/aleph/0.8.1/aleph-0.8.1.jar:/Users/pfeodrippe/.m2/repository/babashka/process/0.5.22/process-0.5.22.jar:/Users/pfeodrippe/.m2/repository/camel-snake-kebab/camel-snake-kebab/0.4.3/camel-snake-kebab-0.4.3.jar:/Users/pfeodrippe/.m2/repository/cider/cider-nrepl/0.49.3/cider-nrepl-0.49.3.jar:/Users/pfeodrippe/.m2/repository/com/clojure-goes-fast/clj-java-decompiler/0.3.4/clj-java-decompiler-0.3.4.jar:/Users/pfeodrippe/.m2/repository/com/nextjournal/beholder/1.0.2/beholder-1.0.2.jar:/Users/pfeodrippe/.m2/repository/djblue/portal/0.48.0/portal-0.48.0.jar:/Users/pfeodrippe/.m2/repository/io/github/clojure/tools.build/0.10.5/tools.build-0.10.5.jar:/Users/pfeodrippe/.m2/repository/io/github/nextjournal/clerk/0.17.1102/clerk-0.17.1102.jar:/Users/pfeodrippe/.m2/repository/lambdaisland/deep-diff2/2.11.216/deep-diff2-2.11.216.jar:/Users/pfeodrippe/.m2/repository/meta-merge/meta-merge/1.0.0/meta-merge-1.0.0.jar:/Users/pfeodrippe/.m2/repository/metosin/jsonista/0.3.7/jsonista-0.3.7.jar:/Users/pfeodrippe/.m2/repository/metosin/malli/0.11.0/malli-0.11.0.jar:/Users/pfeodrippe/.m2/repository/nrepl/nrepl/1.1.1/nrepl-1.1.1.jar:/Users/pfeodrippe/.m2/repository/nubank/matcher-combinators/3.9.1/matcher-combinators-3.9.1.jar:/Users/pfeodrippe/.m2/repository/org/clojure/clojure/1.11.1/clojure-1.11.1.jar:/Users/pfeodrippe/.gitlibs/libs/overtone/overtone/f5d4a82828b026bd470b9c2312ff488f565bbf98/src:/Users/pfeodrippe/.m2/repository/potemkin/potemkin/0.4.7/potemkin-0.4.7.jar:/Users/pfeodrippe/.m2/repository/io/netty/netty-codec/4.1.111.Final/netty-codec-4.1.111.Final.jar:/Users/pfeodrippe/.m2/repository/io/netty/netty-codec-http/4.1.111.Final/netty-codec-http-4.1.111.Final.jar:/Users/pfeodrippe/.m2/repository/io/netty/netty-codec-http2/4.1.111.Final/netty-codec-http2-4.1.111.Final.jar:/Users/pfeodrippe/.m2/repository/io/netty/netty-handler/4.1.111.Final/netty-handler-4.1.111.Final.jar:/Users/pfeodrippe/.m2/repository/io/netty/netty-handler-proxy/4.1.111.Final/netty-handler-proxy-4.1.111.Final.jar:/Users/pfeodrippe/.m2/repository/io/netty/netty-resolver/4.1.111.Final/netty-resolver-4.1.111.Final.jar:/Users/pfeodrippe/.m2/repository/io/netty/netty-resolver-dns/4.1.111.Final/netty-resolver-dns-4.1.111.Final.jar:/Users/pfeodrippe/.m2/repository/io/netty/netty-transport/4.1.111.Final/netty-transport-4.1.111.Final.jar:/Users/pfeodrippe/.m2/repository/io/netty/netty-transport-native-epoll/4.1.111.Final/netty-transport-native-epoll-4.1.111.Final-linux-aarch_64.jar:/Users/pfeodrippe/.m2/repository/io/netty/netty-transport-native-epoll/4.1.111.Final/netty-transport-native-epoll-4.1.111.Final-linux-x86_64.jar:/Users/pfeodrippe/.m2/repository/io/netty/netty-transport-native-kqueue/4.1.111.Final/netty-transport-native-kqueue-4.1.111.Final-osx-x86_64.jar:/Users/pfeodrippe/.m2/repository/io/netty/incubator/netty-incubator-transport-native-io_uring/0.0.25.Final/netty-incubator-transport-native-io_uring-0.0.25.Final-linux-aarch_64.jar:/Users/pfeodrippe/.m2/repository/io/netty/incubator/netty-incubator-transport-native-io_uring/0.0.25.Final/netty-incubator-transport-native-io_uring-0.0.25.Final-linux-x86_64.jar:/Users/pfeodrippe/.m2/repository/manifold/manifold/0.4.3/manifold-0.4.3.jar:/Users/pfeodrippe/.m2/repository/org/clj-commons/byte-streams/0.3.4/byte-streams-0.3.4.jar:/Users/pfeodrippe/.m2/repository/org/clj-commons/dirigiste/1.0.4/dirigiste-1.0.4.jar:/Users/pfeodrippe/.m2/repository/org/clj-commons/primitive-math/1.0.1/primitive-math-1.0.1.jar:/Users/pfeodrippe/.m2/repository/org/clojure/tools.logging/1.3.0/tools.logging-1.3.0.jar:/Users/pfeodrippe/.m2/repository/cider/orchard/0.26.2/orchard-0.26.2.jar:/Users/pfeodrippe/.m2/repository/mx/cider/logjam/0.3.0/logjam-0.3.0.jar:/Users/pfeodrippe/.m2/repository/org/bitbucket/mstrobel/procyon-compilertools/0.6.0/procyon-compilertools-0.6.0.jar:/Users/pfeodrippe/.m2/repository/io/methvin/directory-watcher/0.17.3/directory-watcher-0.17.3.jar:/Users/pfeodrippe/.m2/repository/com/cognitect/transit-clj/1.0.333/transit-clj-1.0.333.jar:/Users/pfeodrippe/.m2/repository/com/cognitect/transit-cljs/0.8.280/transit-cljs-0.8.280.jar:/Users/pfeodrippe/.m2/repository/com/google/code/gson/gson/2.10.1/gson-2.10.1.jar:/Users/pfeodrippe/.m2/repository/org/clojure/tools.deps/0.19.1411/tools.deps-0.19.1411.jar:/Users/pfeodrippe/.m2/repository/org/clojure/tools.namespace/1.5.0/tools.namespace-1.5.0.jar:/Users/pfeodrippe/.m2/repository/org/slf4j/slf4j-nop/1.7.36/slf4j-nop-1.7.36.jar:/Users/pfeodrippe/.m2/repository/babashka/fs/0.5.22/fs-0.5.22.jar:/Users/pfeodrippe/.m2/repository/borkdude/edamame/1.4.24/edamame-1.4.24.jar:/Users/pfeodrippe/.m2/repository/com/pngencoder/pngencoder/0.13.1/pngencoder-0.13.1.jar:/Users/pfeodrippe/.m2/repository/com/taoensso/nippy/3.4.2/nippy-3.4.2.jar:/Users/pfeodrippe/.m2/repository/hiccup/hiccup/2.0.0-RC3/hiccup-2.0.0-RC3.jar:/Users/pfeodrippe/.m2/repository/http-kit/http-kit/2.8.0/http-kit-2.8.0.jar:/Users/pfeodrippe/.m2/repository/io/github/babashka/sci.nrepl/0.0.2/sci.nrepl-0.0.2.jar:/Users/pfeodrippe/.m2/repository/io/github/nextjournal/markdown/0.6.157/markdown-0.6.157.jar:/Users/pfeodrippe/.m2/repository/juji/editscript/0.6.4/editscript-0.6.4.jar:/Users/pfeodrippe/.m2/repository/mvxcvi/multiformats/0.3.107/multiformats-0.3.107.jar:/Users/pfeodrippe/.m2/repository/org/clojure/tools.analyzer/1.1.0/tools.analyzer-1.1.0.jar:/Users/pfeodrippe/.m2/repository/org/flatland/ordered/1.15.12/ordered-1.15.12.jar:/Users/pfeodrippe/.m2/repository/rewrite-clj/rewrite-clj/1.1.45/rewrite-clj-1.1.45.jar:/Users/pfeodrippe/.m2/repository/weavejester/dependency/0.2.1/dependency-0.2.1.jar:/Users/pfeodrippe/.m2/repository/fipp/fipp/0.6.26/fipp-0.6.26.jar:/Users/pfeodrippe/.m2/repository/lambdaisland/clj-diff/1.4.78/clj-diff-1.4.78.jar:/Users/pfeodrippe/.m2/repository/mvxcvi/arrangement/2.1.0/arrangement-2.1.0.jar:/Users/pfeodrippe/.m2/repository/org/clojure/core.rrb-vector/0.1.2/core.rrb-vector-0.1.2.jar:/Users/pfeodrippe/.m2/repository/com/fasterxml/jackson/core/jackson-databind/2.14.1/jackson-databind-2.14.1.jar:/Users/pfeodrippe/.m2/repository/com/fasterxml/jackson/datatype/jackson-datatype-jsr310/2.14.1/jackson-datatype-jsr310-2.14.1.jar:/Users/pfeodrippe/.m2/repository/borkdude/dynaload/0.3.5/dynaload-0.3.5.jar:/Users/pfeodrippe/.m2/repository/org/clojure/test.check/1.1.1/test.check-1.1.1.jar:/Users/pfeodrippe/.m2/repository/org/clojure/math.combinatorics/0.2.0/math.combinatorics-0.2.0.jar:/Users/pfeodrippe/.m2/repository/org/clojure/core.specs.alpha/0.2.62/core.specs.alpha-0.2.62.jar:/Users/pfeodrippe/.m2/repository/org/clojure/spec.alpha/0.3.218/spec.alpha-0.3.218.jar:/Users/pfeodrippe/.m2/repository/casa/squid/jack/0.2.12/jack-0.2.12.jar:/Users/pfeodrippe/.m2/repository/clj-glob/clj-glob/1.0.0/clj-glob-1.0.0.jar:/Users/pfeodrippe/.m2/repository/commons-net/commons-net/3.10.0/commons-net-3.10.0.jar:/Users/pfeodrippe/.m2/repository/javax/jmdns/jmdns/3.4.1/jmdns-3.4.1.jar:/Users/pfeodrippe/.m2/repository/org/clojure/data.json/2.5.0/data.json-2.5.0.jar:/Users/pfeodrippe/.m2/repository/overtone/at-at/1.4.65/at-at-1.4.65.jar:/Users/pfeodrippe/.m2/repository/io/netty/netty-buffer/4.1.111.Final/netty-buffer-4.1.111.Final.jar:/Users/pfeodrippe/.m2/repository/io/netty/netty-common/4.1.111.Final/netty-common-4.1.111.Final.jar:/Users/pfeodrippe/.m2/repository/io/netty/netty-transport-native-unix-common/4.1.111.Final/netty-transport-native-unix-common-4.1.111.Final.jar:/Users/pfeodrippe/.m2/repository/io/netty/netty-codec-socks/4.1.111.Final/netty-codec-socks-4.1.111.Final.jar:/Users/pfeodrippe/.m2/repository/io/netty/netty-codec-dns/4.1.111.Final/netty-codec-dns-4.1.111.Final.jar:/Users/pfeodrippe/.m2/repository/io/netty/netty-transport-classes-epoll/4.1.111.Final/netty-transport-classes-epoll-4.1.111.Final.jar:/Users/pfeodrippe/.m2/repository/io/netty/netty-transport-classes-kqueue/4.1.111.Final/netty-transport-classes-kqueue-4.1.111.Final.jar:/Users/pfeodrippe/.m2/repository/io/netty/incubator/netty-incubator-transport-classes-io_uring/0.0.25.Final/netty-incubator-transport-classes-io_uring-0.0.25.Final.jar:/Users/pfeodrippe/.m2/repository/riddley/riddley/0.2.0/riddley-0.2.0.jar:/Users/pfeodrippe/.m2/repository/org/bitbucket/mstrobel/procyon-core/0.6.0/procyon-core-0.6.0.jar:/Users/pfeodrippe/.m2/repository/org/slf4j/slf4j-api/1.7.36/slf4j-api-1.7.36.jar:/Users/pfeodrippe/.m2/repository/com/cognitect/transit-java/1.0.371/transit-java-1.0.371.jar:/Users/pfeodrippe/.m2/repository/com/cognitect/transit-js/0.8.874/transit-js-0.8.874.jar:/Users/pfeodrippe/.m2/repository/com/cognitect/aws/api/0.8.686/api-0.8.686.jar:/Users/pfeodrippe/.m2/repository/com/cognitect/aws/endpoints/1.1.12.626/endpoints-1.1.12.626.jar:/Users/pfeodrippe/.m2/repository/com/cognitect/aws/s3/848.2.1413.0/s3-848.2.1413.0.jar:/Users/pfeodrippe/.m2/repository/javax/inject/javax.inject/1/javax.inject-1.jar:/Users/pfeodrippe/.m2/repository/org/apache/maven/maven-core/3.8.8/maven-core-3.8.8.jar:/Users/pfeodrippe/.m2/repository/org/apache/maven/maven-resolver-provider/3.8.8/maven-resolver-provider-3.8.8.jar:/Users/pfeodrippe/.m2/repository/org/apache/maven/resolver/maven-resolver-api/1.8.2/maven-resolver-api-1.8.2.jar:/Users/pfeodrippe/.m2/repository/org/apache/maven/resolver/maven-resolver-connector-basic/1.8.2/maven-resolver-connector-basic-1.8.2.jar:/Users/pfeodrippe/.m2/repository/org/apache/maven/resolver/maven-resolver-impl/1.8.2/maven-resolver-impl-1.8.2.jar:/Users/pfeodrippe/.m2/repository/org/apache/maven/resolver/maven-resolver-spi/1.8.2/maven-resolver-spi-1.8.2.jar:/Users/pfeodrippe/.m2/repository/org/apache/maven/resolver/maven-resolver-transport-file/1.8.2/maven-resolver-transport-file-1.8.2.jar:/Users/pfeodrippe/.m2/repository/org/apache/maven/resolver/maven-resolver-transport-http/1.8.2/maven-resolver-transport-http-1.8.2.jar:/Users/pfeodrippe/.m2/repository/org/apache/maven/resolver/maven-resolver-util/1.8.2/maven-resolver-util-1.8.2.jar:/Users/pfeodrippe/.m2/repository/org/clojure/data.xml/0.2.0-alpha9/data.xml-0.2.0-alpha9.jar:/Users/pfeodrippe/.m2/repository/org/clojure/tools.cli/1.1.230/tools.cli-1.1.230.jar:/Users/pfeodrippe/.m2/repository/org/clojure/tools.gitlibs/2.5.197/tools.gitlibs-2.5.197.jar:/Users/pfeodrippe/.m2/repository/org/eclipse/jetty/jetty-client/9.4.53.v20231009/jetty-client-9.4.53.v20231009.jar:/Users/pfeodrippe/.m2/repository/org/eclipse/jetty/jetty-http/9.4.53.v20231009/jetty-http-9.4.53.v20231009.jar:/Users/pfeodrippe/.m2/repository/org/clojure/java.classpath/1.1.0/java.classpath-1.1.0.jar:/Users/pfeodrippe/.m2/repository/com/taoensso/encore/3.112.0/encore-3.112.0.jar:/Users/pfeodrippe/.m2/repository/io/airlift/aircompressor/0.27/aircompressor-0.27.jar:/Users/pfeodrippe/.m2/repository/org/clojure/tools.reader/1.4.2/tools.reader-1.4.2.jar:/Users/pfeodrippe/.m2/repository/org/tukaani/xz/1.9/xz-1.9.jar:/Users/pfeodrippe/.m2/repository/nrepl/bencode/1.1.0/bencode-1.1.0.jar:/Users/pfeodrippe/.m2/repository/org/commonmark/commonmark/0.23.0/commonmark-0.23.0.jar:/Users/pfeodrippe/.m2/repository/org/commonmark/commonmark-ext-autolink/0.23.0/commonmark-ext-autolink-0.23.0.jar:/Users/pfeodrippe/.m2/repository/org/commonmark/commonmark-ext-footnotes/0.23.0/commonmark-ext-footnotes-0.23.0.jar:/Users/pfeodrippe/.m2/repository/org/commonmark/commonmark-ext-gfm-strikethrough/0.23.0/commonmark-ext-gfm-strikethrough-0.23.0.jar:/Users/pfeodrippe/.m2/repository/org/commonmark/commonmark-ext-gfm-tables/0.23.0/commonmark-ext-gfm-tables-0.23.0.jar:/Users/pfeodrippe/.m2/repository/org/commonmark/commonmark-ext-task-list-items/0.23.0/commonmark-ext-task-list-items-0.23.0.jar:/Users/pfeodrippe/.m2/repository/commons-codec/commons-codec/1.15/commons-codec-1.15.jar:/Users/pfeodrippe/.m2/repository/mvxcvi/alphabase/2.1.1/alphabase-2.1.1.jar:/Users/pfeodrippe/.m2/repository/com/fasterxml/jackson/core/jackson-annotations/2.14.1/jackson-annotations-2.14.1.jar:/Users/pfeodrippe/.m2/repository/org/jaudiolibs/jnajack/1.4.0/jnajack-1.4.0.jar:/Users/pfeodrippe/.m2/repository/com/fasterxml/jackson/core/jackson-core/2.14.2/jackson-core-2.14.2.jar:/Users/pfeodrippe/.m2/repository/javax/xml/bind/jaxb-api/2.4.0-b180830.0359/jaxb-api-2.4.0-b180830.0359.jar:/Users/pfeodrippe/.m2/repository/org/msgpack/msgpack/0.6.12/msgpack-0.6.12.jar:/Users/pfeodrippe/.m2/repository/com/cognitect/http-client/1.0.125/http-client-1.0.125.jar:/Users/pfeodrippe/.m2/repository/org/clojure/core.async/1.6.673/core.async-1.6.673.jar:/Users/pfeodrippe/.m2/repository/com/google/inject/guice/4.2.2/guice-4.2.2-no_aop.jar:/Users/pfeodrippe/.m2/repository/org/apache/maven/maven-artifact/3.8.8/maven-artifact-3.8.8.jar:/Users/pfeodrippe/.m2/repository/org/apache/maven/maven-builder-support/3.8.8/maven-builder-support-3.8.8.jar:/Users/pfeodrippe/.m2/repository/org/apache/maven/maven-model/3.8.8/maven-model-3.8.8.jar:/Users/pfeodrippe/.m2/repository/org/apache/maven/maven-model-builder/3.8.8/maven-model-builder-3.8.8.jar:/Users/pfeodrippe/.m2/repository/org/apache/maven/maven-plugin-api/3.8.8/maven-plugin-api-3.8.8.jar:/Users/pfeodrippe/.m2/repository/org/apache/maven/maven-repository-metadata/3.8.8/maven-repository-metadata-3.8.8.jar:/Users/pfeodrippe/.m2/repository/org/apache/maven/maven-settings/3.8.8/maven-settings-3.8.8.jar:/Users/pfeodrippe/.m2/repository/org/apache/maven/maven-settings-builder/3.8.8/maven-settings-builder-3.8.8.jar:/Users/pfeodrippe/.m2/repository/org/apache/maven/shared/maven-shared-utils/3.3.4/maven-shared-utils-3.3.4.jar:/Users/pfeodrippe/.m2/repository/org/codehaus/plexus/plexus-classworlds/2.6.0/plexus-classworlds-2.6.0.jar:/Users/pfeodrippe/.m2/repository/org/codehaus/plexus/plexus-component-annotations/2.1.0/plexus-component-annotations-2.1.0.jar:/Users/pfeodrippe/.m2/repository/org/codehaus/plexus/plexus-interpolation/1.26/plexus-interpolation-1.26.jar:/Users/pfeodrippe/.m2/repository/org/eclipse/sisu/org.eclipse.sisu.inject/0.3.5/org.eclipse.sisu.inject-0.3.5.jar:/Users/pfeodrippe/.m2/repository/org/eclipse/sisu/org.eclipse.sisu.plexus/0.3.5/org.eclipse.sisu.plexus-0.3.5.jar:/Users/pfeodrippe/.m2/repository/javax/annotation/javax.annotation-api/1.3.2/javax.annotation-api-1.3.2.jar:/Users/pfeodrippe/.m2/repository/org/apache/commons/commons-lang3/3.12.0/commons-lang3-3.12.0.jar:/Users/pfeodrippe/.m2/repository/org/apache/maven/resolver/maven-resolver-named-locks/1.8.2/maven-resolver-named-locks-1.8.2.jar:/Users/pfeodrippe/.m2/repository/org/apache/httpcomponents/httpclient/4.5.13/httpclient-4.5.13.jar:/Users/pfeodrippe/.m2/repository/org/apache/httpcomponents/httpcore/4.4.15/httpcore-4.4.15.jar:/Users/pfeodrippe/.m2/repository/org/slf4j/jcl-over-slf4j/1.7.36/jcl-over-slf4j-1.7.36.jar:/Users/pfeodrippe/.m2/repository/org/eclipse/jetty/jetty-io/9.4.53.v20231009/jetty-io-9.4.53.v20231009.jar:/Users/pfeodrippe/.m2/repository/org/eclipse/jetty/jetty-util/9.4.53.v20231009/jetty-util-9.4.53.v20231009.jar:/Users/pfeodrippe/.m2/repository/com/taoensso/truss/1.11.0/truss-1.11.0.jar:/Users/pfeodrippe/.m2/repository/org/nibor/autolink/autolink/0.11.0/autolink-0.11.0.jar:/Users/pfeodrippe/.m2/repository/net/java/dev/jna/jna/5.15.0/jna-5.15.0.jar:/Users/pfeodrippe/.m2/repository/javax/activation/javax.activation-api/1.2.0/javax.activation-api-1.2.0.jar:/Users/pfeodrippe/.m2/repository/com/googlecode/json-simple/json-simple/1.1.1/json-simple-1.1.1.jar:/Users/pfeodrippe/.m2/repository/org/javassist/javassist/3.18.1-GA/javassist-3.18.1-GA.jar:/Users/pfeodrippe/.m2/repository/org/clojure/tools.analyzer.jvm/1.2.2/tools.analyzer.jvm-1.2.2.jar:/Users/pfeodrippe/.m2/repository/aopalliance/aopalliance/1.0/aopalliance-1.0.jar:/Users/pfeodrippe/.m2/repository/org/codehaus/plexus/plexus-sec-dispatcher/2.0/plexus-sec-dispatcher-2.0.jar:/Users/pfeodrippe/.m2/repository/org/clojure/core.memoize/1.0.253/core.memoize-1.0.253.jar:/Users/pfeodrippe/.m2/repository/org/ow2/asm/asm/9.2/asm-9.2.jar:/Users/pfeodrippe/.m2/repository/org/codehaus/plexus/plexus-cipher/2.0/plexus-cipher-2.0.jar:/Users/pfeodrippe/.m2/repository/org/codehaus/plexus/plexus-utils/3.4.1/plexus-utils-3.4.1.jar:/Users/pfeodrippe/.m2/repository/org/clojure/core.cache/1.0.225/core.cache-1.0.225.jar:/Users/pfeodrippe/.m2/repository/org/clojure/data.priority-map/1.1.0/data.priority-map-1.1.0.jar";

    vm_args.version = JNI_VERSION_21;
    vm_args.ignoreUnrecognized = false;

    // Construct a VM
    jint res = JNI_CreateJavaVM(&vm, (void **)&vybe_jenv, &vm_args);

    if (res == JNI_OK) {
        std::cout << "JVM OK \\o/ --=nn\n" << std::flush;
    }
    else {
        std::cout << "\n\nJVM ERRORRRR --=-= " << res << "\n\n\n" << std::flush;
        return;
    }

    // Construct a String
    //jstring jstr = vybe_jenv->NewStringUTF("Hello World");

    // First get the class that contains the method you need to call
    //jclass clazz = vybe_jenv->FindClass("java/lang/String");

    // Get the method that you want to call
    //jmethodID to_lower = vybe_jenv->GetMethodID(clazz, "toLowerCase", "()Ljava/lang/String;");

    // Call the method on the object
    //jobject result = vybe_jenv->CallObjectMethod(jstr, to_lower);

    // Get a C-style string
    //const char* str = vybe_jenv->GetStringUTFChars((jstring) result, NULL);

    // std::cout << str << std::flush;

    // Clean up
    //vybe_jenv->ReleaseStringUTFChars(jstr, str);

    // CUSTOM
    Clojure = vybe_jenv->FindClass("clojure/java/api/Clojure");
    var_method = vybe_static_method(vybe_jenv, Clojure,
                                    "var", "(Ljava/lang/Object;)Lclojure/lang/IFn;");
    jstring var_name = vybe_jenv->NewStringUTF("clojure.core/+");
    vybe_fn = vybe_jenv->CallStaticObjectMethod(Clojure, var_method, var_name);

    ClojureIFn = vybe_jenv->FindClass("clojure/lang/IFn");
    vybe_invoke_method = vybe_method(vybe_jenv, ClojureIFn,
                                     "invoke", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
    if (!vybe_invoke_method) {
        std::cout << "\nINVOKE ERROR: " << vybe_invoke_method << "\n" << std::flush;
        return;
    }

    vybe_long_class = vybe_jenv->FindClass("java/lang/Long");
    vybe_init_long = vybe_jenv->GetMethodID(vybe_long_class, "<init>", "(J)V");
    vybe_long_value = vybe_jenv->GetMethodID(vybe_long_class, "longValue", "()J");
    vybe_obj_1 = vybe_jenv->NewObject(vybe_long_class, vybe_init_long, (jlong) 123);
    vybe_obj_2 = vybe_jenv->NewObject(vybe_long_class, vybe_init_long, (jlong) 451);
    //jlong result = vybe_jenv->CallLongMethod(plus_fn, invoke_method, newLongObj1);
    jobject result = vybe_jenv->CallObjectMethod(vybe_fn, vybe_invoke_method,
                                                 vybe_obj_1, vybe_obj_2);

    //std::cout << "\nRESULT: " << vybe_jenv->GetStringUTFChars((jstring) result, NULL) << "\n" << std::flush;
    //std::cout << "\nRESULT: " << (long)result << "\n" << std::flush;
    std::cout << "\nRESULT: " <<  (jlong)vybe_jenv->CallObjectMethod(result, vybe_long_value) << "\n" << std::flush;

    /////////////////////// EVAL
    //result = vybe_eval("(+ 1 45)");

    jmethodID invoke_method = vybe_method(vybe_jenv, ClojureIFn, "invoke", "(Ljava/lang/Object;)Ljava/lang/Object;");

    jstring read_string_var_name = vybe_jenv->NewStringUTF("clojure.core/read-string");
    jobject vybe_read_string = vybe_jenv->CallStaticObjectMethod(Clojure, var_method, read_string_var_name);
    jstring eval_var_name = vybe_jenv->NewStringUTF("clojure.core/eval");
    jobject vybe_eval_method = vybe_jenv->CallStaticObjectMethod(Clojure, var_method, eval_var_name);

    jstring eval_jstr = vybe_jenv->NewStringUTF("(+ 1 45)");
    jobject read_string_ret = vybe_jenv->CallObjectMethod(vybe_read_string, invoke_method, eval_jstr);
    result = vybe_jenv->CallObjectMethod(vybe_eval_method, invoke_method, read_string_ret);

    std::cout << "\nRESULT: " <<  (jlong)vybe_jenv->CallObjectMethod(result, vybe_long_value) << "\n" << std::flush;

    /////////////////////// NREPL
    eval_jstr = vybe_jenv->NewStringUTF("((requiring-resolve 'vybe.raylib/start-nrepl!))");
    read_string_ret = vybe_jenv->CallObjectMethod(vybe_read_string, invoke_method, eval_jstr);
    result = vybe_jenv->CallObjectMethod(vybe_eval_method, invoke_method, read_string_ret);
}

// Shutdown the VM.
// vm->DestroyJavaVM();


// cmake --build . --config Debug && cp VybeSC_scsynth.scx ~/dev/games/vybe_native/macos/universal/supercollider/Resources/plugins && cp VybeSC_scsynth.scx ~/dev/vybe/vybe_native/macos/universal/supercollider/Resources/plugins && cp VybeSC_scsynth.scx ~/dev/vybe/sonic-pi/prebuilt/macos/universal/supercollider/Resources/plugins/VybeSC_scsynth.scx

// static bool ssss = false;

namespace VybeSC {

    VybeSC::VybeSC() {
        mCalcFunc = make_calc_function<VybeSC, &VybeSC::next>();
        next(1);

        vybe_sc_init_jvm();

        // janet_init();
        // jenv = janet_core_env(NULL);

        // vybe_sc_init_jvm();

        // Janet out;
        // int status = janet_dostring(jenv, "(eval-string (slurp \"/Users/pfeodrippe/dev/vybe/code.edn\"))", "main", &out);

        // Janet mainfun;

        //janet_resolve(jenv, janet_csymbol("my-fn"), &mainfun);
        //JanetCFunction my_fn = janet_unwrap_function(mainfun);

        // janet_resolve(jenv, janet_csymbol("olha"), &mainfun);
        // JanetCFunction my_fn = janet_unwrap_cfunction(mainfun);
        // my_fn(0, NULL);

        // fiber = janet_fiber(my_fn, 64, 0, NULL);
        // janet_gcroot(janet_wrap_fiber(fiber));
        // fiber->env = jenv;
        // Janet out2;
        // janet_pcall(my_fn, 0, NULL, &out2, &fiber);

        //Janet out2 = my_fn(0, NULL);

        // std::ostringstream stringStream;
        // // stringStream << "(spit \"/Users/pfeodrippe/dev/vybe/file.txt\" string(" << status << " \" \" " << janet_type(out) << "))";
        // stringStream << "(spit \"/Users/pfeodrippe/dev/vybe/file.txt\" \"" << status << " " << "\")";
        // std::string copyOfStr = stringStream.str();
        // janet_dostring(jenv, copyOfStr.c_str(), "main", &out);

        /////////////////////////// dlopen
        // void* handle = dlopen("/Users/pfeodrippe/dev/vybesc/libttt.dylib", RTLD_NOW);

        // if (!handle) {
        //     std::cout << "dlopen ERROR --=-=-=-=" << std::flush;
        //     dlclose(handle);
        // } else {
        //     std::cout << "dlopen GOOD --=-=-=-=" << std::flush;
        // }

        // plugin_func f = (plugin_func)dlsym(handle, "olha");
        // if (f == NULL) {
        //     fprintf(stderr, "Could not find plugin_func: %s\n", dlerror());
        // }
        // printf("Calling plugin\n");
        // int ret = (*f)();
        // printf("Plugin returned %d\n", ret);
        // if (dlclose(handle) != 0) {
        //     fprintf(stderr, "Could not close plugin: %s\n", dlerror());
        // }

        // std::ostringstream stringStream;
        // stringStream << "(spit \"/Users/pfeodrippe/dev/vybe/file.txt\" \"" << ret << " " << "\")";
        // std::string copyOfStr = stringStream.str();
        // janet_dostring(jenv, copyOfStr.c_str(), "main", NULL);
    }

    void VybeSC::next(int nSamples) {

        /* if(!ssss) { */
        /*     ssss = true; */
        /*     vybe_jenv->CallObjectMethod(vybe_fn, vybe_invoke_method, */
        /*                                 vybe_obj_1, vybe_obj_2); */
        /* } */

        //Janet jout;
        //janet_pcall(my_fn, 0, NULL, NULL, &fiber);
        //float myconst = janet_unwrap_number(jout);

        //Janet jout;
        //janet_dostring(jenv, "(eval-string (slurp \"/Users/pfeodrippe/dev/vybe/code.edn\"))", "main", &jout);
        //float myconst = janet_unwrap_number(jout);

        // Audio rate input
        const float* input = in(0);

        // Control rate parameter: gain.
        const float gain = 1.0f - in0(1);
        //const float gain = myconst - in0(1);

        // Output buffer
        float* outbuf = out(0);

        // simple gain function
        for (int i = 0; i < nSamples; ++i) {
            outbuf[i] = input[i] * gain;
        }
    }

} // namespace VybeSC

PluginLoad(VybeSCUGens) {
    // Plugin magic
    ft = inTable;
    registerUnit<VybeSC::VybeSC>(ft, "VybeSC", false);
}
