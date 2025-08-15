#include "ChatServer.h"

int main() {
    ChatServer server;
    server.run("127.0.0.1", 12345);
    return 0;
}