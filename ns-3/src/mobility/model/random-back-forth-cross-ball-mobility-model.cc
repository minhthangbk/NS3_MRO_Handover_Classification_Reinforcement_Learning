/*
 * random-back-forth-cross-ball-mobility-model.cc
 *
 *  Created on: Jan 25, 2016
 *      Author: thang
 */

#include <cmath>
#include "ns3/simulator.h"
#include "ns3/double.h"
#include "random-back-forth-cross-ball-mobility-model.h"
#include <ns3/fatal-error.h>
#include <ns3/log.h>
#include <math.h>
#include <ns3/boolean.h>

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (RandomBackForthCrossBallMobilityModel);

TypeId
RandomBackForthCrossBallMobilityModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RandomBackForthCrossBallMobilityModel")
    .SetParent<MobilityModel> ()
    .SetGroupName ("Mobility")
    .AddConstructor<RandomBackForthCrossBallMobilityModel> ()
    .AddAttribute ("MinSpeed",
                   "Minimum speed value, [m/s]",
                   DoubleValue (0.3),
                   MakeDoubleAccessor (&RandomBackForthCrossBallMobilityModel::m_minSpeed),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MaxSpeed",
                   "Maximum speed value, [m/s]",
                   DoubleValue (0.7),
                   MakeDoubleAccessor (&RandomBackForthCrossBallMobilityModel::m_maxSpeed),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Z",
                   "Z value of traveling region (fixed), [m]",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&RandomBackForthCrossBallMobilityModel::m_z),
                   MakeDoubleChecker<double> ())
   .AddAttribute ("MinX",
				  "minimum X, [m]",
				  DoubleValue (20),
				  MakeDoubleAccessor (&RandomBackForthCrossBallMobilityModel::m_minX),
				  MakeDoubleChecker<double> ())
   .AddAttribute ("MaxX",
			  "maximum X, [m]",
			  DoubleValue (20),
			  MakeDoubleAccessor (&RandomBackForthCrossBallMobilityModel::m_maxX),
			  MakeDoubleChecker<double> ())
	.AddAttribute ("MinY",
			"minimum Y, [m]",
			DoubleValue (20),
			MakeDoubleAccessor (&RandomBackForthCrossBallMobilityModel::m_minY),
			MakeDoubleChecker<double> ())
	.AddAttribute ("MaxY",
	     	"maximum Y, [m]",
			DoubleValue (20),
			MakeDoubleAccessor (&RandomBackForthCrossBallMobilityModel::m_maxY),
			MakeDoubleChecker<double> ())
	.AddAttribute ("BallCenterX",
			"center of the ball, [m]",
			DoubleValue (20),
			MakeDoubleAccessor (&RandomBackForthCrossBallMobilityModel::m_ballCenterX),
			MakeDoubleChecker<double> ())
	.AddAttribute ("BallCenterY",
			"center of the ball, [m]",
			DoubleValue (20),
			MakeDoubleAccessor (&RandomBackForthCrossBallMobilityModel::m_ballCenterY),
			MakeDoubleChecker<double> ())
	.AddAttribute ("BallRadius",
	    	"maximum Y, [m]",
			DoubleValue (20),
			MakeDoubleAccessor (&RandomBackForthCrossBallMobilityModel::m_ballRadius),
			MakeDoubleChecker<double> ())
    .AddAttribute("Vertical",
    		"Moving Mode, false: horizontal, true: vertical",
			BooleanValue(false),
			MakeBooleanAccessor(&RandomBackForthCrossBallMobilityModel::m_verOrHor),
			MakeBooleanChecker())
	.AddAttribute("PauseTime",
			"after reaching an edge, wait for a while to avoid ping-pong",
			DoubleValue(0),
			MakeDoubleAccessor (&RandomBackForthCrossBallMobilityModel::m_pauseTime),
			MakeDoubleChecker<double> ())
    ;

  return tid;
}

RandomBackForthCrossBallMobilityModel::RandomBackForthCrossBallMobilityModel ()
{
  m_speed = CreateObject<UniformRandomVariable> ();
  alreadyStarted = false;
//  m_pause = CreateObject<UniformRandomVariable> ();
}

