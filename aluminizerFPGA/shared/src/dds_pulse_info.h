#pragma once

#include <string>

unsigned extract_flags(const std::string& s);


class pulse_info
{
public:
   pulse_info() : t(0), flags(0) {}

   bool getEnabledFlag() const;
   bool isEnabled() const;
        void setEnabledFlag(bool b);

   double t;
   unsigned flags;

   static const unsigned flag_disabled		= 0x00000001;
   static const unsigned flag_scan			= 0x00000002;
   static const unsigned flag_ramsey		= 0x00000004;
   static const unsigned flag_composite   = 0x00000008;

};

class ttl_pulse_info : public pulse_info
{
public:
   ttl_pulse_info();
   ttl_pulse_info(const std::string& s);

   void updateFromString(const char* s);

   int to_string(char* s, size_t n) const;

   unsigned ttl;
};

bool operator==(const ttl_pulse_info&, const ttl_pulse_info&);
bool operator!=(const ttl_pulse_info&, const ttl_pulse_info&);

class dds_pulse_info : public pulse_info
{
public:
   dds_pulse_info();
   dds_pulse_info(const std::string& s);

   void updateFromString(const char* s);

   int to_string(char* s, size_t n) const;

   bool hasSB() const { return sb != NO_SB; }

   const static int NO_SB;

   unsigned iDDS;
   double fOn, fOff;
   int sb;
};

bool operator==(const dds_pulse_info&, const dds_pulse_info&);
bool operator!=(const dds_pulse_info&, const dds_pulse_info&);
