#pragma once

#include "InputParametersGUI.h"
#include "ScanObject.h"

/* general lock in servo */
/*
   template<class T> class ExpLockIn : public T, public LockInParams
   {
   public:
   ExpLockIn(const std::string& sPageName, ExperimentsSheet* pSheet, unsigned exp_id,
            const std::string& unit, const std::string& lock_variable_name) :
   T(sPageName, pSheet, exp_id),
   Modulation		("Modulation " + unit,	  T::GetTxtParams(), "0", T::GetParamsVector()),
   Gain			("Gain",		          T::GetTxtParams(), "0", T::GetParamsVector()),
   Center			("Lock-in center " + unit, T::GetTxtParams(), "0", T::GetParamsVector(), true),
   RunModulo		("Run modulo",		      T::GetTxtParams(), "1", T::GetParamsVector()),
   unit(unit),
   lock_variable_name(lock_variable_name)
   {
      Center.setPrecision(6);
   }

   virtual ~ExpLockIn() {}

   virtual ScanSource* getLockVariable() { return T::currentScanSource(); }

   virtual void InitializeScan()
   {
      RecalculateModulation();
      Center.SetValue(GetInitialCenter());
      T::InitializeScan();
   }

   virtual void DefaultExperimentState()
   {
      T::DefaultExperimentState();
      getLockVariable()->setDefaultValue();
   }

   virtual bool RecalculateModulation() { return false; }

   virtual void AddDataChannels(DataFeed& data_feed)
   {
      T::AddDataChannels(data_feed);

      if(T::pSignalChannel)
         T::pSignalChannel->SetPlot(!scans::IsLockInScan(T::ScanType));
   }

   protected:
   //	enum direction {LEFT, CENTER, RIGHT};

   //LockInParams overrides
   virtual void   modulateOutput(double d) { SetOutput(GetCenter()+d); }
   virtual void   SetOutput(double d) { return getLockVariable()->SetScanOutput(d); }
   virtual double GetOutput() { return getLockVariable()->getNonScanningValue(); }
   virtual double GetModulation() { return Modulation; }
   virtual void   ShiftCenter(double d)	{ SetCenter(GetCenter()+d); }
   virtual void   SetCenter(double d) { return getLockVariable()->setNonScanningValue(d); Center.SetValue(d); Center.PostUpdateGUI(); }
   virtual double GetCenter()	{ return getLockVariable()->getNonScanningValue(); }

   virtual double GetGain()			{ return Gain; }
   virtual double GetSignal()			{ return T::pSignalChannel->GetAverage(); }
   virtual std::string GetLockVariableName()	{ return lock_variable_name; }

   virtual int	GetRunModulo() { return RunModulo.Value(); }
   virtual void SetRunModulo(int rm) { RunModulo.SetValue(rm); }

   virtual double GetInitialCenter() { return GetCenter(); }

   protected:
   GUI_double Modulation;
   GUI_double Gain;
   GUI_double Center;
   GUI_int    RunModulo;

   protected:
   const std::string unit;
   const std::string lock_variable_name;

   };
 */
