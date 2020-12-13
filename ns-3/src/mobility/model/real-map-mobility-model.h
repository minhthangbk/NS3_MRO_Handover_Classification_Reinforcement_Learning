/*
 * real-map-mobility-model.h
 *
 *  Created on: May 2, 2018
 *      Author: thang
 */

#ifndef REAL_MAP_MOBILITY_MODEL_H_
#define REAL_MAP_MOBILITY_MODEL_H_

#include "constant-velocity-helper.h"
#include "mobility-model.h"
#include "position-allocator.h"
#include "ns3/ptr.h"
#include "ns3/random-variable-stream.h"
#include "ns3/event-id.h"
#include "ns3/vector.h"
#include <map>

namespace ns3 {

class RealMapMobilityModel : public MobilityModel
{
public:
	static TypeId GetTypeId (void);
	RealMapMobilityModel ();
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
	bool alreadyStarted;
	/*
	 * Dev by THANG
	 */
	double m_pauseTime;

//	bool m_1or2;//0 forth 1 back
	Ptr<UniformRandomVariable> m_randGen;
	uint16_t m_destId;
	uint16_t m_srcId;
	std::map<uint16_t,std::map<uint16_t,uint16_t> > m_ConnectionMap;
	std::map<uint16_t,Vector> m_LocationsMap;
};

}

#endif /* REAL_MAP_MOBILITY_MODEL_H_ */
