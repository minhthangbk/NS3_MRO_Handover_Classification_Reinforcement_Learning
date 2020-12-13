/*
 * random-circle-in-rectangle.cc
 *
 *  Created on: Aug 24, 2016
 *      Author: thang
 */

#include <cmath>
#include "ns3/simulator.h"
#include "ns3/double.h"
#include "random-circle-in-rectangle.h"
#include <ns3/fatal-error.h>
#include <ns3/log.h>
#include <math.h>
#include <ns3/boolean.h>

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (RandomCircleInRectangle);

TypeId
RandomCircleInRectangle::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RandomCircleInRectangle")
    .SetParent<MobilityModel> ()
    .SetGroupName ("Mobility")
    .AddConstructor<RandomCircleInRectangle> ()
//    .AddAttribute ("MinSpeed",
//                   "Minimum speed value, [m/s]",
//                   DoubleValue (0.3),
//                   MakeDoubleAccessor (&RandomCircleInRectangle::m_minSpeed),
//                   MakeDoubleChecker<double> ())
//    .AddAttribute ("MaxSpeed",
//                   "Maximum speed value, [m/s]",
//                   DoubleValue (0.7),
//                   MakeDoubleAccessor (&RandomCircleInRectangle::m_maxSpeed),
//                   MakeDoubleChecker<double> ())
	.AddAttribute("UserSpeed",
            "speed value, [m/s]",
            DoubleValue (1),
            MakeDoubleAccessor (&RandomCircleInRectangle::m_speed),
            MakeDoubleChecker<double> ())
    .AddAttribute ("Z",
            "Z value of traveling region (fixed), [m]",
            DoubleValue (0.0),
            MakeDoubleAccessor (&RandomCircleInRectangle::m_z),
            MakeDoubleChecker<double> ())
   .AddAttribute ("MinX",
			"minimum X, [m]",
			DoubleValue (20),
			MakeDoubleAccessor (&RandomCircleInRectangle::m_minX),
			MakeDoubleChecker<double> ())
   .AddAttribute ("MaxX",
			"maximum X, [m]",
			DoubleValue (20),
			MakeDoubleAccessor (&RandomCircleInRectangle::m_maxX),
			MakeDoubleChecker<double> ())
	.AddAttribute ("MinY",
			"minimum Y, [m]",
			DoubleValue (20),
			MakeDoubleAccessor (&RandomCircleInRectangle::m_minY),
			MakeDoubleChecker<double> ())
	.AddAttribute ("MaxY",
	     	"maximum Y, [m]",
			DoubleValue (20),
			MakeDoubleAccessor (&RandomCircleInRectangle::m_maxY),
			MakeDoubleChecker<double> ())
	.AddAttribute ("CircleRadius",
	    	"Circle Radius [m]",
			DoubleValue (10),
			MakeDoubleAccessor (&RandomCircleInRectangle::m_cR),
			MakeDoubleChecker<double> ())
    ;

  return tid;
}

RandomCircleInRectangle::RandomCircleInRectangle ()
{
//  m_speed = CreateObject<UniformRandomVariable> ();
  alreadyStarted = false;
//  m_pause = CreateObject<UniformRandomVariable> ();
}

void
RandomCircleInRectangle::DoInitialize (void)
{
//	NS_LOG_LOGIC("RandomCircleInRectangle::DoInitialize MinSpeed " << m_minSpeed << " MaxSpeed " << m_maxSpeed);
	NS_ASSERT(m_minX <= m_maxX);
	NS_ASSERT(m_minY <= m_maxY);

	m_randX = CreateObject<UniformRandomVariable>();	m_randX->SetAttribute("Min",DoubleValue(m_minX)); 	m_randX->SetAttribute("Max",DoubleValue(m_maxX));
	m_randY = CreateObject<UniformRandomVariable>();	m_randY->SetAttribute("Min",DoubleValue(m_minY));	m_randY->SetAttribute("Max",DoubleValue(m_maxY));

	m_cX = m_randX->GetValue();//Center of the circle
	m_cY = m_randY->GetValue();

	m_helper.Update ();
	m_helper.Pause ();

	m_curAlpha = 0;
	//horizontal move -> choose random y and fix x

	NS_ASSERT (!m_event.IsRunning ());
	m_event = Simulator::ScheduleNow (&RandomCircleInRectangle::BeginWalk, this);
	alreadyStarted = true;
	NotifyCourseChange ();
	MobilityModel::DoInitialize ();
}

void
RandomCircleInRectangle::BeginWalk (void)
{
  m_helper.Update ();
  m_helper.Pause ();
  Vector current = m_helper.GetCurrentPosition ();

  m_curAlpha += 0.2; //radian
  Vector destination;
  destination.x = m_cX + m_cR*std::cos(m_curAlpha);
  destination.y = m_cY + m_cR*std::sin(m_curAlpha);

  double dx = destination.x-current.x;
  double dy = destination.y-current.y;
  double dz = current.z;
  double k = m_speed / std::sqrt (dx*dx + dy*dy + dz*dz);

  m_helper.SetVelocity (Vector (k*dx, k*dy, k*dz));
  m_helper.Unpause ();

  Time travelDelay = Seconds (CalculateDistance (destination, current) / m_speed);
  m_event = Simulator::Schedule (travelDelay,
                                 &RandomCircleInRectangle::BeginWalk, this);
  NotifyCourseChange ();
}

Vector
RandomCircleInRectangle::DoGetPosition (void) const
{
  m_helper.Update ();
  return m_helper.GetCurrentPosition ();
}

void
RandomCircleInRectangle::DoSetPosition (const Vector &position)
{
  if (alreadyStarted)
    {
      m_helper.SetPosition (position);
      Simulator::Remove (m_event);
      m_event = Simulator::ScheduleNow (&RandomCircleInRectangle::BeginWalk, this);
    }
}

Vector
RandomCircleInRectangle::DoGetVelocity (void) const
{
  return m_helper.GetVelocity ();
}

int64_t
RandomCircleInRectangle::DoAssignStreams (int64_t stream)
{
//  m_speed->SetStream (stream);
  m_randX->SetStream(stream);
  m_randY->SetStream(stream+1);
  return 3;
}

}  // namespace ns3


