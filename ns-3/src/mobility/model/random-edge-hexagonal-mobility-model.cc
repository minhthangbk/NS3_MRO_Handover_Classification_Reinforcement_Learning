/*
 * random-edge-hexagonal-mobility-model.cc
 *
 *  Created on: Mar 28, 2016
 *      Author: thang
 */
#include <cmath>
#include "ns3/simulator.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"
#include "random-edge-hexagonal-mobility-model.h"
#include <ns3/fatal-error.h>
#include <ns3/log.h>
#include <math.h>
#include <ns3/boolean.h>


namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (RandomEdgeHexagonalMobilityModel);

TypeId
RandomEdgeHexagonalMobilityModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RandomEdgeHexagonalMobilityModel")
    .SetParent<MobilityModel> ()
    .SetGroupName ("Mobility")
    .AddConstructor<RandomEdgeHexagonalMobilityModel> ()
    .AddAttribute ("MinSpeed",
                   "Minimum speed value, [m/s]",
                   DoubleValue (0.3),
                   MakeDoubleAccessor (&RandomEdgeHexagonalMobilityModel::m_minSpeed),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MaxSpeed",
                   "Maximum speed value, [m/s]",
                   DoubleValue (0.7),
                   MakeDoubleAccessor (&RandomEdgeHexagonalMobilityModel::m_maxSpeed),
                   MakeDoubleChecker<double> ())
   .AddAttribute ("Radius",
				  "[m]",
				  DoubleValue (20),
				  MakeDoubleAccessor (&RandomEdgeHexagonalMobilityModel::m_R),
				  MakeDoubleChecker<double> ())
   .AddAttribute ("CenterX",
			  	  "X coordination of the center",
				  DoubleValue (20),
				  MakeDoubleAccessor (&RandomEdgeHexagonalMobilityModel::m_centerX),
				  MakeDoubleChecker<double> ())
	.AddAttribute ("CenterY",
				"Y coordination of the center",
					DoubleValue (20),
					MakeDoubleAccessor (&RandomEdgeHexagonalMobilityModel::m_centerY),
					MakeDoubleChecker<double> ())
	.AddAttribute("PauseTime",
				"after reaching an edge, wait for a while to avoid ping-pong",
				DoubleValue(0),
				MakeDoubleAccessor (&RandomEdgeHexagonalMobilityModel::m_pauseTime),
				MakeDoubleChecker<double> ())
    ;

  return tid;
}

RandomEdgeHexagonalMobilityModel::RandomEdgeHexagonalMobilityModel ()
{
	m_z = 0;
	alreadyStarted = false;
}

void
RandomEdgeHexagonalMobilityModel::DoInitialize (void)
{
	NS_ASSERT (m_minSpeed >= 1e-6);
	//	NS_LOG_LOGIC("RandomEdgeHexagonalMobilityModel::DoInitialize MinSpeed " << m_minSpeed << " MaxSpeed " << m_maxSpeed);
	NS_ASSERT (m_minSpeed <= m_maxSpeed);

	m_speedRV = CreateObject<UniformRandomVariable> ();
	m_speedRV->SetAttribute ("Min", DoubleValue (m_minSpeed));
	m_speedRV->SetAttribute ("Max", DoubleValue (m_maxSpeed));

	//Choose rectangle
	m_edgeRV = CreateObject<UniformRandomVariable>();
	m_edgeRV->SetAttribute("Min",DoubleValue(0));
	m_edgeRV->SetAttribute("Max",DoubleValue(6));//2*pi

	m_x = CreateObject<UniformRandomVariable> ();

	m_helper.Update ();
	m_helper.Pause ();

	//Set up initial point
	double startX, startY;
	m_prevEdge = 10;//Initial value for this variable should be different to 0,1,2,3,4,5
	double toss= m_edgeRV->GetValue();
	if(toss<1){
		startX = m_centerX+m_x->GetValue(m_R/2, m_R);
		startY = (m_centerY+0)+(-1.732)*(startX-(m_centerX+m_R));
		m_curEdge = 0;

	}else if((1 <= toss) && (toss<2)){
		startX = m_centerX+m_x->GetValue(m_R/2, m_R/2);
		startY = m_centerY+m_R*(1.732)/2;
		m_curEdge = 1;

	}else if((2 <= toss) && (toss<3)){
		startX = m_centerX + m_x->GetValue(-m_R, -m_R/2);
		startY = m_centerY +(1.732)*(startX-(m_centerX-m_R));
		m_curEdge = 2;

	}else if((1 <= toss) && (toss<4)){
		startX = m_centerX+m_x->GetValue(-m_R, -m_R/2);
		startY = m_centerY+(-1.732)*(startX-(m_centerX-m_R));
		m_curEdge = 3;

	}else if((1 <= toss) && (toss<5)){
		startX = m_centerX+m_x->GetValue(-m_R/2, m_R/2);
		startY = m_centerY-m_R*(1.732/2);
		m_curEdge = 4;

	}else{// if((1 << m_edgeRV) && (m_edgeRV<6)){
		startX = m_centerX+m_x->GetValue(m_R/2, m_R);
		startY = m_centerY+(1.732)*(startX-(m_centerX+m_R));
		m_curEdge = 5;
	}

	m_helper.SetPosition(Vector(startX, startY, 0));

	NS_ASSERT (!m_event.IsRunning ());
	m_event = Simulator::ScheduleNow (&RandomEdgeHexagonalMobilityModel::BeginWalk, this);
	alreadyStarted = true;
	NotifyCourseChange ();
	MobilityModel::DoInitialize ();
}

