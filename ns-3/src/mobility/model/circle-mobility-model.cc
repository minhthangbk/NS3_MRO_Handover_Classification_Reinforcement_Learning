/*
 * circle-mobility-model.cc
 *
 *  Created on: Jan 24, 2016
 *      Author: thang
 */

#include <cmath>
#include "ns3/simulator.h"
#include "ns3/double.h"
#include "ns3/boolean.h"
#include "circle-mobility-model.h"
#include <ns3/fatal-error.h>
#include <ns3/log.h>
#include <math.h>


namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (CircleMobilityModel);

TypeId
CircleMobilityModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CircleMobilityModel")
    .SetParent<MobilityModel> ()
    .SetGroupName ("Mobility")
    .AddConstructor<CircleMobilityModel> ()
    .AddAttribute ("MinSpeed",
                   "Minimum speed value, [m/s]",
                   DoubleValue (0.3),
                   MakeDoubleAccessor (&CircleMobilityModel::m_minSpeed),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MaxSpeed",
                   "Maximum speed value, [m/s]",
                   DoubleValue (0.7),
                   MakeDoubleAccessor (&CircleMobilityModel::m_maxSpeed),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Z",
                   "Z value of traveling region (fixed), [m]",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&CircleMobilityModel::m_z),
                   MakeDoubleChecker<double> ())
   .AddAttribute ("Radius",
				  "maximum radius of the doughnut, [m]",
				  DoubleValue (20),
				  MakeDoubleAccessor (&CircleMobilityModel::m_radius),
				  MakeDoubleChecker<double> ())
   .AddAttribute ("CenterX",
				"Center X",
				DoubleValue (0.0),
				MakeDoubleAccessor (&CircleMobilityModel::m_centerX),
				MakeDoubleChecker<double> ())
  .AddAttribute ("CenterY",
		  	   "Center Y",
			   DoubleValue (0.0),
			   MakeDoubleAccessor (&CircleMobilityModel::m_centerY),
			   MakeDoubleChecker<double> ())
  .AddAttribute ("Clockwise",
  		  	   "Enable Clockwise",
  			   BooleanValue (false),
  			   MakeBooleanAccessor (&CircleMobilityModel::m_clockwise),
  			   MakeBooleanChecker());

  return tid;
}

CircleMobilityModel::CircleMobilityModel () :
  alreadyStarted (false)
{
  m_speed = CreateObject<UniformRandomVariable> ();
//  m_pause = CreateObject<UniformRandomVariable> ();
}

void
CircleMobilityModel::DoInitialize (void)
{
	NS_ASSERT (m_minSpeed >= 1e-6);
	NS_ASSERT (m_minSpeed <= m_maxSpeed);
	alreadyStarted = true;
	m_speed->SetAttribute ("Min", DoubleValue (m_minSpeed));
	m_speed->SetAttribute ("Max", DoubleValue (m_maxSpeed));
	m_position = CreateObject<RandomDiscPositionAllocator> ();
	m_position->SetX (m_centerX);//Set the center of the disc
	m_position->SetY (m_centerY);
	m_position->SetMaxRho(m_radius);	//Set the min radius for random variable
	m_position->SetMinRho(m_radius);	//Set the max radius for random variable
	m_helper.Update ();
	m_helper.Pause ();
	m_helper.SetPosition(m_position->GetNext());
	m_nxtThe = m_position->GetCurrentTheta();
	NS_ASSERT (!m_event.IsRunning ());
//	m_event = Simulator::Schedule(Seconds (0.01*m_radius/m_speed),&CircleMobilityModel::BeginWalk, this, m_position->GetNext());
	m_event = Simulator::ScheduleNow (&CircleMobilityModel::BeginWalk, this);
	NotifyCourseChange ();
	MobilityModel::DoInitialize ();
}


//void
//CircleMobilityModel::SteadyStateBeginWalk (const Vector &destination)
//{
//  m_helper.Update ();
//  Vector m_current = m_helper.GetCurrentPosition ();
//  double u = m_u_r->GetValue (0, 1);
//  double speed = std::pow (m_maxSpeed, u)/std::pow (m_minSpeed, u - 1);
//  double dx = (destination.x - m_current.x);
//  double dy = (destination.y - m_current.y);
//  double dz = (destination.z - m_current.z);
//  double k = speed / std::sqrt (dx*dx + dy*dy + dz*dz);
//
//  m_helper.SetVelocity (Vector (k*dx, k*dy, k*dz));
//  m_helper.Unpause ();
//  Time travelDelay = Seconds (CalculateDistance (destination, m_current) / speed);
//  m_event = Simulator::Schedule (travelDelay,
//                                 &CircleMobilityModel::Start, this);
//  NotifyCourseChange ();
//}

void
CircleMobilityModel::BeginWalk (void)
{
  m_helper.Update ();
  m_helper.Pause ();
  Vector m_current = m_helper.GetCurrentPosition ();
  Vector destination;

  if(m_clockwise == false){
	  m_nxtThe += 0.01;
  }else{
	  m_nxtThe -= 0.01;
  }
  destination.x = m_centerX + m_radius*std::cos(m_nxtThe);//counter clock wise
  destination.y = m_centerY + m_radius*std::sin(m_nxtThe);
  destination.z = m_current.z;//only move within X-Y plan
  double speed = m_speed->GetValue ();
  double dx = (destination.x - m_current.x);
  double dy = (destination.y - m_current.y);
  double dz = (destination.z - m_current.z);
  double k = speed / std::sqrt (dx*dx + dy*dy + dz*dz);

  m_helper.SetVelocity (Vector (k*dx, k*dy, k*dz));
  m_helper.Unpause ();

  Time travelDelay = Seconds (CalculateDistance (destination, m_current) / speed);
  m_event = Simulator::Schedule (travelDelay,
                                 &CircleMobilityModel::BeginWalk, this);
  NotifyCourseChange ();
}

//void
//CircleMobilityModel::Start (void)
//{
//  m_helper.Update ();
//  m_helper.Pause ();
//  Time pause = Seconds (m_pause->GetValue ());
//  m_event = Simulator::Schedule (pause, &CircleMobilityModel::BeginWalk, this);//Create a loop
//  NotifyCourseChange ();
//}

Vector
CircleMobilityModel::DoGetPosition (void) const
{
  m_helper.Update ();
  return m_helper.GetCurrentPosition ();
}

void
CircleMobilityModel::DoSetPosition (const Vector &position)
{
  if (alreadyStarted)
    {
      m_helper.SetPosition (position);
      Simulator::Remove (m_event);
      m_event = Simulator::ScheduleNow (&CircleMobilityModel::BeginWalk, this);
    }
}

Vector
CircleMobilityModel::DoGetVelocity (void) const
{
  return m_helper.GetVelocity ();
}

int64_t
CircleMobilityModel::DoAssignStreams (int64_t stream)
{
  int64_t positionStreamsAllocated = 0;
  m_speed->SetStream (stream);
//  m_pause->SetStream (stream + 1);
//  m_u_r->SetStream (stream + 6);
  positionStreamsAllocated = m_position->AssignStreams (stream + 1);
  return (1 + positionStreamsAllocated);
}

}  // namespace ns3
