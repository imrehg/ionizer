#ifdef PRECOMPILED_HEADER
#include "common.h"
#endif

#include "ScanObject.h"
#include "ExpSCAN.h"

using namespace std;



LockScan::LockScan(ExpSCAN* scanPage, vector<scan_variable*> scan_variables, LockParams* lp, ScanSource* ss) :
	ContinuousScan(scanPage, scan_variables),
	pCenterChannel( new ConstantChannel("center", false) ),
	pLockParams(lp),
	num_new_points(0),
	ss(ss)
{
	data_feed.AddChannel(pCenterChannel);
}

void LockScan::UseValidData()
{
	ContinuousScan::UseValidData();

	if (validData.size() > 0)
		pLockParams->RespondToMeasurement();

	//take 3 data points before signaling OnIdle
	if (++num_new_points == GetNumToTake())
	{
		num_new_points = 0;
		setRunStatus(ExperimentBase::IDLE);
	}
	else
		setRunStatus(ExperimentBase::NEED_MORE_DATA);

}

string LockScan::GetPlotLabel()
{
	return pLockParams->GetLockVariableName() + " = " + to_string<double>(pLockParams->GetCenter(), 2);
}

LockInScan::LockInScan(ExpSCAN* scanPage, vector<scan_variable*> scan_variables, LockInParams* lp, ScanSource* ss) :
	LockScan(scanPage, scan_variables, lp, ss),
	pLockInParams(lp),
	iCurrentPoint(0),
	flipLeftRight(0),
	nLeft(0),
	nRight(0),
	nCenter(0),
	measurements(3),
	pcAdjustments( new ConstantChannel("adjustments", false) )
{
	data_feed.AddChannel(pcAdjustments);

	data_feed.AddChannel(pcLeftSignal   = new ConstantChannel("Left", true, 3));
	data_feed.AddChannel(pcCenterSignal = new ConstantChannel("Center", true, 3));
	data_feed.AddChannel(pcRightSignal  = new ConstantChannel("Right", true, 3));

	//is this the same output as in SetOutput ???
//	pLockInParams->SetCenter( pLockInParams->GetOutput() );
}

void LockInScan::AcquireDataPoint()
{
	//pLockInParams->SetOutput( pLockInParams->GetCenter() + ModulationState(iCurrentPoint) );

	pLockInParams->modulateOutput( ModulationState(iCurrentPoint) );

	if ( iCurrentPoint == LeftIndex())
		nLeft++;

	if ( iCurrentPoint == RightIndex())
		nRight++;

	if ( iCurrentPoint == CenterIndex())
		nCenter++;
/*
   if( (nLeft == nRight) && (nLeft % 10 == 0))
   {
      //flipLeftRight = 1 - flipLeftRight;

      for(size_t i = 0; i < measurements.size(); i++)
         measurements[i].counts = -1;
   }
 */

	pCenterChannel->SetCurrentData( pLockInParams->GetCenter() );
	pcAdjustments->SetCurrentData( pLockInParams->GetOutput() );

	return ContinuousScan::AcquireDataPoint();
}

void LockInScan::UpdateLockSignalChannels()
{
	double counts = pLockInParams->GetSignal();

	//update array of measurements with latest data
	measurements[iCurrentPoint].x       = pLockInParams->GetOutput();
	measurements[iCurrentPoint].counts     = counts;

	if (iCurrentPoint == LeftIndex()) pcLeftSignal->SetCurrentData(counts);
	if (iCurrentPoint == RightIndex()) pcRightSignal->SetCurrentData(counts);
	if (iCurrentPoint == CenterIndex()) pcCenterSignal->SetCurrentData(counts);
}


void LockInScan::UseValidData()
{
	ContinuousScan::UseValidData();

	if (validData.size() > 0)
	{
		if (!pLockInParams->UpdateMeasurements(measurements))
		{
			UpdateLockSignalChannels();
			pLockInParams->RespondToMeasurements(this, measurements, iCurrentPoint);
		}
		else
		{
			double err, dX;

			pLockInParams->getErrorSignal(&err, &dX);

			printf("error = %6.4f    dX = %12.9f\r\n", err, dX);
			pLockInParams->RespondToMeasurements(this, err, dX);
		}
	}

	if ( ++num_new_points == GetNumToTake() )
	{
		num_new_points = 0;
		setRunStatus(ExperimentBase::IDLE);
	}
	else
		setRunStatus(ExperimentBase::NEED_MORE_DATA);
}

void LockInScan::NextDataPoint()
{
	iCurrentPoint = (iCurrentPoint + 1) % measurements.size();
	ContinuousScan::NextDataPoint();
}

void LockInScan::DefaultExperimentState()
{
	pLockInParams->modulateOutput( 0 );
}

void LockInScan::ClearMeasurements()
{
	for (size_t i = 0; i < measurements.size(); i++)
		measurements[i].counts = -1;
}


double LockInScan::ModulationState(int i)
{
	int multiplier = 0;

	if (i == LeftIndex())
		multiplier = -1;

	if (i == RightIndex())
		multiplier = 1;

	if (flipLeftRight)
		multiplier *= -1;

	return multiplier * pLockInParams->GetModulation();
}


void LockInParams::RespondToMeasurements(LockInScan* pScan, const measurements_t& m, size_t i)
{
	if ((int)i == pScan->RightIndex() || (int)i == pScan->LeftIndex())
	{
		for (size_t j = 0; j < m.size(); j++)
			if (m[j].counts == -1)
				return;

		error_signal = m.at(pScan->RightIndex()).counts - m.at(pScan->LeftIndex()).counts;
		deltaX      = m.at(pScan->RightIndex()).x - m.at(pScan->LeftIndex()).x;

		if (MakeCorrection())
			ShiftCenterNormalized(GetGain() * error_signal / 2);
	}
}

void LockInParams::RespondToMeasurements(LockInScan*, double err, double dX)
{
	error_signal = err;
	deltaX = dX;

	if (MakeCorrection())
		ShiftCenterNormalized(GetGain() * error_signal / 2);
}

ExperimentBase::run_status LockScan::DataPoint()
{
	return ContinuousScan::DataPoint();
}

