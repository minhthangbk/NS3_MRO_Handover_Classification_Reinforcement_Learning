/*
 * random-circle-in-rectangle.h
 *
 *  Created on: Aug 24, 2016
 *      Author: thang
 */

#ifndef SRC_MOBILITY_MODEL_RANDOM_CIRCLE_IN_RECTANGLE_H_
#define SRC_MOBILITY_MODEL_RANDOM_CIRCLE_IN_RECTANGLE_H_

#include "constant-velocity-helper.h"
#include "mobility-model.h"
#include "position-allocator.h"
#include "ns3/ptr.h"
#include "ns3/random-variable-stream.h"
#include "ns3/event-id.h"

namespace ns3 {

class RandomCircleInRectangle : public MobilityModel
{
public:
	static TypeId GetTypeId (void);
	RandomCircleInRectangle ();
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

	double m_minX;//rectangle area
	double m_maxX;
	double m_minY;
	double m_maxY;//next theta

	double m_curAlpha;

	Ptr<UniformRandomVariable> m_randX; //randomly allocate the center
	Ptr<UniformRandomVariable> m_randY;
	double m_cX;
	double m_cY;
	double m_cR;
};

}  // namespace ns3



#endif /* SRC_MOBILITY_MODEL_RANDOM_CIRCLE_IN_RECTANGLE_H_ */
