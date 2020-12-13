/*
 * random-circle-in-disc.h
 *
 *  Created on: Aug 24, 2016
 *      Author: thang
 */

#ifndef SRC_MOBILITY_MODEL_RANDOM_CIRCLE_IN_DISC_H_
#define SRC_MOBILITY_MODEL_RANDOM_CIRCLE_IN_DISC_H_

#include "constant-velocity-helper.h"
#include "mobility-model.h"
#include "position-allocator.h"
#include "ns3/ptr.h"
#include "ns3/random-variable-stream.h"
#include "ns3/event-id.h"

namespace ns3 {

class RandomCircleInDisc : public MobilityModel
{
public:
	static TypeId GetTypeId (void);
	RandomCircleInDisc ();
protected:
	virtual void DoInitialize (void);
private:

	void BeginWalk (void);
	virtual Vector DoGetPosition (void) const;
	virtual void DoSetPosition (const Vector &position);
	virtual Vector DoGetVelocity (void) const;
	virtual int64_t DoAssignStreams (int64_t);

	ConstantVelocityHelper m_helper; //!< helper for velocity computations
//	double m_maxSpeed; //!< maximum speed value (m/s)
//	double m_minSpeed; //!< minimum speed value (m/s)
//	Ptr<UniformRandomVariable> m_speed; //!< random variable for speed values
	double m_speed; //m/s
	double m_z; //!< z value of traveling region
	EventId m_event; //!< current event ID
	bool alreadyStarted;

	double m_discCenterX;//rectangle area
	double m_discCenterY;
	Ptr<UniformRandomVariable> m_discR;
	Ptr<UniformRandomVariable> m_discTheta;
	Ptr<UniformRandomVariable> m_UeTheta;
	double m_minDiscR;
	double m_maxDiscR;

	double m_curAlpha;

	double m_UeCenterX;
	double m_UeCenterY;
	double m_UeR;
};
}
#endif /* SRC_MOBILITY_MODEL_RANDOM_CIRCLE_IN_DISC_H_ */
