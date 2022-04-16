#include "../include/com_xyz_nio_channels_SocketChannel.h"

/*
 * Class:     com_xyz_nio_channels_SocketChannel
 * Method:    register0
 * Signature: (Lcom/xyz/nio/channels/Selector;I)Lcom/xyz/nio/channels/SelectionKey;
 */
JNIEXPORT jobject JNICALL Java_com_xyz_nio_channels_SocketChannel_register0
        (JNIEnv *env, jobject socketChannel, jobject selector, jint ops) {
    jint socketChannelSocketFd = JniTools::get_fd(socketChannel, "socketFd");

    jint selectorEpollFd = JniTools::get_fd(selector, "epollFd");

    // set、set add function
    jclass selectorClz = env->GetObjectClass(selector);
    jfieldID selectorKeysId = JniTools::get_field_id(selectorClz, "keys", "Ljava/util/Set;");
    jobject selectorKeys = env->GetObjectField(selector, selectorKeysId);
    jmethodID setAddId = JniTools::get_method_id("java/util/Set", "add", "(Ljava/lang/Object;)Z");

    // map、map put function
    jfieldID selectorFdToKeyId = JniTools::get_field_id(selectorClz, "fdToKey", "Ljava/util/Map;");
    jobject selectorFdToKey = env->GetObjectField(selector, selectorFdToKeyId);
    jmethodID mapPutId = JniTools::get_method_id("java/util/Map", "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");

    jclass selectionKeyClz = JniTools::find_class("com/xyz/nio/channels/SelectionKey");
    jmethodID selectionKeyConstructId = JniTools::get_method_id(selectionKeyClz, "<init>",
                                                                "(Lcom/xyz/nio/channels/Selector;Lcom/xyz/nio/channels/Channel;I)V");
    jobject selectionKey = nullptr;

    struct epoll_event event;
    switch (ops) {
        case OP_READ:
            {
                event.events = EPOLLIN | EPOLLET;
                event.data.fd = socketChannelSocketFd;

                if (-1 == epoll_ctl(selectorEpollFd, EPOLL_CTL_ADD, socketChannelSocketFd, &event)) {
                    perror("epoll ctl failed");
                    exit(-1);
                }

                selectionKey = env->NewObject(selectionKeyClz, selectionKeyConstructId, selector, socketChannel, OP_READ);

                // 将selectionKey加入selector的keys set中
                env->CallBooleanMethod(selectorKeys, setAddId, selectionKey);

                // 将socket fd和selectionKey的关系加入selector的fdToKey map中
                env->CallObjectMethod(selectorFdToKey, mapPutId, JniTools::jint_to_jstring(socketChannelSocketFd), selectionKey);
            }
            break;
        case OP_WRITE:
            {
                event.events = EPOLLOUT | EPOLLET;
                event.data.fd = socketChannelSocketFd;

                if (-1 == epoll_ctl(selectorEpollFd, EPOLL_CTL_ADD, socketChannelSocketFd, &event)) {
                    perror("epoll ctl failed");
                    exit(-1);
                }

                selectionKey = env->NewObject(selectionKeyClz, selectionKeyConstructId, selector, socketChannel, OP_WRITE);

                // 将selectionKey加入selector的keys set中
                env->CallBooleanMethod(selectorKeys, setAddId, selectionKey);

                // 将socket fd和selectionKey的关系加入selector的fdToKey map中
                env->CallObjectMethod(selectorFdToKey, mapPutId, JniTools::jint_to_jstring(socketChannelSocketFd), selectionKey);
            }
            break;
        case OP_ACCEPT:
            break;
    }

    return selectionKey;
}

/*
 * Class:     com_xyz_nio_channels_SocketChannel
 * Method:    read0
 * Signature: ([B)I
 */
JNIEXPORT jint JNICALL Java_com_xyz_nio_channels_SocketChannel_read0
        (JNIEnv *env, jobject socketChannel, jbyteArray buff) {
    jint socketChannelSocketFd = JniTools::get_fd(socketChannel, "socketFd");

    jclass socketChannelClz = env->GetObjectClass(socketChannel);
    jfieldID socketChannelSelectorId = JniTools::get_field_id(socketChannelClz, "selector", "Lcom/xyz/nio/channels/Selector;");
    jobject selector = env->GetObjectField(socketChannel, socketChannelSelectorId);
    jint selectorEpollFd = JniTools::get_fd(selector, "epollFd");


    struct epoll_event event;
    event.events = EPOLLOUT | EPOLLIN | EPOLLET;
    event.data.fd = socketChannelSocketFd;

    byte cbuff[READ_BUFF_SIZE] = {0};
    int read_size = read(socketChannelSocketFd, cbuff, sizeof(cbuff));
    if (-1 == read_size) {
        perror("read failed");
        exit(-1);
    }  else if (0 == read_size) {
        INFO_PRINT("client quit");
        epoll_ctl(selectorEpollFd, EPOLL_CTL_DEL, socketChannelSocketFd, &event);
        close(socketChannelSocketFd);
        return 0;
    } else if ( 0 < read_size) {
        env->SetByteArrayRegion(buff, 0, read_size, reinterpret_cast<const jbyte *>(cbuff));
        printf("client say: %s\n", cbuff);
        return read_size;
    }
}

/*
 * Class:     com_xyz_nio_channels_SocketChannel
 * Method:    configureBlocking
 * Signature: (Z)Lcom/xyz/nio/channels/SocketChannel;
 */
JNIEXPORT jobject JNICALL Java_com_xyz_nio_channels_SocketChannel_configureBlocking
        (JNIEnv *env, jobject socketChannel, jboolean block) {
    jint socketChannelSocketFd = JniTools::get_fd(socketChannel, "socketFd");

    int flags = fcntl(socketChannelSocketFd, F_GETFL, 0);
    if (-1 == fcntl(socketChannelSocketFd, F_SETFL, flags | O_NONBLOCK)) {
        perror("set server socket non block failed");
        exit(-1);
    }

    return socketChannel;
}