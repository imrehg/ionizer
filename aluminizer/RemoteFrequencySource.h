#pragma once

#include "FrequencySource.h"
#include "freq_control_packet.h"

class RemoteFrequencySource : public QThread, public SweepingFrequencySource
{
Q_OBJECT

public:
RemoteFrequencySource(double dMinFrequency, double dMaxFrequency,
                      double dMaxRampRate, const std::string& name,
                      const std::string &server_name, unsigned short port);

virtual ~RemoteFrequencySource();

void run();

void SetDeviceFrequency(double dNewFreq);

virtual double GetRampRate();
virtual double GetWindow();
virtual double GetTimeConst();
virtual std::string getStatusText();

bool isConnected();
void getCurrentRFS(rfs_state*);

signals:
void sig_new_state(rfs_state);

public slots:
void slot_get_state();
void slot_set_state(rfs_state);
void slot_shiftVisFrequency(double);
void slot_newErrSig(double);
void slot_connect();
void slot_clearHistory();
void slot_connected();

protected:
void stayCurrent();
void get_remote_state_if_connected();
virtual double GetCurrentDeviceFrequency();
virtual void ClearHistory();
virtual void ShiftFrequency(double dShift);
virtual void newErrSig(double err_sig);

//! connect to remote host
void connect();

void SendPacket(rfs_state&, freq_control_packet::packet_type ptype);

QHostAddress host_address;
QTcpSocket tcp_sock;

std::string server_name;
unsigned short port;

QReadWriteLock tcp_lock, current_rfs_lock;
rfs_state current_rfs;
};

