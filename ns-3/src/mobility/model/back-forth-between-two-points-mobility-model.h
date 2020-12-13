/*
 * back-forth-between-two-points-mobility-model.h
 *
 *  Created on: Nov 18, 2017
 *      Author: thang
 */

#ifndef BACK_FORTH_BETWEEN_TWO_POINTS_MOBILITY_MODEL_H_
#define BACK_FORTH_BETWEEN_TWO_POINTS_MOBILITY_MODEL_H_

#include "constant-velocity-helper.h"
#include "mobility-model.h"
#include "position-allocator.h"
#include "ns3/ptr.h"
#include "ns3/random-variable-stream.h"
#include "ns3/event-id.h"

namespace ns3 {

class BackForthBetweenTwoPointsMobilityModel : public MobilityModel
{
public:
	static TypeId GetTypeId (void);
	BackForthBetweenTwoPointsMobilityModel ();
protected:
	virtual void DoInitialize (void);
private:

	void BeginWalk (void);
	virtual Vector DoGetPosition (void) const;
	virtual void DoSetPosition (const Vector &position);
	virtual Vector DoGetVelocity (void) const;
	virtual int64_t DoAssignStreams (int64_t);

	void PauseAtEdge();
	void PauseAtStart();
	ConstantVelocityHelper m_helper; //!< helper for velocity computations
	double m_maxSpeed; //!< maximum speed value (m/s)
	double m_minSpeed; //!< minimum speed value (m/s)
	Ptr<UniformRandomVariable> m_speed; //!< random variable for speed values
	double m_z; //!< z value of traveling region
//	Ptr<RandomBoxPositionAllocator> m_position; //!< position allocator//Change by THANG
	EventId m_event; //!< current event ID
	bool alreadyStarted;
	/*
	 * Dev by THANG
	 */
	double m_x1;
	double m_y1;
	double m_x2;//next theta
	double m_y2;
	double m_pauseTime;

	double m_initTime;
	bool m_1or2;//0 forth 1 back
	Ptr<UniformRandomVariable> m_randGen;
};

}

#endif /* BACK_FORTH_BETWEEN_TWO_POINTS_MOBILITY_MODEL_H_ */
