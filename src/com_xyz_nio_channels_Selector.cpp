#include "../include/com_xyz_nio_channels_Selector.h"

extern JNIEnv* g_env;
/*
 * Class:     com_xyz_nio_channels_Selector
 * Method:    open
 * Signature: ()Lcom/xyz/nio/channels/Selector;
 */
JNIEXPORT jobject JNICALL Java_com_xyz_nio_channels_Selector_open
        (JNIEnv *env, jclass selectorClz) {
    // init
    if (g_env == nullptr) {
        g_env = env;
    }

    int epoll_fd = epoll_create(1);
    if (-1 == epoll_fd) {
        perror("create epoll fd failed");
        exit(-1);
    }

    jmethodID selectorConstructId = JniTools::get_method_id(selectorClz, "<init>", "(I)V");

    jobject selector = env->NewObject(selectorClz, selectorConstructId, epoll_fd);

    return selector;
}


/*
 * Class:     com_xyz_nio_channels_Selector
 * Method:    select
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_xyz_nio_channels_Selector_select
        (JNIEnv *env, jobject selector) {
    // epoll fd
    jint selectorEpollFd = JniTools::get_fd(selector, "epollFd");

    // set、set add function
    jclass selectorClz = env->GetObjectClass(selector);
    jfieldID selectorSelectedKeysId = JniTools::get_field_id(selectorClz, "selectedKeys", "Ljava/util/Set;");
    jobject selectorSelectedKeys = env->GetObjectField(selector, selectorSelectedKeysId);
    jmethodID setAddId = JniTools::get_method_id("java/util/Set", "add", "(Ljava/lang/Object;)Z");

    // map、map get function
    jfieldID selectorFdToKeyId = JniTools::get_field_id(selectorClz, "fdToKey", "Ljava/util/Map;");
    jobject selectorFdToKey = env->GetObjectField(selector, selectorFdToKeyId);
    jmethodID mapGetId = JniTools::get_method_id("java/util/Map", "get", "(Ljava/lang/Object;)Ljava/lang/Object;");

    // ServerSocket fd
    jfieldID selectorServerSocketId = JniTools::get_field_id(selectorClz, "serverSocketChannel", "Lcom/xyz/nio/channels/ServerSocketChannel;");
    jobject selectorServerSocket = env->GetObjectField(selector, selectorServerSocketId);
    jint serverSocketChannelServerSocketFd = JniTools::get_fd(selectorServerSocket, "serverSocketFd");

    // selectionKey
    jclass selectionKeyClz = JniTools::find_class("com/xyz/nio/channels/SelectionKey");
    jfieldID selectionReadyOpsId = JniTools::get_field_id(selectionKeyClz, "readyOps", "I");

    struct epoll_event ready_socket_event[MAX_EVENTS];

    int ready_socket_num = epoll_wait(selectorEpollFd, ready_socket_event, MAX_EVENTS, -1);

    if (0 == ready_socket_num) {
        return 0;
    }

    int ready_socket_fd;
    int ready_ops;
    for (int i = 0; i < ready_socket_num; ++i) {
        // accept
        if (serverSocketChannelServerSocketFd == ready_socket_event[i].data.fd) {
            ready_socket_fd = serverSocketChannelServerSocketFd;
            ready_ops = OP_ACCEPT;
        } else if (ready_socket_event[i].events & EPOLLIN) {
            ready_socket_fd = ready_socket_event[i].data.fd;
            ready_ops = OP_READ;
        } else if (ready_socket_event[i].events & EPOLLOUT) {
            ready_socket_fd = ready_socket_event[i].data.fd;
            ready_ops = OP_WRITE;
        } else {
            printf("unknown event\n");
            continue;
        }

        jobject selectionKey = env->CallObjectMethod(selectorFdToKey, mapGetId, JniTools::jint_to_jstring(ready_socket_fd));

        env->SetIntField(selectionKey, selectionReadyOpsId, ready_ops);

        env->CallBooleanMethod(selectorSelectedKeys, setAddId, selectionKey);
    }

    return ready_socket_num;
}
