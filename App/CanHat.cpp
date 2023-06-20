//
// Created by user on 30.03.2023.
//

#include <memory>
#include <QThread>
#include <iostream>
#include "CanHat.h"

CanHat::CanHat()
        : ifr({}),addr({})
{}

bool CanHat::Open(const Protos::Port::StringMap &params) {
    int ret;
//    struct Param {
//        QString Name;
//        int Value;
//    };
//    Param canBitrate_bps{ "CANBitrate",	500000 };
//    QByteArray cmd = QString("sudo ip link set can0 up type can bitrate %1").arg(canBitrate_bps.Value).toLocal8Bit();
//    system(cmd.data());
//    system("sudo ip link set can0 up type can bitrate 500000");
//    system("sudo ifconfig can0 txqueuelen 65536");

    //Create socket
    canSocketFd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (canSocketFd < 0) {
        perror("socket PF_CAN failed");
        return false;
    }

    //Specify can0 device
    strcpy(ifr.ifr_name, "can0");
    ret = ioctl(canSocketFd, SIOCGIFINDEX, &ifr);
    if (ret < 0) {
        perror("ioctl failed");
        return false;
    }

    //Bind the socket to can0
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    ret = bind(canSocketFd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        perror("bind failed");
        return false;
    }

//    struct can_filter rfilter[1];
//    rfilter[0].can_id = 0x1FFFFFFF;
//    rfilter[0].can_mask = CAN_EFF_MASK;
//    setsockopt(canSocketFd, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &rfilter, sizeof(rfilter));
//    int fd_frames = 1;
//    setsockopt(canSocketFd, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &fd_frames, sizeof(fd_frames));

    isOpened = true;
    threadFlag.store(true);
    Thread = std::thread([this](){ ReadThread();});
    return true;
}

bool CanHat::Close() {
    return closeConnection();
}

bool CanHat::closeConnection(){
    if (Thread.joinable())
    {
        threadFlag.store(false);
        Thread.join();
    }
    RxQueue.clear();

    if(isOpened){
        close(canSocketFd);
//        system("sudo ip link set can0 down");
        isOpened = false;
    }
    return true;
}

int CanHat::Read(Protos::Packet *packets, int count) {
    int n = std::min(count, (int)RxQueue.size());
    for (int i = 0; i != n; i++) {
        packets[i] = RxQueue.front();
        RxQueue.pop_front();
    }
    return n;
}

void CanHat::ReadThread(){
    int bytes;
    can_frame frame{};
    while (threadFlag.load()){
        memset(&frame, 0, sizeof(struct can_frame));
        bytes = read(canSocketFd, &frame, sizeof(frame));
        if(!bytes) continue;
        if(bytes < 0){
            //todo handle error
            continue;
        }
        Protos::Packet packet;
        ParseSocketCanPacket(frame, packet);
        RxQueue.push_back(packet);
    }
}

bool CanHat::Write(const Protos::Packet &packet) {
    can_frame send_frame{};
    memset(&send_frame, 0, sizeof(struct can_frame));
    send_frame.can_id = (packet.ID0.Byte | packet.Src<<8 | (packet.Dst<<16)| (packet.ID3.Byte << 24))
                        | 0x80000000; //31bit should be set to 1 to set extended CAN ID
    send_frame.can_dlc = packet.Dlc;
    for(int i=0; i<packet.Dlc; i++)
        send_frame.data[i] = packet.DataBuffer[i];
    int bytes = write(canSocketFd, &send_frame, sizeof(send_frame));
    if(bytes != sizeof(send_frame)) return false;
    return true;
}

void CanHat::ParseSocketCanPacket(const can_frame& frame, Protos::Packet& packet)
{
    union frame_id{
        canid_t id;
        struct{
            unsigned char b[4];
        }bytes;
    };
    frame_id frameId{};
    frameId.id = frame.can_id;
    packet.ID3.Byte = frameId.bytes.b[3] & 0x1F;
    packet.Dst      = frameId.bytes.b[2];
    packet.Src      = frameId.bytes.b[1];
    packet.ID0.Byte = frameId.bytes.b[0];
    packet.Dlc = frame.can_dlc;
    for(int i=0; i<packet.Dlc;i++)
        packet.DataBuffer[i] = frame.data[i];
//    timestamp
//    timeval tv{};
//    ioctl(canSocketFd, SIOCGSTAMP, &tv);
}

QString CanHat::FormatError(int error) const {
    return{};
}

std::optional<int> CanHat::GetLastError() const {
    return{};
}

bool CanHat::IsOpen() const {
    return isOpened;
}

CanHat::~CanHat() {
    closeConnection();
}

std::unique_ptr<Protos::Port> CanHat::CreateCanHat()
{
    return std::unique_ptr<Protos::Port>(new CanHat());
}