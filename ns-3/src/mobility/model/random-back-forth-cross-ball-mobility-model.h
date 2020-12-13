/*
 * random-back-forth-cross-ball-mobility-model.h
 *
 *  Created on: Jan 25, 2016
 *      Author: thang
 */

#ifndef NS_3_23_SRC_MOBILITY_MODEL_RANDOM_BACK_FORTH_CROSS_BALL_MOBILITY_MODEL_H_
#define NS_3_23_SRC_MOBILITY_MODEL_RANDOM_BACK_FORTH_CROSS_BALL_MOBILITY_MODEL_H_

#include "constant-velocity-helper.h"
#include "mobility-model.h"
#include "position-allocator.h"
#include "ns3/ptr.h"
#include "ns3/random-variable-stream.h"
#include "ns3/event-id.h"

namespace ns3 {

class RandomBackForthCrossBallMobilityModel : public MobilityModel
{
public:
	static TypeId GetTypeId (void);
	RandomBackForthCrossBallMobilityModel ();
protected:
	virtual void DoInitialize (void);
private:

	void BeginWalk (void);
	virtual Vector DoGetPosition (void) const;
	virtual void DoSetPosition (const Vector &position);
	virtual Vector DoGetVelocity (void) const;
	virtual int64_t DoAssignStreams (int64_t);

	void PauseAtEdge();
	ConstantVelocityHelper m_helper; //!< helper for velocity computations
	double m_maxSpeed; //!< maximum speed value (m/s)
	double m_minSpeed; //!< minimum speed value (m/s)
	Ptr<UniformRandomVariable> m_speed; //!< random variable for speed values
	double m_z; //!< z value of traveling region
//	Ptr<RandomBoxPositionAllocator> m_position; //!< position allocator//Change by THANG
	EventId m_event; //!< current event ID
	bool m_verOrHor; //vertical or horizontal
	bool alreadyStarted;
	/*
	 * Dev by THANG
	 */
	double m_minX;
	double m_maxX;
	double m_minY;
	double m_maxY;//next theta
	double m_pauseTime;

	bool m_backOrForth;//0 forth 1 back
	Ptr<UniformRandomVariable> m_x;
	Ptr<UniformRandomVariable> m_y;

	double m_ballCenterX;
	double m_ballCenterY;
	double m_ballRadius;
};

}  // namespace ns3



#endif /* NS_3_23_SRC_MOBILITY_MODEL_RANDOM_BACK_FORTH_CROSS_BALL_MOBILITY_MODEL_H_ */
