/*
 * circle-mobility-model.h
 *
 *  Created on: Jan 24, 2016
 *      Author: thang
 */

#ifndef NS_3_23_SRC_MOBILITY_MODEL_CIRCLE_MOBILITY_MODEL_H_
#define NS_3_23_SRC_MOBILITY_MODEL_CIRCLE_MOBILITY_MODEL_H_

#include "constant-velocity-helper.h"
#include "mobility-model.h"
#include "position-allocator.h"
#include "ns3/ptr.h"
#include "ns3/random-variable-stream.h"
#include "ns3/event-id.h"
namespace ns3 {

class CircleMobilityModel : public MobilityModel
{
public:
	static TypeId GetTypeId (void);
	CircleMobilityModel ();
protected:
	virtual void DoInitialize (void);
private:

	void BeginWalk (void);
	virtual Vector DoGetPosition (void) const;
	virtual void DoSetPosition (const Vector &position);
	virtual Vector DoGetVelocity (void) const;
	virtual int64_t DoAssignStreams (int64_t);

	ConstantVelocityHelper m_helper; //!< helper for velocity computations
	double m_maxSpeed; //!< maximum speed value (m/s)
	double m_minSpeed; //!< minimum speed value (m/s)
	Ptr<UniformRandomVariable> m_speed; //!< random variable for speed values
	double m_z; //!< z value of traveling region
	Ptr<RandomDiscPositionAllocator> m_position; //!< position allocator//Change by THANG
	EventId m_event; //!< current event ID
	bool alreadyStarted; //!< flag for starting state
	/*
	 * Dev by THANG
	 */
	double m_centerX;
	double m_centerY;
	double m_radius;
	double m_nxtThe;//next theta
	double m_clockwise;

};

}  // namespace ns3

#endif /* NS_3_23_SRC_MOBILITY_MODEL_CIRCLE_MOBILITY_MODEL_H_ */
