/*
 * random-circle-cross-circle.h
 *
 *  Created on: Dec 26, 2016
 *      Author: thang
 */

#ifndef SRC_MOBILITY_MODEL_RANDOM_CIRCLE_CROSS_CIRCLE_H_
#define SRC_MOBILITY_MODEL_RANDOM_CIRCLE_CROSS_CIRCLE_H_

#include "constant-velocity-helper.h"
#include "mobility-model.h"
#include "position-allocator.h"
#include "ns3/ptr.h"
#include "ns3/random-variable-stream.h"
#include "ns3/event-id.h"

namespace ns3 {

class RandomCircleCrossCircle : public MobilityModel
{
public:
	static TypeId GetTypeId (void);
	RandomCircleCrossCircle ();
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

	double m_c1CenterX;
	double m_c1CenterY;
	double m_c1R;
	Ptr<UniformRandomVariable> m_c2R;
	Ptr<UniformRandomVariable> m_c2Theta;

	Ptr<UniformRandomVariable> m_ueTheta;

	double m_ueCurTheta;
	//	double m_minDiscR;
	//	double m_maxDiscR;

	double m_ueCenterX;
	double m_ueCenterY;
	double m_ueR;
};
}

#endif /* SRC_MOBILITY_MODEL_RANDOM_CIRCLE_CROSS_CIRCLE_H_ */
