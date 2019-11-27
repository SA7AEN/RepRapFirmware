/*
 * DDARing.h
 *
 *  Created on: 28 Feb 2019
 *      Author: David
 *
 *  This class represents a queue of moves, where for each move the movement is synchronised between all the motors involved.
 */

#ifndef SRC_MOVEMENT_DDARING_H_
#define SRC_MOVEMENT_DDARING_H_

#include "DDA.h"

class DDARing
{
public:
	DDARing();

	void Init1(unsigned int numDdas);
	void Init2();
	void Exit();

	void RecycleDDAs();
	bool CanAddMove() const;
	bool AddStandardMove(const RawMove &nextMove, bool doMotorMapping) __attribute__ ((hot));	// Set up a new move, returning true if it represents real movement
	bool AddSpecialMove(float feedRate, const float coords[MaxDriversPerAxis]);
#if SUPPORT_ASYNC_MOVES
	bool AddAsyncMove(const AsyncMove& nextMove);
#endif

	void Spin(uint8_t simulationMode, bool shouldStartMove);					// Try to process moves in the ring, returning true if the ring is idle
	bool IsIdle() const;														// Return true if this DDA ring is idle

	float PushBabyStepping(size_t axis, float amount);							// Try to push some babystepping through the lookahead queue, returning the amount pushed

	void Interrupt(Platform& p);												// Check endstops, generate step pulses
	void OnMoveCompleted(DDA *cdda, Platform& p);								// called when the state has been set to 'completed'
	bool ScheduleNextStepInterrupt();											// Schedule the next step interrupt, returning true if we failed because it is due immediately
	void CurrentMoveCompleted() __attribute__ ((hot));							// Signal that the current move has just been completed
	void TryStartNextMove(Platform& p, uint32_t startTime) __attribute__ ((hot));	// Try to start another move

	uint32_t ExtruderPrintingSince() const { return extrudersPrintingSince; }	// When we started doing normal moves after the most recent extruder-only move
	int32_t GetAccumulatedExtrusion(size_t extruder, size_t drive, bool& isPrinting);

	uint32_t GetScheduledMoves() const { return scheduledMoves; }				// How many moves have been scheduled?
	uint32_t GetCompletedMoves() const { return completedMoves; }				// How many moves have been completed?
	void ResetMoveCounters() { scheduledMoves = completedMoves = 0; }

	float GetSimulationTime() const { return simulationTime; }
	void ResetSimulationTime() { simulationTime = 0.0; }

#if HAS_SMART_DRIVERS
	uint32_t GetStepInterval(size_t axis, uint32_t microstepShift) const;
#endif

	DDA *GetCurrentDDA() const { return currentDda; }							// Return the DDA of the currently-executing move, or nullptr

	uint32_t GetClearNumHiccups();
	float GetTopSpeed() const;
	float GetRequestedSpeed() const;

	int32_t GetEndPoint(size_t drive) const { return liveEndPoints[drive]; } 	// Get the current position of a motor
	void GetCurrentMachinePosition(float m[MaxAxes], bool disableMotorMapping) const; // Get the current position in untransformed coords
	void SetPositions(const float move[MaxAxesPlusExtruders]);					// Force the machine coordinates to be these
	void AdjustMotorPositions(const float adjustment[], size_t numMotors);		// Perform motor endpoint adjustment
	void LiveCoordinates(float m[MaxAxesPlusExtruders]);						// Gives the last point at the end of the last complete DDA transformed to user coords
	void SetLiveCoordinates(const float coords[MaxAxesPlusExtruders]);			// Force the live coordinates (see above) to be these
	void ResetExtruderPositions();												// Resets the extrusion amounts of the live coordinates

	bool PauseMoves(RestorePoint& rp);											// Pause the print as soon as we can, returning true if we were able to skip any
#if HAS_VOLTAGE_MONITOR || HAS_STALL_DETECT
	bool LowPowerOrStallPause(RestorePoint& rp);								// Pause the print immediately, returning true if we were able to
#endif

#if SUPPORT_LASER
	uint32_t ManageLaserPower() const;											// Manage the laser power
#endif

