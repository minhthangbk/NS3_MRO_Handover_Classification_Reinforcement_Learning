/*
 * back-forth-between-two-points-mobility-model.cc
 *
 *  Created on: Nov 18, 2017
 *      Author: thang
 */

#include <cmath>
#include "ns3/simulator.h"
#include "ns3/double.h"
#include "back-forth-between-two-points-mobility-model.h"
#include <ns3/fatal-error.h>
#include <ns3/log.h>
#include <math.h>
#include <ns3/boolean.h>

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (BackForthBetweenTwoPointsMobilityModel);

TypeId
BackForthBetweenTwoPointsMobilityModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::BackForthBetweenTwoPointsMobilityModel")
    .SetParent<MobilityModel> ()
    .SetGroupName ("Mobility")
    .AddConstructor<BackForthBetweenTwoPointsMobilityModel> ()
    .AddAttribute ("MinSpeed",
                   "Minimum speed value, [m/s]",
                   DoubleValue (1),
                   MakeDoubleAccessor (&BackForthBetweenTwoPointsMobilityModel::m_minSpeed),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MaxSpeed",
                   "Maximum speed value, [m/s]",
                   DoubleValue (1),
                   MakeDoubleAccessor (&BackForthBetweenTwoPointsMobilityModel::m_maxSpeed),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Z",
                   "Z value of traveling region (fixed), [m]",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&BackForthBetweenTwoPointsMobilityModel::m_z),
                   MakeDoubleChecker<double> ())
   .AddAttribute ("X1",
				  "X1, [m]",
				  DoubleValue (20),
				  MakeDoubleAccessor (&BackForthBetweenTwoPointsMobilityModel::m_x1),
				  MakeDoubleChecker<double> ())
   .AddAttribute ("X2",
			  "X2, [m]",
			  DoubleValue (20),
			  MakeDoubleAccessor (&BackForthBetweenTwoPointsMobilityModel::m_x2),
			  MakeDoubleChecker<double> ())
	.AddAttribute ("Y1",
			"Y1, [m]",
			DoubleValue (20),
			MakeDoubleAccessor (&BackForthBetweenTwoPointsMobilityModel::m_y1),
			MakeDoubleChecker<double> ())
	.AddAttribute ("Y2",
	     	"Y2, [m]",
			DoubleValue (20),
			MakeDoubleAccessor (&BackForthBetweenTwoPointsMobilityModel::m_y2),
			MakeDoubleChecker<double> ())
	.AddAttribute("PauseTime",
			"after reaching an edge, wait for a while to avoid ping-pong",
			DoubleValue(0),
			MakeDoubleAccessor (&BackForthBetweenTwoPointsMobilityModel::m_pauseTime),
			MakeDoubleChecker<double> ())
	.AddAttribute("InitialTime",
			"Initial time for each user, each UE have different initial time to avoid simultaneously handover",
			DoubleValue(0),
			MakeDoubleAccessor (&BackForthBetweenTwoPointsMobilityModel::m_initTime),
			MakeDoubleChecker<double> ())
    ;

  return tid;
}

BackForthBetweenTwoPointsMobilityModel::BackForthBetweenTwoPointsMobilityModel ()
{
  m_speed = CreateObject<UniformRandomVariable> ();
  alreadyStarted = false;
//  m_pause = CreateObject<UniformRandomVariable> ();
}

void
BackForthBetweenTwoPointsMobilityModel::DoInitialize (void)
{
	NS_ASSERT (m_minSpeed >= 1e-6);
//	NS_LOG_LOGIC("BackForthBetweenTwoPointsMobilityModel::DoInitialize MinSpeed " << m_minSpeed << " MaxSpeed " << m_maxSpeed);
	NS_ASSERT (m_minSpeed <= m_maxSpeed);

	m_speed->SetAttribute ("Min", DoubleValue (m_minSpeed));
	m_speed->SetAttribute ("Max", DoubleValue (m_maxSpeed));
	m_randGen = CreateObject<UniformRandomVariable>();

	m_helper.Update ();
	m_helper.Pause ();

	//horizontal move -> choose random y and fix x
	m_1or2 = false;//1:false true 2: true

	if(m_randGen->GetValue() >= 0.5)
	{//1 first
		m_helper.SetPosition(Vector(m_x1, m_y1, m_z));
	}
	else
	{//2 first
		m_1or2 = true;
		m_helper.SetPosition(Vector(m_x2, m_y2, m_z));
	}

	NS_ASSERT (!m_event.IsRunning ());
//	m_event = Simulator::ScheduleNow (&BackForthBetweenTwoPointsMobilityModel::BeginWalk, this);
	m_event = Simulator::ScheduleNow (&BackForthBetweenTwoPointsMobilityModel::PauseAtStart, this);
	alreadyStarted = true;
	NotifyCourseChange ();
	MobilityModel::DoInitialize ();
}

void
BackForthBetweenTwoPointsMobilityModel::PauseAtStart()
{
//	m_helper.Update ();
//	m_helper.Pause ();
	uint64_t iniTime = m_randGen->GetInteger(5,m_initTime);
//	NS_LOG_UNCOND("iniTime" << iniTime);
	Time pause = MilliSeconds (iniTime);
	m_event = Simulator::Schedule (pause, &BackForthBetweenTwoPointsMobilityModel::BeginWalk, this);
//	alreadyStarted = true;
	NotifyCourseChange ();
}

void
BackForthBetweenTwoPointsMobilityModel::BeginWalk (void)
{
	m_helper.Update ();
	m_helper.Pause ();
	Vector current = m_helper.GetCurrentPosition ();
	Vector destination;
	if(!m_1or2)
	{
		m_1or2 = true;
		destination.x = m_x2;
		destination.y = m_y2;
		destination.z = m_z;
	}
	else
	{
		m_1or2 = false;
		destination.x = m_x1;
		destination.y = m_y1;
		destination.z = m_z;
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
			&BackForthBetweenTwoPointsMobilityModel::PauseAtEdge, this);
	NotifyCourseChange ();
}

void
BackForthBetweenTwoPointsMobilityModel::PauseAtEdge()
{
	m_helper.Update ();
	m_helper.Pause ();
	Time pause = Seconds (m_pauseTime);
	m_event = Simulator::Schedule (pause, &BackForthBetweenTwoPointsMobilityModel::BeginWalk, this);
	NotifyCourseChange ();
}

Vector
BackForthBetweenTwoPointsMobilityModel::DoGetPosition (void) const
{
	m_helper.Update ();
	return m_helper.GetCurrentPosition ();
}

void
BackForthBetweenTwoPointsMobilityModel::DoSetPosition (const Vector &position)
{
	if (alreadyStarted)
	{
		m_helper.SetPosition (position);
		Simulator::Remove (m_event);
		m_event = Simulator::ScheduleNow (&BackForthBetweenTwoPointsMobilityModel::BeginWalk, this);
	}
}

Vector
BackForthBetweenTwoPointsMobilityModel::DoGetVelocity (void) const
{
	return m_helper.GetVelocity ();
}

int64_t
BackForthBetweenTwoPointsMobilityModel::DoAssignStreams (int64_t stream)
{
	m_speed->SetStream (stream);
	m_randGen->SetStream(stream+1);
	return 3;
}

}  // namespace ns3


