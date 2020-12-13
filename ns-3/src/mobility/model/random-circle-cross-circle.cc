/*
 * random-circle-cross-circle.cc
 *
 *  Created on: Dec 26, 2016
 *      Author: thang
 */

#include <cmath>
#include "ns3/simulator.h"
#include "ns3/double.h"
#include "random-circle-cross-circle.h"
#include <ns3/fatal-error.h>
#include <ns3/log.h>
#include <math.h>
#include <ns3/boolean.h>

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (RandomCircleCrossCircle);

TypeId
RandomCircleCrossCircle::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RandomCircleCrossCircle")
    .SetParent<MobilityModel> ()
    .SetGroupName ("Mobility")
    .AddConstructor<RandomCircleCrossCircle> ()
//    .AddAttribute ("MinSpeed",
//                   "Minimum speed value, [m/s]",
//                   DoubleValue (0.3),
//                   MakeDoubleAccessor (&RandomCircleCrossCircle::m_minSpeed),
//                   MakeDoubleChecker<double> ())
//    .AddAttribute ("MaxSpeed",
//                   "Maximum speed value, [m/s]",
//                   DoubleValue (0.7),
//                   MakeDoubleAccessor (&RandomCircleCrossCircle::m_maxSpeed),
//                   MakeDoubleChecker<double> ())
	.AddAttribute("UserSpeed",
            "speed value, [m/s]",
            DoubleValue (1),
            MakeDoubleAccessor (&RandomCircleCrossCircle::m_speed),
            MakeDoubleChecker<double> ())
    .AddAttribute ("Z",
            "Z value of traveling region (fixed), [m]",
            DoubleValue (0.0),
            MakeDoubleAccessor (&RandomCircleCrossCircle::m_z),
            MakeDoubleChecker<double> ())
   .AddAttribute ("CircleCenterX",
			"Center X of the disc, [m]",
			DoubleValue (20),
			MakeDoubleAccessor (&RandomCircleCrossCircle::m_c1CenterX),
			MakeDoubleChecker<double> ())
   .AddAttribute ("CircleCenterY",
		   "Center Y of the disc, [m]",
			DoubleValue (20),
			MakeDoubleAccessor (&RandomCircleCrossCircle::m_c1CenterY),
			MakeDoubleChecker<double> ())
	.AddAttribute ("CircleRadius",
			"Radius of the disc [m]",
			DoubleValue (20),
			MakeDoubleAccessor (&RandomCircleCrossCircle::m_c1R),
			MakeDoubleChecker<double> ())
//	.AddAttribute ("MaxDiscRadius",
//			"Maximum Radius of the disc [m]",
//			DoubleValue (20),
//			MakeDoubleAccessor (&RandomCircleCrossCircle::m_maxDiscR),
//			MakeDoubleChecker<double> ())
	.AddAttribute ("UeRadius",
	    	"Circle Radius of Ue [m]",
			DoubleValue (10),
			MakeDoubleAccessor (&RandomCircleCrossCircle::m_ueR),
			MakeDoubleChecker<double> ())
    ;

  return tid;
}

RandomCircleCrossCircle::RandomCircleCrossCircle ()
{
//  m_speed = CreateObject<UniformRandomVariable> ();
  alreadyStarted = false;
  m_c2Theta = CreateObject<UniformRandomVariable>();
  m_ueTheta = CreateObject<UniformRandomVariable>();
//  m_pause = CreateObject<UniformRandomVariable> ();
}

void
RandomCircleCrossCircle::DoInitialize (void)
{
	m_c2R = CreateObject<UniformRandomVariable>();
	m_c2R->SetAttribute("Min",DoubleValue(std::abs(m_c1R-m_ueR)));
	m_c2R->SetAttribute("Max",DoubleValue(m_c1R+m_ueR));

//	m_c2Theta = CreateObject<UniformRandomVariable>();
	m_c2Theta->SetAttribute("Min",DoubleValue(0));
	m_c2Theta->SetAttribute("Max",DoubleValue(6.283));

//	m_UeTheta = CreateObject<UniformRandomVariable>();
	m_ueTheta->SetAttribute("Min",DoubleValue(0));
	m_ueTheta->SetAttribute("Max",DoubleValue(6.283));

	double c2R = m_c2R->GetValue();
	double c2Theta = m_c2Theta->GetValue();
	m_ueCenterX = m_c1CenterX + c2R*(std::cos(c2Theta));//Center of the circle
	m_ueCenterY = m_c1CenterY + c2R*(std::sin(c2Theta));

	m_helper.Update ();
	m_helper.Pause ();

	m_ueCurTheta = m_ueTheta->GetValue();

	double UeStartX = m_ueCenterX + m_ueR*std::cos(m_ueCurTheta);
	double UeStartY = m_ueCenterY + m_ueR*std::sin(m_ueCurTheta);

	m_helper.SetPosition(Vector(UeStartX, UeStartY, 0)); //have to set the initial position for UE
	//horizontal move -> choose random y and fix x

//	NS_LOG_UNCOND("DiscR " << r << " DiscCenterX " << m_discCenterX << " DiscCenterY "
//			<< m_discCenterY << " UeCenterX " << m_UeCenterX << " UeCenterY " << m_UeCenterY);

	NS_ASSERT (!m_event.IsRunning ());
	m_event = Simulator::ScheduleNow (&RandomCircleCrossCircle::BeginWalk, this);
	alreadyStarted = true;
	NotifyCourseChange ();
	MobilityModel::DoInitialize ();
}

void
RandomCircleCrossCircle::BeginWalk (void)
{
  m_helper.Update ();
  m_helper.Pause ();
  Vector current = m_helper.GetCurrentPosition ();

  m_ueCurTheta += 0.2; //radian
  Vector destination;
  destination.x = m_ueCenterX + m_ueR*(std::cos(m_ueCurTheta));
  destination.y = m_ueCenterY + m_ueR*(std::sin(m_ueCurTheta));
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
                                 &RandomCircleCrossCircle::BeginWalk, this);
  NotifyCourseChange ();
}

Vector
RandomCircleCrossCircle::DoGetPosition (void) const{
  m_helper.Update ();
  return m_helper.GetCurrentPosition ();
}

void
RandomCircleCrossCircle::DoSetPosition (const Vector &position){
  if (alreadyStarted)
    {
      m_helper.SetPosition (position);
      Simulator::Remove (m_event);
      m_event = Simulator::ScheduleNow (&RandomCircleCrossCircle::BeginWalk, this);
    }
}

Vector
RandomCircleCrossCircle::DoGetVelocity (void) const{
  return m_helper.GetVelocity ();
}

int64_t
RandomCircleCrossCircle::DoAssignStreams (int64_t stream){
//  m_speed->SetStream (stream);
  m_c2R->SetStream(stream);
  m_c2Theta->SetStream(stream+1);
  m_ueTheta->SetStream(stream+2);
  return 3;
}

}  // namespace ns3




