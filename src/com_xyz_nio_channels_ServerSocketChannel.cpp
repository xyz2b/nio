#include "../include/com_xyz_nio_channels_ServerSocketChannel.h"

JNIEnv* g_env;

/*
 * Class:     com_xyz_nio_channels_ServerSocketChannel
 * Method:    open
 * Signature: (I)Lcom/xyz/nio/channels/ServerSocketChannel;
 */
JNIEXPORT jobject JNICALL Java_com_xyz_nio_channels_ServerSocketChannel_open
        (JNIEnv *env, jclass serverSocketChannelClz) {
    // init
    if (g_env == nullptr) {
        g_env = env;
    }

    if (-1 == sighandler::signal_sig_int()) {
        perror("signal sig int failed");
        exit(-1);
    }

    int server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (-1 == server_sock) {
        perror("create server socket failed");
        exit(-1);
    }
    INFO_PRINT("server sock fd: %d\n", server_sock);

    int opt = 1;
    if (-1 == setsockopt(server_sock, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("set socket opt SO_REUSEPORT failed");
        exit(-1);
    }

    // 获取构造方法
    jmethodID serverSocketChannelConstructId = JniTools::get_method_id(serverSocketChannelClz, "<init>", "()V");

    // 创建对象
    jobject serverSocketChannel = env->NewObject(serverSocketChannelClz, serverSocketChannelConstructId);

    // 获取属性信息
    jfieldID serverSocketChannelServerSocketFdId = JniTools::get_field_id(serverSocketChannelClz, "serverSocketFd", "I");

    // 赋值（非静态属性值在对象中）
    env->SetIntField(serverSocketChannel, serverSocketChannelServerSocketFdId, server_sock);

    return serverSocketChannel;
}

/*
 * Class:     com_xyz_nio_channels_ServerSocketChannel
 * Method:    bind
 * Signature: (Lcom/xyz/nio/channels/InetAddress;)Lcom/xyz/nio/channels/ServerSocketChannel;
 */
JNIEXPORT jobject JNICALL Java_com_xyz_nio_channels_ServerSocketChannel_bind
        (JNIEnv *env, jobject serverSocketChannel, jobject inetAddress) {

    jint serverSocketChannelServerSocketFd = JniTools::get_fd(serverSocketChannel, "serverSocketFd");

    // 获取clz
    jclass inetAddressClz = env->GetObjectClass(inetAddress);

    // 获取属性信息
    jfieldID inetAddressIpId = JniTools::get_field_id(inetAddressClz, "ip", "Ljava/lang/String;");

    // 读取属性值（非静态属性值在对象中）
    jstring inetAddressIp = static_cast<jstring>(env->GetObjectField(inetAddress, inetAddressIpId));

    // 获取属性信息
    jfieldID inetAddressPortFieldId = JniTools::get_field_id(inetAddressClz, "port", "I");

    // 读取属性值（非静态属性值在对象中）
    jint inetAddressPort = env->GetIntField(inetAddress, inetAddressPortFieldId);

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(JniTools::getCharsFromJString(inetAddressIp, false));  // JniTools::javaStringToCString(inetAddressIp)
    server_addr.sin_port = htons(inetAddressPort);

    int status;
    status = bind(serverSocketChannelServerSocketFd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (0 != status) {
        perror("bind failed");
        exit(status);
    }

    status = listen(serverSocketChannelServerSocketFd, 10);
    if (-1 == status) {
        perror("listen failed");
        exit(-1);
    }

    return serverSocketChannel;
}

/*
 * Class:     com_xyz_nio_channels_ServerSocketChannel
 * Method:    configureBlocking
 * Signature: (Z)Lcom/xyz/nio/channels/ServerSocketChannel;
 */
JNIEXPORT jobject JNICALL Java_com_xyz_nio_channels_ServerSocketChannel_configureBlocking
        (JNIEnv *env, jobject serverSocketChannel, jboolean block) {
    jint serverSocketChannelServerSocketFd = JniTools::get_fd(serverSocketChannel, "serverSocketFd");

    int flags = fcntl(serverSocketChannelServerSocketFd, F_GETFL, 0);
    if (-1 == fcntl(serverSocketChannelServerSocketFd, F_SETFL, flags | O_NONBLOCK)) {
        perror("set server socket non block failed");
        exit(-1);
    }

    return serverSocketChannel;
}

/*
 * Class:     com_xyz_nio_channels_ServerSocketChannel
 * Method:    accept
 * Signature: ()Lcom/xyz/nio/channels/SocketChannel;
 */
JNIEXPORT jobject JNICALL Java_com_xyz_nio_channels_ServerSocketChannel_accept
        (JNIEnv *env, jobject serverSocketChannel) {
    jint serverSocketChannelServerSocketFd = JniTools::get_fd(serverSocketChannel, "serverSocketFd");

    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    memset(&client_addr, 0, client_addr_len);

    int client_sock = accept(serverSocketChannelServerSocketFd, (struct sockaddr*)&client_addr, &client_addr_len);
    if (client_sock < 0) {
        perror("accept failed");
        exit(-1);
    }

    int client_port = htons(client_addr.sin_port);

    char client_ip[16];
    memset(client_ip, 0, sizeof(client_ip));
    inet_ntop(AF_INET, &(client_addr.sin_addr.s_addr), client_ip, sizeof(client_ip));
    jstring client_ip_string = JniTools::charsToJstring(client_ip);

    // 创建inetAddress对象
    jclass inetAddressClz = JniTools::find_class("com/xyz/nio/channels/InetAddress");

    jmethodID inetAddressConstructId = JniTools::get_method_id(inetAddressClz, "<init>", "(Ljava/lang/String;I)V");

    jobject inetAddress = env->NewObject(inetAddressClz, inetAddressConstructId, client_ip_string, client_port);


    // 创建SocketChannel对象
    jclass socketChannelClz = JniTools::find_class("com/xyz/nio/channels/SocketChannel");

    jmethodID socketChannelConstructId = JniTools::get_method_id(socketChannelClz, "<init>", "(ILcom/xyz/nio/channels/InetAddress;)V");

    jobject socketChannel = env->NewObject(socketChannelClz, socketChannelConstructId, client_sock, inetAddress);

    return socketChannel;
}

/*
 * Class:     com_xyz_nio_channels_ServerSocketChannel
 * Method:    register0
 * Signature: (Lcom/xyz/nio/channels/Selector;I)Lcom/xyz/nio/channels/SelectionKey;
 */
JNIEXPORT jobject JNICALL Java_com_xyz_nio_channels_ServerSocketChannel_register0
        (JNIEnv *env, jobject serverSocketChannel, jobject selector, jint ops) {

    jint serverSocketChannelServerSocketFd = JniTools::get_fd(serverSocketChannel, "serverSocketFd");

    jint selectorEpollFd = JniTools::get_fd(selector, "epollFd");

    jclass selectorClz = env->GetObjectClass(selector);
    // set、set add function
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
            break;
        case OP_WRITE:
            break;
        case OP_ACCEPT:
            {
                event.events = EPOLLIN | EPOLLET;
                event.data.fd = serverSocketChannelServerSocketFd;

                if (-1 == epoll_ctl(selectorEpollFd, EPOLL_CTL_ADD, serverSocketChannelServerSocketFd, &event)) {
                    perror("epoll ctl failed");
                    exit(-1);
                }

                selectionKey = env->NewObject(selectionKeyClz, selectionKeyConstructId, selector, serverSocketChannel, OP_ACCEPT);

                // 将selectionKey加入selector的keys set中
                env->CallBooleanMethod(selectorKeys, setAddId, selectionKey);
                INFO_PRINT("add selectionKey to keys success");

                // 将socket fd和selectionKey的关系加入selector的 fdToKey map中
                env->CallObjectMethod(selectorFdToKey, mapPutId, JniTools::jint_to_jstring(serverSocketChannelServerSocketFd), selectionKey);
                INFO_PRINT("add selectionKey to fdToKey success");
            }
            break;
    }

    return selectionKey;
}
