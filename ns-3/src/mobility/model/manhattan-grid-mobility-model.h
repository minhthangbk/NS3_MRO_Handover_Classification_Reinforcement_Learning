/*
 * manhattan-grid-mobility-model.h
 *
 *  Created on: Apr 28, 2017
 *      Author: thang
 */

#ifndef SRC_MOBILITY_MODEL_MANHATTAN_GRID_MOBILITY_MODEL_H_
#define SRC_MOBILITY_MODEL_MANHATTAN_GRID_MOBILITY_MODEL_H_

#include "constant-velocity-helper.h"
#include "mobility-model.h"
#include "position-allocator.h"
#include "ns3/ptr.h"
#include "ns3/random-variable-stream.h"
#include "ns3/event-id.h"

namespace ns3 {

class ManhattanGridMobilityModel : public MobilityModel
{
public:
	static TypeId GetTypeId (void);
	ManhattanGridMobilityModel ();
protected:
	virtual void DoInitialize (void);
private:

//	void Start (void);
	void BeginWalk (void);
	virtual Vector DoGetPosition (void) const;
	virtual void DoSetPosition (const Vector &position);
	virtual Vector DoGetVelocity (void) const;
	virtual int64_t DoAssignStreams (int64_t);

	bool OutOfBounds(Vector pos);//Check whether the user goes beyond the boundary
	bool MustTurn(Vector pos, uint8_t dir);//Check whether the user need to change direction
	Vector AlignPosition(Vector pos);
	Vector GetDestination(Vector pos, double dist, uint8_t dir);

	ConstantVelocityHelper m_helper; 			//!< helper for velocity computations
	double m_maxSpeed; 							//!< maximum speed value (m/s)
	double m_minSpeed; 							//!< minimum speed value (m/s)
//	Ptr<UniformRandomVariable> m_speedRV; 		//!< random variable for speed values
//	Ptr<UniformRandomVariable> m_turnRV; 		//!< random variable for speed values
//	Ptr<UniformRandomVariable> m_speedChangeRV; 		//!< random variable for speed values
	Ptr<UniformRandomVariable> m_rndGen; 		//!< random variable for many purpose

	EventId m_event; //!< current event ID

	bool m_alreadyStarted;

	uint8_t m_xBlocks; /** Number of blocks on x-axis. */
	uint8_t m_yBlocks; /** Number of blocks on y-axis. */
	double m_turnProb; 	/** Probability for the mobile to turn at a crossing. */
	double m_updateDist; /** Distance interval in which to possibly update the mobile's speed [m]. */
	double m_speedChangePro; /** Probability for the mobile to change its speed (every updateDist m). */
	double m_pausePro; /** Probability for the mobile to pause (every updateDist m), given it does not change it's speed. */
	double m_maxPause; /** Maximum pause time [s]. */
	double m_minPause; /** Maximum pause time [s]. */

	double m_xDim; /** Size of a block on x-axis, calculated from xblocks. */
	double m_yDim; /** Size of a block on y-axis, calculated from xblocks. */

	double m_xMax;
	double m_xMin;
	double m_yMax;
	double m_yMin;

//	double m_curX;
//	double m_curY;

	uint8_t m_curDir; //Current direction
	double m_gridDist; //distance from the current position to the closed crossroad given direction
	double m_dist;
	double m_curSpeed;

};


}//End namespace



#endif /* SRC_MOBILITY_MODEL_MANHATTAN_GRID_MOBILITY_MODEL_H_ */
