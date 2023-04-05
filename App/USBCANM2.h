//
// Created by user on 03.04.2023.
//

#ifndef PROTOSSERVERCMAKE_USBCANM2_H
#define PROTOSSERVERCMAKE_USBCANM2_H

//#include "can_api.h"
//#include "usb_can_m2.h"
#include "protos_port.h"
#include <memory>
#include <qserialport.h>
#include <qregexp.h>
#include <packet_utils.h>

using namespace Protos;

class USBCANM2 : public Protos::Port
{
    public:
        USBCANM2()
                : COM(new QSerialPort())
        {
            QObject::connect(COM, &QSerialPort::readyRead, [this]() { HandleReadyRead(); });
            QObject::connect(COM, &QSerialPort::errorOccurred, [this](QSerialPort::SerialPortError e)
            {	OnPortErrorOccured(e); });
        }
        ~USBCANM2() {
            Close();
            delete COM;
        }
        bool Close() override;
        QString FormatError(int error) const override;
        std::optional<int> GetLastError() const override;
        bool IsOpen() const override;
        bool Open(const StringMap& params) override;
        int Read(Protos::Packet* packets, int count) override;
        bool Write(const Protos::Packet& packet) override;
        bool ParsePacket(char type, const char* buffer, int len, Protos::Packet& packet);
        static std::unique_ptr<Port> CreateUSBCANM2();

    private:
        void HandleReadyRead();
        void OnPortErrorOccured(QSerialPort::SerialPortError e);
        QSerialPort* COM;
        std::deque<Protos::Packet> RxQueue; ///< in packets queue

        inline bool closeConnection();
};

namespace {
    enum class CMD2 : char {
        NOPE = 0, READ, WRITE, OPEN, CLOSE
    };

    QString GetCANBitrateCmd(int rate_kbs) {
        QString cmd;
        switch (rate_kbs) {
            case 1000: cmd = "S8";
                break;
            case 800: cmd = "S7";
                break;
            case 500: cmd = "S6";
                break;
            case 250: cmd = "S5";
                break;
            case 125: cmd = "S4";
                break;
            case 100: cmd = "S3";
                break;
            case 50: cmd = "S2";
                break;
            case 20: cmd = "S1";
                break;
        }
        cmd.append("\r");
        return cmd;
    }
}

#endif //PROTOSSERVERCMAKE_USBCANM2_H
