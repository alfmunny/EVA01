#include "src/log.h"
#include "src/iomanager.h"
#include "src/timer.h"
#include "src/util.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <string.h>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

namespace eva01 {

static Logger::ptr g_logger = EVA_ROOT_LOGGER();

int sock = 0;

void test_fiber() { 
    EVA_LOG_INFO(g_logger) << "test_fiber sock=" << sock;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock, F_SETFL, O_NONBLOCK);

    sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "103.235.46.39", &addr.sin_addr.s_addr);

    if(!connect(sock, (const sockaddr*)&addr, sizeof(addr))) {
    } else if (errno == EINPROGRESS){
        EVA_LOG_INFO(g_logger) << "add event errno=" << errno << " " << strerror(errno);

        IOManager::GetThis()->addEvent(sock, IOManager::READ, [](){
            EVA_LOG_INFO(g_logger) << "read callback";
        });
        IOManager::GetThis()->addEvent(sock, IOManager::WRITE, [](){
            EVA_LOG_INFO(g_logger) << "write callback";
            IOManager::GetThis()->cancelEvent(sock, IOManager::READ);
            close(sock);
        });
    } else {
        EVA_LOG_INFO(g_logger) << "else" << errno << " " << strerror(errno);
    }
}

TEST_CASE("Test IOManager") {
    IOManager iom(2, "iom");
    iom.schedule(&test_fiber);

}

}
