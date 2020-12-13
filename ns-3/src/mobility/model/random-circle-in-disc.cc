/*
 * random-circle-in-disc.cc
 *
 *  Created on: Aug 24, 2016
 *      Author: thang
 */

#include <cmath>
#include "ns3/simulator.h"
#include "ns3/double.h"
#include "random-circle-in-disc.h"
#include <ns3/fatal-error.h>
#include <ns3/log.h>
#include <math.h>
#include <ns3/boolean.h>

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (RandomCircleInDisc);

TypeId
RandomCircleInDisc::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RandomCircleInDisc")
    .SetParent<MobilityModel> ()
    .SetGroupName ("Mobility")
    .AddConstructor<RandomCircleInDisc> ()
//    .AddAttribute ("MinSpeed",
//                   "Minimum speed value, [m/s]",
//                   DoubleValue (0.3),
//                   MakeDoubleAccessor (&RandomCircleInDisc::m_minSpeed),
//                   MakeDoubleChecker<double> ())
//    .AddAttribute ("MaxSpeed",
//                   "Maximum speed value, [m/s]",
//                   DoubleValue (0.7),
//                   MakeDoubleAccessor (&RandomCircleInDisc::m_maxSpeed),
//                   MakeDoubleChecker<double> ())
	.AddAttribute("UserSpeed",
            "speed value, [m/s]",
            DoubleValue (1),
            MakeDoubleAccessor (&RandomCircleInDisc::m_speed),
            MakeDoubleChecker<double> ())
    .AddAttribute ("Z",
            "Z value of traveling region (fixed), [m]",
            DoubleValue (0.0),
            MakeDoubleAccessor (&RandomCircleInDisc::m_z),
            MakeDoubleChecker<double> ())
   .AddAttribute ("DiscCenterX",
			"Center X of the disc, [m]",
			DoubleValue (20),
			MakeDoubleAccessor (&RandomCircleInDisc::m_discCenterX),
			MakeDoubleChecker<double> ())
   .AddAttribute ("DiscCenterY",
		   "Center Y of the disc, [m]",
			DoubleValue (20),
			MakeDoubleAccessor (&RandomCircleInDisc::m_discCenterY),
			MakeDoubleChecker<double> ())
	.AddAttribute ("MinDiscRadius",
			"Minimum Radius of the disc [m]",
			DoubleValue (20),
			MakeDoubleAccessor (&RandomCircleInDisc::m_minDiscR),
			MakeDoubleChecker<double> ())
	.AddAttribute ("MaxDiscRadius",
			"Maximum Radius of the disc [m]",
			DoubleValue (20),
			MakeDoubleAccessor (&RandomCircleInDisc::m_maxDiscR),
			MakeDoubleChecker<double> ())
	.AddAttribute ("UeRadius",
	    	"Circle Radius of Ue [m]",
			DoubleValue (10),
			MakeDoubleAccessor (&RandomCircleInDisc::m_UeR),
			MakeDoubleChecker<double> ())
    ;

  return tid;
}

RandomCircleInDisc::RandomCircleInDisc ()
{
//  m_speed = CreateObject<UniformRandomVariable> ();
  alreadyStarted = false;
//  m_pause = CreateObject<UniformRandomVariable> ();
}

void
RandomCircleInDisc::DoInitialize (void)
{
	m_discR = CreateObject<UniformRandomVariable>();
	m_discR->SetAttribute("Min",DoubleValue(m_minDiscR));
	m_discR->SetAttribute("Max",DoubleValue(m_maxDiscR));

	m_discTheta = CreateObject<UniformRandomVariable>();
	m_discTheta->SetAttribute("Min",DoubleValue(0));
	m_discTheta->SetAttribute("Max",DoubleValue(6.283));

	m_UeTheta = CreateObject<UniformRandomVariable>();
	m_UeTheta->SetAttribute("Min",DoubleValue(0));
	m_UeTheta->SetAttribute("Max",DoubleValue(6.283));

	double theta = m_discTheta->GetValue();
	double r = m_discR->GetValue();
	m_UeCenterX = m_discCenterX + r*(std::cos(theta));//Center of the circle
	m_UeCenterY = m_discCenterY + r*(std::sin(theta));

	m_helper.Update ();
	m_helper.Pause ();

	m_curAlpha = m_UeTheta->GetValue();

	double UeStartX = m_UeCenterX + m_UeR*std::cos(m_curAlpha);
	double UeStartY = m_UeCenterY + m_UeR*std::sin(m_curAlpha);

	m_helper.SetPosition(Vector(UeStartX, UeStartY, 0)); //have to set the initial position for UE
	//horizontal move -> choose random y and fix x

//	NS_LOG_UNCOND("DiscR " << r << " DiscCenterX " << m_discCenterX << " DiscCenterY "
//			<< m_discCenterY << " UeCenterX " << m_UeCenterX << " UeCenterY " << m_UeCenterY);

	NS_ASSERT (!m_event.IsRunning ());
	m_event = Simulator::ScheduleNow (&RandomCircleInDisc::BeginWalk, this);
	alreadyStarted = true;
	NotifyCourseChange ();
	MobilityModel::DoInitialize ();
}

void
RandomCircleInDisc::BeginWalk (void)
{
  m_helper.Update ();
  m_helper.Pause ();
  Vector current = m_helper.GetCurrentPosition ();

  m_curAlpha += 0.2; //radian
  Vector destination;
  destination.x = m_UeCenterX + m_UeR*(std::cos(m_curAlpha));
  destination.y = m_UeCenterY + m_UeR*(std::sin(m_curAlpha));
  destination.z = m_z;

  double dx = destination.x-current.x;
  double dy = destination.y-current.y;
  double dz = 0;
  double k = m_speed / std::sqrt (dx*dx + dy*dy + dz*dz);

  m_helper.SetVelocity (Vector (k*dx, k*dy, k*dz));
  m_helper.Unpause ();

  Time travelDelay = Seconds (CalculateDistance (destination, current) / m_speed);

//  NS_LOG_UNCOND("m_UeCenterX " << m_UeCenterX << " m_UeCenterY " << m_UeCenterY
//  		  << " destination.x " << destination.x << " destination.y " << destination.y << " travelDelay " << travelDelay.GetSeconds());

  m_event = Simulator::Schedule (travelDelay,
                                 &RandomCircleInDisc::BeginWalk, this);
  NotifyCourseChange ();
}

Vector
RandomCircleInDisc::DoGetPosition (void) const{
  m_helper.Update ();
  return m_helper.GetCurrentPosition ();
}

void
RandomCircleInDisc::DoSetPosition (const Vector &position){
  if (alreadyStarted)
    {
      m_helper.SetPosition (position);
      Simulator::Remove (m_event);
      m_event = Simulator::ScheduleNow (&RandomCircleInDisc::BeginWalk, this);
    }
}

Vector
RandomCircleInDisc::DoGetVelocity (void) const{
  return m_helper.GetVelocity ();
}

int64_t
RandomCircleInDisc::DoAssignStreams (int64_t stream){
//  m_speed->SetStream (stream);
  m_discR->SetStream(stream);
  m_discTheta->SetStream(stream+1);
  m_UeTheta->SetStream(stream+2);
  return 3;
}

}  // namespace ns3