	void RecordLookaheadError() { ++numLookaheadErrors; }						// Record a lookahead error
	void Diagnostics(MessageType mtype, const char *prefix);

private:
	bool StartNextMove(Platform& p, uint32_t startTime) __attribute__ ((hot));	// Start the next move, returning true if laser or IObits need to be controlled
	void PrepareMoves(DDA *firstUnpreparedMove, int32_t moveTimeLeft, unsigned int alreadyPrepared, uint8_t simulationMode);

	static bool TimerCallback(CallbackParameter p, StepTimer::Ticks& when);

	DDA* volatile currentDda;
	DDA* addPointer;
	DDA* volatile getPointer;
	DDA* checkPointer;

	StepTimer timer;															// Timer object to control getting step interrupts

	volatile float liveCoordinates[MaxAxesPlusExtruders];						// The endpoint that the machine moved to in the last completed move
	volatile bool liveCoordinatesValid;											// True if the XYZ live coordinates are reliable (the extruder ones always are)
	volatile int32_t liveEndPoints[MaxAxesPlusExtruders];						// The XYZ endpoints of the last completed move in motor coordinates

	unsigned int numDdasInRing;

	uint32_t scheduledMoves;													// Move counters for the code queue
	volatile uint32_t completedMoves;											// This one is modified by an ISR, hence volatile
	volatile int32_t numHiccups;												// Modified in the ISR

	unsigned int numLookaheadUnderruns;											// How many times we have run out of moves to adjust during lookahead
	unsigned int numPrepareUnderruns;											// How many times we wanted a new move but there were only un-prepared moves in the queue
	unsigned int numLookaheadErrors;											// How many times our lookahead algorithm failed
	unsigned int stepErrors;													// count of step errors, for diagnostics

	float simulationTime;														// Print time since we started simulating
	float extrusionPending[MaxExtruders];										// Extrusion not done due to rounding to nearest step
	volatile int32_t extrusionAccumulators[MaxExtruders]; 						// Accumulated extruder motor steps
	volatile uint32_t extrudersPrintingSince;									// The milliseconds clock time when extrudersPrinting was set to true
	volatile bool extrudersPrinting;											// Set whenever an extruder starts a printing move, cleared by a non-printing extruder move
};

// Start the next move. Return true if laser or IO bits need to be active
// Must be called with base priority greater than or equal to the step interrupt, to avoid a race with the step ISR.
inline bool DDARing::StartNextMove(Platform& p, uint32_t startTime)
pre(ddaRingGetPointer->GetState() == DDA::frozen)
{
	DDA * const cdda = getPointer;			// capture volatile variable
	if (cdda->IsNonPrintingExtruderMove())
	{
		extrudersPrinting = false;
	}
	else if (!extrudersPrinting)
	{
		extrudersPrinting = true;
		extrudersPrintingSince = millis();
	}
	currentDda = cdda;
	cdda->Start(p, startTime);
#if SUPPORT_LASER || SUPPORT_IOBITS
	return cdda->ControlLaser();
#else
	return false;
#endif
}

#if HAS_SMART_DRIVERS
inline uint32_t DDARing::GetStepInterval(size_t axis, uint32_t microstepShift) const
{
	const DDA * const cdda = currentDda;		// capture volatile variable
	return (cdda != nullptr) ? cdda->GetStepInterval(axis, microstepShift) : 0;
}
#endif

// Schedule the next step interrupt for this DDA ring
// Base priority must be >= NvicPriorityStep when calling this
inline bool DDARing::ScheduleNextStepInterrupt()
{
	DDA * const cdda = currentDda;				// capture volatile variable
	return (cdda != nullptr) && cdda->ScheduleNextStepInterrupt(timer);
}

inline uint32_t DDARing::GetClearNumHiccups()
{
	const uint32_t ret = numHiccups;
	numHiccups = 0;
	return ret;
}

#endif /* SRC_MOVEMENT_DDARING_H_ */
