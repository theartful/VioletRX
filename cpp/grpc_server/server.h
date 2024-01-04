#include "receiver.pb.h"
#include "receiver.grpc.pb.h"

class Server {


private:
    Receiver::Rx::AsyncService* service;
};
