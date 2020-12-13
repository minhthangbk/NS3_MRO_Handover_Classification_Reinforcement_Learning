/*
 * real-map-mobility-model.cc
 *
 *  Created on: May 2, 2018
 *      Author: thang
 */
/*
 * back-forth-between-two-points-mobility-model.cc
 *
 *  Created on: Nov 18, 2017
 *      Author: thang
 */

#include <cmath>
#include "ns3/simulator.h"
#include "ns3/double.h"
#include "real-map-mobility-model.h"
#include <ns3/fatal-error.h>
#include <ns3/log.h>
#include <math.h>
#include <ns3/boolean.h>
#include <map>

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (RealMapMobilityModel);

TypeId
RealMapMobilityModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RealMapMobilityModel")
    .SetParent<MobilityModel> ()
    .SetGroupName ("Mobility")
    .AddConstructor<RealMapMobilityModel> ()
    .AddAttribute ("MinSpeed",
                   "Minimum speed value, [m/s]",
                   DoubleValue (1),
                   MakeDoubleAccessor (&RealMapMobilityModel::m_minSpeed),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MaxSpeed",
                   "Maximum speed value, [m/s]",
                   DoubleValue (1),
                   MakeDoubleAccessor (&RealMapMobilityModel::m_maxSpeed),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Z",
                   "Z value of traveling region (fixed), [m]",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&RealMapMobilityModel::m_z),
                   MakeDoubleChecker<double> ())
	.AddAttribute("PauseTime",
			"after reaching an edge, wait for a while to avoid ping-pong",
			DoubleValue(0),
			MakeDoubleAccessor (&RealMapMobilityModel::m_pauseTime),
			MakeDoubleChecker<double> ())
    ;

  return tid;
}

RealMapMobilityModel::RealMapMobilityModel ()
{
	m_speed = CreateObject<UniformRandomVariable> ();
	alreadyStarted = false;
	//  m_pause = CreateObject<UniformRandomVariable> ();

	//Manually set the points
	//TODO: make sure that there are at least 2 options at each nodes to avoid non-stop loop!!!
	m_ConnectionMap[1][1] = 2; m_ConnectionMap[1][2] = 3; m_ConnectionMap[1][3] = 4;
	m_ConnectionMap[2][1] = 1; m_ConnectionMap[2][2] = 5;
	m_ConnectionMap[3][1] = 1; m_ConnectionMap[3][2] = 2; m_ConnectionMap[3][3] = 4; m_ConnectionMap[3][4] = 5;
	m_ConnectionMap[4][1] = 1; m_ConnectionMap[4][2] = 5;
	m_ConnectionMap[5][1] = 2; m_ConnectionMap[5][2] = 3; m_ConnectionMap[5][3] = 4;

	m_LocationsMap[1] = Vector(50,50,m_z);
	m_LocationsMap[2] = Vector(80,80,m_z);
	m_LocationsMap[3] = Vector(80,50,m_z);
	m_LocationsMap[4] = Vector(80,20,m_z);
	m_LocationsMap[5] = Vector(110,50,m_z);
}

void
RealMapMobilityModel::DoInitialize (void)
{
	NS_ASSERT (m_minSpeed >= 1e-6);
//	NS_LOG_LOGIC("RealMapMobilityModel::DoInitialize MinSpeed " << m_minSpeed << " MaxSpeed " << m_maxSpeed);
	NS_ASSERT (m_minSpeed <= m_maxSpeed);

	m_speed->SetAttribute ("Min", DoubleValue (m_minSpeed));
	m_speed->SetAttribute ("Max", DoubleValue (m_maxSpeed));
	m_randGen = CreateObject<UniformRandomVariable>();
	m_srcId = m_randGen->GetInteger(1,5);
	m_destId = 0;//m_randGen->GetInteger(1,5);

	m_helper.Update ();
	m_helper.Pause ();

	//randomly choose the beginning point
	m_helper.SetPosition(m_LocationsMap[m_srcId]);


	NS_ASSERT (!m_event.IsRunning ());
	m_event = Simulator::ScheduleNow (&RealMapMobilityModel::BeginWalk, this);
	alreadyStarted = true;
	NotifyCourseChange ();
	MobilityModel::DoInitialize ();
}

void
RealMapMobilityModel::BeginWalk (void)
{
	m_helper.Update ();
	m_helper.Pause ();
	Vector current = m_helper.GetCurrentPosition ();
	Vector destination;
	if(m_destId == 0)
	{
		m_destId = m_randGen->GetInteger(1,5);
//		destination.x = m_LocationsMap[m_destId].x;
//		destination.y = m_LocationsMap[m_destId].y;
//		destination.z = m_LocationsMap[m_destId].z;
		destination = m_LocationsMap[m_destId];

	}
	else
	{
		uint16_t posId = 0;
		do
		{
			posId = m_ConnectionMap[m_destId][m_randGen->GetInteger(1,m_ConnectionMap[m_destId].size())];
		}while(posId == m_srcId); //TODO:Not return to previous position --> make sure that there are at least 2 options at each nodes to avoid non-stop loop!!!

		destination = m_LocationsMap[posId];
		m_srcId = m_destId; //update new info
		m_destId = posId;
	}

	double speed = m_speed->GetValue ();
	double dx = (destination.x - current.x);
	double dy = (destination.y - current.y);
	double dz = (destination.z - current.z);
	double k = speed / std::sqrt (dx*dx + dy*dy + dz*dz);

	m_helper.SetVelocity (Vector (k*dx, k*dy, k*dz));
	m_helper.Unpause ();

	Time travelDelay = Seconds (CalculateDistance (destination, current) / speed);
	m_event = Simulator::Schedule (travelDelay,
			&RealMapMobilityModel::PauseAtEdge, this);
	NotifyCourseChange ();
}

void
RealMapMobilityModel::PauseAtEdge()
{
	m_helper.Update ();
	m_helper.Pause ();
	Time pause = Seconds (m_pauseTime);
	m_event = Simulator::Schedule (pause, &RealMapMobilityModel::BeginWalk, this);
	NotifyCourseChange ();
}

Vector
RealMapMobilityModel::DoGetPosition (void) const
{
	m_helper.Update ();
	return m_helper.GetCurrentPosition ();
}

void
RealMapMobilityModel::DoSetPosition (const Vector &position)
{
	if (alreadyStarted)
	{
		m_helper.SetPosition (position);
		Simulator::Remove (m_event);
		m_event = Simulator::ScheduleNow (&RealMapMobilityModel::BeginWalk, this);
	}
}

Vector
RealMapMobilityModel::DoGetVelocity (void) const
{
	return m_helper.GetVelocity ();
}

int64_t
RealMapMobilityModel::DoAssignStreams (int64_t stream)
{
	m_speed->SetStream (stream);
	m_randGen->SetStream(stream+1);
	return 3;
}

}  // namespace ns3






