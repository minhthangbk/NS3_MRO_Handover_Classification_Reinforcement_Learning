/*
 * manhattan-conor-mobility-model.h
 *
 *  Created on: Sep 20, 2017
 *      Author: thang
 */

#ifndef MANHATTAN_CONOR_MOBILITY_MODEL_H_
#define MANHATTAN_CONOR_MOBILITY_MODEL_H_

#include "constant-velocity-helper.h"
#include "mobility-model.h"
#include "position-allocator.h"
#include "ns3/ptr.h"
#include "ns3/random-variable-stream.h"
#include "ns3/event-id.h"

namespace ns3 {

class ManhattanCornerMobilityModel : public MobilityModel
{
public:
	static TypeId GetTypeId (void);
	ManhattanCornerMobilityModel ();
protected:
	virtual void DoInitialize (void);
private:

//	void Start (void);
	void BeginWalk (void);
	virtual Vector DoGetPosition (void) const;
	virtual void DoSetPosition (const Vector &position);
	virtual Vector DoGetVelocity (void) const;
	virtual int64_t DoAssignStreams (int64_t);

	bool MustTurn(Vector pos, uint8_t dir);//Check whether the user need to change direction

	ConstantVelocityHelper m_helper; 			//!< helper for velocity computations
	double m_maxSpeed; 							//!< maximum speed value (m/s)
	double m_minSpeed; 							//!< minimum speed value (m/s)
	Ptr<UniformRandomVariable> m_rndGen; 		//!< random variable for many purpose

	EventId m_event; //!< current event ID

	bool m_alreadyStarted;

	double m_maxPause; /** Maximum pause time [s]. */
	double m_minPause; /** Maximum pause time [s]. */

	double m_xMax;
	double m_xMin;
	double m_yMax;
	double m_yMin;

	uint8_t m_curDir; //Current direction
	double m_dist;
	double m_curSpeed;

	bool m_leftOrRight; //0: left; 1: right
	bool m_topOrBottom; //0: top; 1: bottom
};


}//End namespace



#endif /* MANHATTAN_CONOR_MOBILITY_MODEL_H_ */
