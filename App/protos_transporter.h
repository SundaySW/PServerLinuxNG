
#ifndef PROTOSSERVERCMAKE_TRANSPORTER_H
#define PROTOSSERVERCMAKE_TRANSPORTER_H

#include "protos_port.h"
#include "protos_msg.h"
#include <mutex>
#include <memory>
#include <thread>
#include <vector>
#include <functional>
#include <qcoreevent.h>
#include <optional>
#include <condition_variable>
#include "CanHat.h"
#include "USBCANM2.h"

namespace Protos
{
	class Transporter
	{
	public:
		std::function<void(const char*, const Packet&, const QString&)> OnPacket;
		std::function<void(const QString&)> OnReadError;
		enum { PORT_EVENT = QEvent::User + 2 };

		~Transporter();
		bool IsStarted() const;
		void OnEvents();
		bool Restart(QString& error);
		void Stop();
		void Write(const Protos::Msg& msg);
	
	private:
		void Notify();
		void ThreadProc();
		bool WaitStop();
		bool WaitPortOpened(const std::chrono::milliseconds& timeout);

		PacketArray RxEvents;
		std::optional<int> ReadError = 0;

		struct Tx {
			Protos::Packet Packet;
			std::optional<int> Error;
		};
		std::vector<Tx> TxQueue, TxEvents;		
		
		using Guard = std::unique_lock<std::mutex>;
		std::mutex Mutex;
		std::unique_ptr<Protos::Port> Port;
		std::thread Thread;
		std::condition_variable Cv;
		bool StopWork = false;
		bool Posted = false;
	};
}

#endif //PROTOSSERVERCMAKE_TRANSPORTER_H