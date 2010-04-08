#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include "RemoteFrequencySource.h"
#include "FPGA_page.h"

using namespace std;

RemoteFrequencySource::RemoteFrequencySource(double dMinFreq, double dMaxFreq,
                                             double dMaxRampRate, const std::string& name,
                                             const std::string &server_name,
                                             unsigned short port) :
	SweepingFrequencySource(dMinFreq, dMaxFreq, dMaxRampRate, name),
	host_address(QString(server_name.c_str())),
	server_name(server_name),
	port(port),
	tcp_lock(),
	current_rfs_lock()
{
	qRegisterMetaType<rfs_state>("rfs_state");
	cout << "packet size: " << sizeof(freq_control_packet) << endl;
}

RemoteFrequencySource::~RemoteFrequencySource()
{
	quit();
	wait();
}

void RemoteFrequencySource::slot_connected()
{
	get_remote_state_if_connected();
}

void RemoteFrequencySource::run()
{
	QObject::connect(&tcp_sock, SIGNAL(connected()),
	                 this, SLOT(slot_connected()),
	                 Qt::QueuedConnection);

	exec();
}

void RemoteFrequencySource::slot_get_state()
{
	get_remote_state_if_connected();
}

void RemoteFrequencySource::slot_set_state(rfs_state rfs)
{
	SendPacket(rfs, freq_control_packet::SET_STATE);
}

void RemoteFrequencySource::slot_shiftVisFrequency(double dShift)
{
	ShiftFrequency(dShift);
}

void RemoteFrequencySource::slot_newErrSig(double err_sig)
{
	newErrSig(err_sig);
}

void RemoteFrequencySource::slot_connect()
{
	connect();
}

void RemoteFrequencySource::slot_clearHistory()
{
	ClearHistory();
}

void RemoteFrequencySource::connect()
{
	tcp_sock.connectToHost(host_address, port);
}

std::string RemoteFrequencySource::getStatusText()
{
	return getTcpStatusText(tcp_sock.state());
}

void RemoteFrequencySource::getCurrentRFS(rfs_state* rfs)
{
	QReadLocker rfs_lock(&current_rfs_lock);

	*rfs = current_rfs;
}

//! sweep to a new frequency
void RemoteFrequencySource::SetDeviceFrequency(double dNewFreq)
{
	if ( IsValidFrequency(dNewFreq) )
	{
		rfs_state rfs;
		getCurrentRFS(&rfs);
		rfs.freq_or_delta = dNewFreq;
		SendPacket( rfs, freq_control_packet::SET_STATE );
	}
}

//! shift the frequency.
void RemoteFrequencySource::ShiftFrequency(double dShift)
{
	rfs_state rfs;

	rfs.freq_or_delta = dShift;
	SendPacket(rfs, freq_control_packet::SHIFT_FREQ);
}

void RemoteFrequencySource::newErrSig(double err_sig)
{
	rfs_state rfs;

	rfs.freq_or_delta = err_sig;
	SendPacket(rfs, freq_control_packet::NEW_ERR_SIG);
}

// set ramp rate
void RemoteFrequencySource::ClearHistory()
{
	rfs_state rfs;

	SendPacket( rfs, freq_control_packet::CLEAR_HISTORY );
}


void RemoteFrequencySource::stayCurrent()
{
	if ( CurrentTime_s() - GetLastUpdateTime() < 10 )
		get_remote_state_if_connected();
}

// return current frequency as estimated by the software if we got an update during the last second
// otherwise ask the remote source about the current frequency
double RemoteFrequencySource::GetCurrentDeviceFrequency()
{
	stayCurrent();
	QReadLocker rfs_lock(&current_rfs_lock);
	return current_rfs.freq_or_delta;
}

double RemoteFrequencySource::GetRampRate()
{
	stayCurrent();
	QReadLocker rfs_lock(&current_rfs_lock);
	return current_rfs.ramp_window;
}

double RemoteFrequencySource::GetWindow()
{
	stayCurrent();
	QReadLocker rfs_lock(&current_rfs_lock);
	return current_rfs.ramp_window;
}

double RemoteFrequencySource::GetTimeConst()
{
	stayCurrent();
	QReadLocker rfs_lock(&current_rfs_lock);
	return current_rfs.ramp_time_const;
}

void RemoteFrequencySource::get_remote_state_if_connected()
{
	if (isConnected())
	{
		rfs_state rfs;
		SendPacket( rfs, freq_control_packet::GET_STATE );
	}
}

class SleepHelper : public QThread
{
public:
static void msleep(int ms)
{
	QThread::msleep(ms);
}
};

bool RemoteFrequencySource::isConnected()
{
	return tcp_sock.state() == QAbstractSocket::ConnectedState;
}

void RemoteFrequencySource::SendPacket(rfs_state& rfs, freq_control_packet::packet_type ptype)
{
	QWriteLocker locker(&tcp_lock);

	freq_control_packet p(rfs, ptype);

	int retval = 0;

	if (isConnected())
	{
		retval = tcp_sock.write((char*)&p, sizeof(p));

		if (retval == sizeof(p))
		{
//			cout << "send " << p.description() << ": " << p << endl;

			if (tcp_sock.waitForReadyRead(1000))
				retval = tcp_sock.read((char*)&p, sizeof(p));

			if (retval == sizeof(p))
			{
//				cout << "recv " << p.description() << ": " << p;
//				cout << " round trip:  " << setprecision(3) << fixed << CurrentTime_s() - p.tSent << " ms" << endl << endl;


				QWriteLocker rfs_lock(&current_rfs_lock);
				current_rfs = p.rfs;

				emit sig_new_state(current_rfs);
			}
			else
				cout << name << " -- [SendPacket] communication failure (recv)" << endl;
		}
		else
			cout << name << " -- [SendPacket] communication failure (send)" << endl;
	}
	else
		cout << name << " -- [SendPacket] communication failure (no connection)" << endl;
}