void
RandomBackForthCrossBallMobilityModel::DoInitialize (void)
{
	NS_ASSERT (m_minSpeed >= 1e-6);
//	NS_LOG_LOGIC("RandomBackForthCrossBallMobilityModel::DoInitialize MinSpeed " << m_minSpeed << " MaxSpeed " << m_maxSpeed);
	NS_ASSERT (m_minSpeed <= m_maxSpeed);
	NS_ASSERT(m_minX <= m_maxX);
	NS_ASSERT(m_minY <= m_maxY);
	NS_ASSERT((m_ballCenterX+m_ballRadius) < m_maxX);//The ball have to be located inside a rectangular
	NS_ASSERT((m_ballCenterX-m_ballRadius) > m_minX);
	NS_ASSERT((m_ballCenterY+m_ballRadius) < m_maxY);
	NS_ASSERT((m_ballCenterY-m_ballRadius) > m_minY);
	m_speed->SetAttribute ("Min", DoubleValue (m_minSpeed));
	m_speed->SetAttribute ("Max", DoubleValue (m_maxSpeed));
	m_x = CreateObject<UniformRandomVariable>();	m_x->SetAttribute("Min",DoubleValue(m_minX)); 	m_x->SetAttribute("Max",DoubleValue(m_maxX));
	m_y = CreateObject<UniformRandomVariable>();	m_y->SetAttribute("Min",DoubleValue(m_minY));	m_y->SetAttribute("Max",DoubleValue(m_maxY));

	m_helper.Update ();
	m_helper.Pause ();

	//horizontal move -> choose random y and fix x
	m_backOrForth = false;//go forth first

	if(m_verOrHor == true){//vertically
		m_helper.SetPosition(Vector(m_x->GetValue(), m_minY, 0));
	}else{//horizontally
		m_helper.SetPosition(Vector(m_minX, m_y->GetValue(), 0));
	}

	NS_ASSERT (!m_event.IsRunning ());
	m_event = Simulator::ScheduleNow (&RandomBackForthCrossBallMobilityModel::BeginWalk, this);
	alreadyStarted = true;
	NotifyCourseChange ();
	MobilityModel::DoInitialize ();
}

void
RandomBackForthCrossBallMobilityModel::BeginWalk (void)
{
	m_helper.Update ();
	m_helper.Pause ();
	Vector current = m_helper.GetCurrentPosition ();
	double vector1X = m_ballCenterX-current.x;
	double vector1Y = m_ballCenterY-current.y;
	double vector1Len = std::sqrt(std::pow(vector1X,2)+std::pow(vector1Y,2));
	NS_ASSERT(vector1Len > m_ballRadius);
	double alpha1 = std::asin(m_ballRadius/vector1Len);
	Vector destination;
	double alpha2;
	do{
		if(m_backOrForth == false){//go forth
			if(m_verOrHor == true){//vertically
				destination.x = m_x->GetValue();
				destination.y = m_maxY;
				destination.z = current.z;//only move within X-Y plan
			}else{//horizontally
				destination.x = m_maxX;
				destination.y = m_y->GetValue();
				destination.z = current.z;//only move within X-Y plan
			}
		}else{//go back
			if(m_verOrHor == true){//vertically
				destination.x = m_x->GetValue();
				destination.y = m_minY;
				destination.z = current.z;//only move within X-Y plan
			}else{//horizontally
				destination.x = m_minX;
				destination.y = m_y->GetValue();
				destination.z = current.z;//only move within X-Y plan
			}
		}
		double vector2X = destination.x - current.x;
		double vector2Y = destination.y - current.y;
		double vector2Len = std::sqrt(std::pow(vector2X,2)+std::pow(vector2Y,2));
		alpha2 = std::acos((vector2X*vector1X + vector2Y*vector1Y)/(vector2Len*vector1Len));
	}while((alpha2 < 0) && (alpha2 >= alpha1));

	if(m_backOrForth == false){//Update direction for next movement
		m_backOrForth = true;
	}else{
		m_backOrForth = false;
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
			&RandomBackForthCrossBallMobilityModel::PauseAtEdge, this);
	NotifyCourseChange ();
}

void
RandomBackForthCrossBallMobilityModel::PauseAtEdge()
{
	m_helper.Update ();
	m_helper.Pause ();
	Time pause = Seconds (m_pauseTime);
	m_event = Simulator::Schedule (pause, &RandomBackForthCrossBallMobilityModel::BeginWalk, this);
	NotifyCourseChange ();
}

Vector
RandomBackForthCrossBallMobilityModel::DoGetPosition (void) const
{
	m_helper.Update ();
	return m_helper.GetCurrentPosition ();
}

void
RandomBackForthCrossBallMobilityModel::DoSetPosition (const Vector &position)
{
	if (alreadyStarted)
	{
		m_helper.SetPosition (position);
		Simulator::Remove (m_event);
		m_event = Simulator::ScheduleNow (&RandomBackForthCrossBallMobilityModel::BeginWalk, this);
	}
}

Vector
RandomBackForthCrossBallMobilityModel::DoGetVelocity (void) const
{
	return m_helper.GetVelocity ();
}

int64_t
RandomBackForthCrossBallMobilityModel::DoAssignStreams (int64_t stream)
{
	m_speed->SetStream (stream);
	m_x->SetStream(stream+1);
	m_y->SetStream(stream+2);
	return 3;
}

}  // namespace ns3


