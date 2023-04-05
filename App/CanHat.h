//
// Created by user on 30.03.2023.
//

#ifndef PROTOSSERVERCMAKE_CANHAT_H
#define PROTOSSERVERCMAKE_CANHAT_H

#include "protos_port.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <thread>

class CanHat: public Protos::Port{
public:
    CanHat();
    int Read(Protos::Packet*,int) override;
    bool Write(const Protos::Packet&) override;
    bool Open(const StringMap&) override;
    bool Close() override;
    bool IsOpen() const override;
    QString FormatError(int error) const override;
    std::optional<int> GetLastError() const override;
    static std::unique_ptr<Port> CreateCanHat();
    ~CanHat();
private:
    int canSocketFd;
    ifreq ifr;
    sockaddr_can addr;
    std::deque<Protos::Packet> RxQueue;
    bool isOpened = false;
    std::atomic<bool> threadFlag{false};
    std::thread Thread;
    static void ParseSocketCanPacket(const can_frame&,Protos::Packet&);
    bool closeConnection();
    void ReadThread();
};

#endif //PROTOSSERVERCMAKE_CANHAT_H
