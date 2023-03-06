#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "src/hook.h"
#include "src/iomanager.h"
#include "src/log.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

eva01::Logger::ptr g_logger = EVA_ROOT_LOGGER();

void test_sock() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "110.242.68.66", &addr.sin_addr.s_addr);

    EVA_LOG_INFO(g_logger) << "begin connect";
    int rt = connect(sock, (const sockaddr*)&addr, sizeof(addr));
    EVA_LOG_INFO(g_logger) << "connect rt=" << rt << " errno=" << errno;

    if(rt) {
        return;
    }

    const char data[] = "GET / HTTP/1.0\r\n\r\n";
    rt = send(sock, data, sizeof(data), 0);
    EVA_LOG_INFO(g_logger) << "send rt=" << rt << " errno=" << errno;

    if(rt <= 0) {
        return;
    }

    std::string buff;
    buff.resize(4096);

    rt = recv(sock, &buff[0], buff.size(), 0);
    EVA_LOG_INFO(g_logger) << "recv rt=" << rt << " errno=" << errno;

    if(rt <= 0) {
        return;
    }

    buff.resize(rt);
    EVA_LOG_INFO(g_logger) << buff;
}


TEST_CASE("Test Hook in scheudler") {
    eva01::IOManager iom(1);
    iom.schedule([]() {
            sleep(2);
            EVA_LOG_INFO(g_logger) << "sleep 2";
            });

    iom.schedule([]() {
            sleep(3);
            EVA_LOG_INFO(g_logger) << "sleep 3";
            });
    EVA_LOG_INFO(g_logger) << "test_sleep";
}

TEST_CASE("Test Hook with socket") {
    eva01::IOManager iom(1);
    iom.schedule(test_sock);
}
