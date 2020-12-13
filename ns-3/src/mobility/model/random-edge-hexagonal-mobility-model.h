/*
 * random-edge-hexagonal-mobility-model.h
 *
 *  Created on: Mar 28, 2016
 *      Author: thang
 */

#ifndef NS_3_23_SRC_MOBILITY_MODEL_RANDOM_EDGE_HEXAGONAL_MOBILITY_MODEL_H_
#define NS_3_23_SRC_MOBILITY_MODEL_RANDOM_EDGE_HEXAGONAL_MOBILITY_MODEL_H_

#include "constant-velocity-helper.h"
#include "mobility-model.h"
#include "position-allocator.h"
#include "ns3/ptr.h"
#include "ns3/random-variable-stream.h"
#include "ns3/event-id.h"
namespace ns3 {

class RandomEdgeHexagonalMobilityModel: public MobilityModel
{
public:
	static TypeId GetTypeId (void);
	RandomEdgeHexagonalMobilityModel ();
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
	Ptr<UniformRandomVariable> m_speedRV; //!< random variable for speed values
	Ptr<UniformRandomVariable> m_edgeRV; //!< random variable for speed values
	double m_z; //!< z value of traveling region
//	Ptr<RandomBoxPositionAllocator> m_position; //!< position allocator//Change by THANG
	EventId m_event; //!< current event ID

	bool alreadyStarted;
	/*
	 * Dev by THANG
	 */
	double m_R;
	double m_centerX;
	double m_centerY;
	uint8_t m_curEdge;
	Ptr<UniformRandomVariable> m_x;
	uint8_t m_prevEdge; //avoid unwanted ping-pong issue for handover
	double m_pauseTime;
};

}  // namespace ns3



#endif /* NS_3_23_SRC_MOBILITY_MODEL_RANDOM_EDGE_HEXAGONAL_MOBILITY_MODEL_H_ */