void
RandomEdgeHexagonalMobilityModel::BeginWalk (void)
{
	m_helper.Update ();
	m_helper.Pause ();
	Vector current = m_helper.GetCurrentPosition ();
	Vector destination;

	uint8_t nextEdge = 0;
	do{
		double toss = m_edgeRV->GetValue();
		if(toss<1){
			nextEdge = 0;

		}else if((1 <= toss) && (toss<2)){
			nextEdge = 1;

		}else if((2 <= toss) && (toss<3)){
			nextEdge = 2;

		}else if((1 <= toss) && (toss<4)){
			nextEdge = 3;

		}else if((1 <= toss) && (toss<5)){
			nextEdge = 4;

		}else{// if((1 << m_edgeRV) && (m_edgeRV<6)){
			nextEdge = 5;
		}
	}while((nextEdge == m_curEdge) || (nextEdge == m_prevEdge));
	m_prevEdge = m_curEdge;
	m_curEdge = nextEdge;

	switch(m_curEdge){
	case 0:
		destination.x = m_centerX+m_x->GetValue(m_R/2, m_R);
		destination.y = m_centerY+(-1.732)*(destination.x-(m_centerX+m_R));
		break;
	case 1:
		destination.x = m_centerX+m_x->GetValue(m_R/2, m_R/2);
		destination.y = m_centerY+m_R*(1.732)/2;
		break;
	case 2:
		destination.x = m_centerX + m_x->GetValue(-m_R, -m_R/2);
		destination.y = m_centerY +(1.732)*(destination.x-(m_centerX-m_R));
		break;
	case 3:
		destination.x = m_centerX+m_x->GetValue(-m_R, -m_R/2);
		destination.y = m_centerY+(-1.732)*(destination.x-(m_centerX-m_R));
		break;
	case 4:
		destination.x = m_centerX+m_x->GetValue(-m_R/2, m_R/2);
		destination.y = m_centerY-m_R*(1.732/2);
		break;
	case 5:
		destination.x = m_centerX+m_x->GetValue(m_R/2, m_R);
		destination.y = m_centerY+(1.732)*(destination.x-(m_centerX+m_R));
		break;
	}

	double speed = m_speedRV->GetValue ();
	double dx = (destination.x - current.x);
	double dy = (destination.y - current.y);
	double dz = (destination.z - current.z);
	double k = speed / std::sqrt (dx*dx + dy*dy + dz*dz);

	m_helper.SetVelocity (Vector (k*dx, k*dy, k*dz));
	m_helper.Unpause ();

	Time travelDelay = Seconds (CalculateDistance (destination, current) / speed);
	m_event = Simulator::Schedule (travelDelay,
			&RandomEdgeHexagonalMobilityModel::PauseAtEdge, this);
	NotifyCourseChange ();
}

void
RandomEdgeHexagonalMobilityModel::PauseAtEdge()
{
	m_helper.Update ();
	m_helper.Pause ();
	Time pause = Seconds (m_pauseTime);
	m_event = Simulator::Schedule (pause, &RandomEdgeHexagonalMobilityModel::BeginWalk, this);
	NotifyCourseChange ();
}

Vector
RandomEdgeHexagonalMobilityModel::DoGetPosition (void) const
{
	m_helper.Update ();
	return m_helper.GetCurrentPosition ();
}

void
RandomEdgeHexagonalMobilityModel::DoSetPosition (const Vector &position)
{
	if (alreadyStarted)
	{
		m_helper.SetPosition (position);
		Simulator::Remove (m_event);
		m_event = Simulator::ScheduleNow (&RandomEdgeHexagonalMobilityModel::BeginWalk, this);
	}
}

Vector
RandomEdgeHexagonalMobilityModel::DoGetVelocity (void) const
{
	return m_helper.GetVelocity ();
}

int64_t
RandomEdgeHexagonalMobilityModel::DoAssignStreams (int64_t stream)
{
	m_speedRV->SetStream (stream);
	m_edgeRV->SetStream(stream+1);
	m_x->SetStream(stream+2);
	return (3);
}

}  // namespace ns3



