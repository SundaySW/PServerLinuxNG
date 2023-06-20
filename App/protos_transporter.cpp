#include "protos_transporter.h"
#include "server_conf.h"
#include "QCoreApplication"

extern QObject* GetServerApp();
using namespace std::chrono_literals;

namespace Protos
{
	Transporter::~Transporter()
	{
		Stop();
	}

	void Transporter::Notify()
	{
		if (!Posted)
		{
            QCoreApplication::postEvent(GetServerApp(), new QEvent((QEvent::Type)PORT_EVENT));
			Posted = true;
		}
	}

	void Transporter::Stop()
	{
		if (Thread.joinable())
		{
			Guard lock(Mutex);
			StopWork = true;
			lock.unlock();
			Cv.notify_all();
			Thread.join();
			std::thread t;
			Thread.swap(t);
		}

		StopWork = false;
		ReadError.reset();
		RxEvents.clear();
		TxEvents.clear();
		TxQueue.clear();

		//if (Port)
		//	Port->Close();
	}

	bool Transporter::IsStarted() const
	{
		return (Port && Port->IsOpen() && Thread.joinable());
	}

	void Transporter::OnEvents()
	{
		Mutex.lock();
		auto rxq = std::move(RxEvents);
		auto txq = std::move(TxEvents);
		auto readError = ReadError;
		ReadError.reset();
		Posted = false;
		Mutex.unlock();

		for (auto& e : rxq)
			OnPacket("rx", e, {});

		//if (readError)
		//	OnReadError(Port->FormatError(readError.value()));

		for (auto& e : txq)
			OnPacket("tx", e.Packet, e.Error ? Port->FormatError(e.Error.value()) : QString());
	}

	bool Transporter::Restart(QString& error)
	{
		Stop();
		QString e;
		Conf::Read((QCoreApplication::applicationDirPath() + "/protos_server.conf").toStdString(), e);
	/*	if (Conf::Port.Type == Protos::PORT_MOXACP602EI)
		{
			Port = CreateMoxaCP602();
			Port->Open(Conf::Port.Params);
		}
		else if (Conf::Port.Type == Protos::PORT_CAN_USB_M)
		{
			Port = CreateUSBCANM2();
			Port->Open(Conf::Port.Params);
		}
		else
		{
			error = QStringLiteral("���������������� ��� �����.");
			return false;
		}

		if (!Port->IsOpen())
		{
			auto err = Port->GetLastError();
			if (err)
				error = Port->FormatError(*err);
			return false;
		}*/

		//{
		/*auto port = new PICAPort(this);
		const char* greeting = "#PROTOS_SERVER\n";
		port->Open(Conf::Port.Params);
		port->Write(greeting, strlen(greeting));
		Port.reset(port);*/
		//}
		//return Port->IsOpen();
		Thread = std::thread([this]()
		{
			ThreadProc();
		});
		return WaitPortOpened(500ms);
	}

	void Transporter::ThreadProc()
	{
		std::optional<QEventLoop> loop;
        switch (Conf::Port.Type) {
            case Protos::PORT_CAN_USB_M:
                loop.emplace();
                Port = USBCANM2::CreateUSBCANM2();
                Port->Open(Conf::Port.Params);
                break;
            case Protos::PORT_CAN_HAT:
                loop.emplace();
                Port = CanHat::CreateCanHat();
                Port->Open(Conf::Port.Params);
                break;
            default:
                return;
        }
		if (!Port->IsOpen())
		{
			return;
			//auto err = Port->GetLastError();
			//if (err)
			//	error = Port->FormatError(*err);
			//return false;
		}

		while (!WaitStop())
		{
			if (loop)
				loop->processEvents();

			std::optional<int> readError;

			size_t readCount = 0;
			Packet readPackets[256];
            int arr_size = sizeof(readPackets)/sizeof(readPackets[0]);
			for (; readCount < arr_size; readCount++)
			{
				if (!Port->Read(&readPackets[readCount], 1))
				{
					readError = Port->GetLastError();
					break;
				}
			}

			Mutex.lock();
			auto txq = std::move(TxQueue);
			Mutex.unlock();

			for (auto& tx : txq)
			{
				if (!Port->Write(tx.Packet))
					tx.Error = Port->GetLastError();
			}

			if (readCount || !txq.empty() || readError)
			{
				Guard lock(Mutex);
				RxEvents.insert(RxEvents.end(), readPackets, readPackets + readCount);
				TxEvents.insert(TxEvents.end(), txq.begin(), txq.end());
				if(readError)
					ReadError = readError;
				Notify();
			}
		}
		Port->Close();
	}

	bool Transporter::WaitStop()
	{
		Guard lock(Mutex);
		bool stopped = Cv.wait_for(lock, std::chrono::milliseconds(5), 
			[this](){return StopWork == true;});
		return stopped;
	}

	bool Transporter::WaitPortOpened(const std::chrono::milliseconds& timeout)
	{
		Guard lock(Mutex);
		bool opened = Cv.wait_for(lock, timeout,
			[this]()->bool 
			{
				return Port && Port->IsOpen(); 
			});
		return opened;
	}

	void Transporter::Write(const Protos::Msg& msg)
	{
		auto packets = msg.ToPackets();
		if (packets.empty()) return;
		Guard lock(Mutex);
		auto& tx = TxQueue.emplace_back();
		tx.Packet = packets[0];
	}

}//namespace protos
