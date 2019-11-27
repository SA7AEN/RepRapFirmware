/*
 * ZProbeEndstop.cpp
 *
 *  Created on: 15 Sep 2019
 *      Author: David
 */

#include "ZProbeEndstop.h"

#include "ZProbe.h"
#include "RepRap.h"
#include "Platform.h"
#include "Movement/Kinematics/Kinematics.h"

// Z probe endstop
ZProbeEndstop::ZProbeEndstop(uint8_t axis, EndStopPosition pos) : Endstop(axis, pos), zProbeNumber(0)
{
}

// Test whether we are at or near the stop
EndStopHit ZProbeEndstop::Stopped() const
{
	const ZProbe * const zp = reprap.GetPlatform().GetEndstops().GetZProbe(zProbeNumber);
	return (zp != nullptr) ? zp->Stopped() : EndStopHit::atStop;
}

// This is called to prime axis endstops
bool ZProbeEndstop::Prime(const Kinematics& kin, const AxisDriversConfig& axisDrivers)
{
	// Decide whether we stop just the driver, just the axis, or everything
	stopAll = (kin.GetConnectedAxes(GetAxis()) & ~MakeBitmap<AxesBitmap>(GetAxis())) != 0;

#if SUPPORT_CAN_EXPANSION
	//TODO if the Z probe is remote, check that the expansoin board knows about it
#endif

	return true;
}

// Check whether the endstop is triggered and return the action that should be performed. Called from the step ISR.
EndstopHitDetails ZProbeEndstop::CheckTriggered(bool goingSlow)
{
	EndstopHitDetails rslt;				// initialised by default constructor
	switch (Stopped())
	{
	case EndStopHit::atStop:
		rslt.SetAction((stopAll) ? EndstopHitAction::stopAll : EndstopHitAction::stopAxis);
		rslt.axis = GetAxis();
		if (GetAtHighEnd())
		{
			rslt.setAxisHigh = true;
		}
		else
		{
			rslt.setAxisLow = true;
		}
		break;

	case EndStopHit::nearStop:
		if (!goingSlow)
		{
			rslt.SetAction(EndstopHitAction::reduceSpeed);
		}
		break;

	default:
		break;
	}
	return rslt;
}

// This is called by the ISR to acknowledge that it is acting on the return from calling CheckTriggered. Called from the step ISR.
// Return true if we have finished with this endstop or probe in this move.
bool ZProbeEndstop::Acknowledge(EndstopHitDetails what)
{
	return what.GetAction() != EndstopHitAction::reduceSpeed;
}

void ZProbeEndstop::AppendDetails(const StringRef& str)
{
	str.cat("Z probe");
}

// End

