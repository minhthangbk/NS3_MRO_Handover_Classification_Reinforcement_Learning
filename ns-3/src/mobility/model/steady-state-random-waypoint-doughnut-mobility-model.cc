/*
 * SteadyStateRandomWaypointDoughnutMobilityModel.cc
 *
 *  Created on: Dec 4, 2015
 *      Author: thang
 */



#include <cmath>
#include "ns3/simulator.h"
#include "ns3/double.h"
#include "steady-state-random-waypoint-doughnut-mobility-model.h"
#include "ns3/test.h"
#include <ns3/fatal-error.h>
#include <ns3/log.h>
#include <math.h>
namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SteadyStateRandomWaypointDoughnutMobilityModel);

TypeId
SteadyStateRandomWaypointDoughnutMobilityModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SteadyStateRandomWaypointDoughnutMobilityModel")
    .SetParent<MobilityModel> ()
    .SetGroupName ("Mobility")
    .AddConstructor<SteadyStateRandomWaypointDoughnutMobilityModel> ()
    .AddAttribute ("MinSpeed",
                   "Minimum speed value, [m/s]",
                   DoubleValue (0.3),
                   MakeDoubleAccessor (&SteadyStateRandomWaypointDoughnutMobilityModel::m_minSpeed),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MaxSpeed",
                   "Maximum speed value, [m/s]",
                   DoubleValue (0.7),
                   MakeDoubleAccessor (&SteadyStateRandomWaypointDoughnutMobilityModel::m_maxSpeed),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MinPause",
                   "Minimum pause value, [s]",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&SteadyStateRandomWaypointDoughnutMobilityModel::m_minPause),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MaxPause",
                   "Maximum pause value, [s]",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&SteadyStateRandomWaypointDoughnutMobilityModel::m_maxPause),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MinX",
                   "Minimum X value of traveling region, [m]",
                   DoubleValue (1),
                   MakeDoubleAccessor (&SteadyStateRandomWaypointDoughnutMobilityModel::m_minX),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MaxX",
                   "Maximum X value of traveling region, [m]",
                   DoubleValue (1),
                   MakeDoubleAccessor (&SteadyStateRandomWaypointDoughnutMobilityModel::m_maxX),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MinY",
                   "Minimum Y value of traveling region, [m]",
                   DoubleValue (1),
                   MakeDoubleAccessor (&SteadyStateRandomWaypointDoughnutMobilityModel::m_minY),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("MaxY",
                   "Maximum Y value of traveling region, [m]",
                   DoubleValue (1),
                   MakeDoubleAccessor (&SteadyStateRandomWaypointDoughnutMobilityModel::m_maxY),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Z",
                   "Z value of traveling region (fixed), [m]",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&SteadyStateRandomWaypointDoughnutMobilityModel::m_z),
                   MakeDoubleChecker<double> ())
   .AddAttribute ("MaxRadius",
				  "maximum radius of the doughnut, [m]",
				  DoubleValue (20),
				  MakeDoubleAccessor (&SteadyStateRandomWaypointDoughnutMobilityModel::m_maxR),
				  MakeDoubleChecker<double> ())
   .AddAttribute ("MinRadius",
				 "minimum radius of the doughnut, [m]",
				 DoubleValue (10),
				 MakeDoubleAccessor (&SteadyStateRandomWaypointDoughnutMobilityModel::m_minR),
				 MakeDoubleChecker<double> ())
   .AddAttribute ("DoughnutCenterX",
				"Center X",
				DoubleValue (0.0),
				MakeDoubleAccessor (&SteadyStateRandomWaypointDoughnutMobilityModel::m_centerX),
				MakeDoubleChecker<double> ())
  .AddAttribute ("DoughnutCenterY",
		  	   "Center Y",
			   DoubleValue (0.0),
			   MakeDoubleAccessor (&SteadyStateRandomWaypointDoughnutMobilityModel::m_centerY),
			   MakeDoubleChecker<double> ());

  return tid;
}

SteadyStateRandomWaypointDoughnutMobilityModel::SteadyStateRandomWaypointDoughnutMobilityModel () :
  alreadyStarted (false)
{
  m_speed = CreateObject<UniformRandomVariable> ();
  m_pause = CreateObject<UniformRandomVariable> ();
  m_x1_r = CreateObject<UniformRandomVariable> ();
  m_y1_r = CreateObject<UniformRandomVariable> ();
  m_x2_r = CreateObject<UniformRandomVariable> ();
  m_y2_r = CreateObject<UniformRandomVariable> ();
  m_u_r = CreateObject<UniformRandomVariable> ();
  m_x = CreateObject<UniformRandomVariable> ();
  m_y = CreateObject<UniformRandomVariable> ();
}

void
SteadyStateRandomWaypointDoughnutMobilityModel::DoInitialize (void)
{
  DoInitializePrivate ();
  MobilityModel::DoInitialize ();
}

void
SteadyStateRandomWaypointDoughnutMobilityModel::DoInitializePrivate (void)
{
  alreadyStarted = true;
  // Configure random variables based on attributes
  NS_ASSERT (m_minSpeed >= 1e-6);
  NS_ASSERT (m_minSpeed <= m_maxSpeed);
  NS_ASSERT (m_minR < m_maxR);

  m_speed->SetAttribute ("Min", DoubleValue (m_minSpeed));
  m_speed->SetAttribute ("Max", DoubleValue (m_maxSpeed));
  NS_ASSERT (m_minX < m_maxX);
  NS_ASSERT (m_minY < m_maxY);
//  m_position = CreateObject<RandomRectanglePositionAllocator> ();

  /*
   * Change by THANG
   */
  m_position = CreateObject<RandomDiscPositionAllocator> ();
  m_position->SetX (m_centerX);//Set the center of the disc
  m_position->SetY (m_centerY);
  m_position->SetMaxRho(m_maxR);	//Set the min radius for random variable
  m_position->SetMinRho(m_minR);	//Set the max radius for random variable

  m_x->SetAttribute ("Min", DoubleValue (m_minX));
  m_x->SetAttribute ("Max", DoubleValue (m_maxX));
  m_y->SetAttribute ("Min", DoubleValue (m_minY));
  m_y->SetAttribute ("Max", DoubleValue (m_maxY));

  NS_ASSERT (m_minPause <= m_maxPause);
  m_pause->SetAttribute ("Min", DoubleValue (m_minPause));
  m_pause->SetAttribute ("Max", DoubleValue (m_maxPause));

  m_helper.Update ();
  m_helper.Pause ();

  // calculate the steady-state probability that a node is initially paused
  double expectedPauseTime = (m_minPause + m_maxPause)/2; // = 0
  double a = m_maxX - m_minX;
  double b = m_maxY - m_minY;
  double v0 = m_minSpeed;
  double v1 = m_maxSpeed;
  double log1 = b*b / a*std::log (std::sqrt ((a*a)/(b*b) + 1) + a/b);
  double log2 = a*a / b*std::log (std::sqrt ((b*b)/(a*a) + 1) + b/a);
  double expectedTravelTime = 1.0/6.0*(log1 + log2);
  expectedTravelTime += 1.0/15.0*((a*a*a)/(b*b) + (b*b*b)/(a*a)) -
    1.0/15.0*std::sqrt (a*a + b*b)*((a*a)/(b*b) + (b*b)/(a*a) - 3);
  if (v0 == v1)
    {
      expectedTravelTime /= v0;
    }
  else
    {
      expectedTravelTime *= std::log (v1/v0)/(v1 - v0);
    }
  double probabilityPaused = expectedPauseTime/(expectedPauseTime + expectedTravelTime);//default = 0
  NS_ASSERT (probabilityPaused >= 0 && probabilityPaused <= 1);

  double u = m_u_r->GetValue (0, 1);
  probabilityPaused = 1; //THANG no need to wait
  if (u < probabilityPaused) // node initially paused
    {
      m_helper.SetPosition (m_position->GetNext ());
      u = m_u_r->GetValue (0, 1);
      Time pause;
      if (m_minPause != m_maxPause)
        {
          if (u < (2*m_minPause/(m_minPause + m_maxPause)))
            {
              pause = Seconds (u*(m_minPause + m_maxPause)/2);
            }
          else
            {
              // there is an error in equation 20 in the Tech. Report MCS-03-04
              // this error is corrected in the TMC 2004 paper and below
              pause = Seconds (m_maxPause - std::sqrt ((1 - u)*(m_maxPause*m_maxPause - m_minPause*m_minPause)));
            }
        }
      else // if pause is constant
        {
          pause = Seconds (u*expectedPauseTime);
        }
      NS_ASSERT (!m_event.IsRunning ());
      m_event = Simulator::Schedule (pause, &SteadyStateRandomWaypointDoughnutMobilityModel::BeginWalk, this);
    }
  else // node initially moving //always start first
    {
      double x1, x2, y1, y2;
      double r = 0;
      double u1 = 1;
      while (u1 >= r)
        {
          x1 = m_x1_r->GetValue (0, a);
          y1 = m_y1_r->GetValue (0, b);
          x2 = m_x2_r->GetValue (0, a);
          y2 = m_y2_r->GetValue (0, b);
          u1 = m_u_r->GetValue (0, 1);
          r = std::sqrt (((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1))/(a*a + b*b));
          NS_ASSERT (r <= 1);
        }
      double u2 = m_u_r->GetValue (0, 1);
      m_helper.SetPosition (Vector (m_minX + u2*x1 + (1 - u2)*x2, m_minY + u2*y1 + (1 - u2)*y2, m_z));

      NS_ASSERT (!m_event.IsRunning ());
      m_event = Simulator::ScheduleNow (&SteadyStateRandomWaypointDoughnutMobilityModel::SteadyStateBeginWalk, this,
                                        Vector (m_minX + x2, m_minY + y2, m_z));
    }
  NotifyCourseChange ();
}

void
SteadyStateRandomWaypointDoughnutMobilityModel::SteadyStateBeginWalk (const Vector &destination)
{
  m_helper.Update ();
  Vector m_current = m_helper.GetCurrentPosition ();
  NS_ASSERT (m_minX <= m_current.x && m_current.x <= m_maxX);
  NS_ASSERT (m_minY <= m_current.y && m_current.y <= m_maxY);
  NS_ASSERT (m_minX <= destination.x && destination.x <= m_maxX);
  NS_ASSERT (m_minY <= destination.y && destination.y <= m_maxY);
  double u = m_u_r->GetValue (0, 1);
  double speed = std::pow (m_maxSpeed, u)/std::pow (m_minSpeed, u - 1);
  double dx = (destination.x - m_current.x);
  double dy = (destination.y - m_current.y);
  double dz = (destination.z - m_current.z);
  double k = speed / std::sqrt (dx*dx + dy*dy + dz*dz);

  m_helper.SetVelocity (Vector (k*dx, k*dy, k*dz));
  m_helper.Unpause ();
  Time travelDelay = Seconds (CalculateDistance (destination, m_current) / speed);
  m_event = Simulator::Schedule (travelDelay,
                                 &SteadyStateRandomWaypointDoughnutMobilityModel::Start, this);
  NotifyCourseChange ();
}

void
SteadyStateRandomWaypointDoughnutMobilityModel::BeginWalk (void)
{
  m_helper.Update ();
  Vector m_current = m_helper.GetCurrentPosition ();
  NS_ASSERT (m_minX <= m_current.x && m_current.x <= m_maxX);
  NS_ASSERT (m_minY <= m_current.y && m_current.y <= m_maxY);
  NS_ASSERT (m_minR < m_maxR && m_minR > 0 && m_maxR > 0);
//  NS_ASSERT ((m_minX - m_maxX + m_maxY - m_minY) == 0);//Square not rectangular
  NS_ASSERT (round(m_minX - m_maxX + m_maxY - m_minY) < 0.1);//due to the precise of double value
  /*
   * THANG modified SteadyStateRandomWaypointMobilityModel
   */

  Vector destination;
  double d2center;
  double alpha1;
  double alpha2;//For not hitting the small circle
  double alpha3;//angle between current vector position and horizontal unit vector
  double vector1X = m_current.x - m_centerX;//Vector from current point to center point
  double vector1Y = m_current.y - m_centerY;
  double vector1L = std::sqrt(std::pow(vector1X,2)+std::pow(vector1Y,2));

  NS_ASSERT(m_minR <= vector1L);
  alpha1 = std::asin(m_minR/vector1L);
  alpha3 = std::acos(vector1X/vector1L);
  alpha3 = (vector1Y > 0) ? alpha3 : -alpha3;

  double clockwiseTangentUnitVectorX = std::cos(alpha3-1.5708);//pi/2 ~= 1.5708
  double clockwiseTangentUnitVectorY = std::sin(alpha3-1.5708);//pi/2
  double clockwiseTangentUnitVectorLen = std::sqrt(std::pow(clockwiseTangentUnitVectorX,2)+std::pow(clockwiseTangentUnitVectorY,2));//ensure for the scalar product, ~= 1
  double alpha4;

  double vector2X;
  double vector2Y;
  double vector2Len;
  do{
	  destination = m_position->GetNext ();
	  d2center = std::sqrt(std::pow(destination.x-m_centerX,2)+std::pow(destination.y-m_centerY,2));

	  vector2X = destination.x - m_current.x;
	  vector2Y = destination.y - m_current.y;
	  vector2Len = std::sqrt(std::pow(vector2X,2)+std::pow(vector2Y,2));

	  alpha2 = std::acos((vector2X * (-vector1X) + vector2Y * (-vector1Y))/(vector1L * vector2Len));

	  alpha4 = std::acos((vector2X*clockwiseTangentUnitVectorX + vector2Y*clockwiseTangentUnitVectorY)/(vector2Len*clockwiseTangentUnitVectorLen));

  }while( (d2center < m_minR) || (d2center > m_maxR) || ((alpha2 <= alpha1) && (d2center >= m_minR)) || (alpha4 > 1.5));

  double speed = m_speed->GetValue ();
  double dx = (destination.x - m_current.x);
  double dy = (destination.y - m_current.y);
  double dz = (destination.z - m_current.z);
  double k = speed / std::sqrt (dx*dx + dy*dy + dz*dz);

  m_helper.SetVelocity (Vector (k*dx, k*dy, k*dz));
  m_helper.Unpause ();
  Time travelDelay = Seconds (CalculateDistance (destination, m_current) / speed);
  m_event = Simulator::Schedule (travelDelay,
                                 &SteadyStateRandomWaypointDoughnutMobilityModel::Start, this);
  NotifyCourseChange ();
}

void
SteadyStateRandomWaypointDoughnutMobilityModel::Start (void)
{
  m_helper.Update ();
  m_helper.Pause ();
  Time pause = Seconds (m_pause->GetValue ());
  m_event = Simulator::Schedule (pause, &SteadyStateRandomWaypointDoughnutMobilityModel::BeginWalk, this);
  NotifyCourseChange ();
}

Vector
SteadyStateRandomWaypointDoughnutMobilityModel::DoGetPosition (void) const
{
  m_helper.Update ();
  return m_helper.GetCurrentPosition ();
}
void
SteadyStateRandomWaypointDoughnutMobilityModel::DoSetPosition (const Vector &position)
{
  if (alreadyStarted)
    {
      m_helper.SetPosition (position);
      Simulator::Remove (m_event);
      m_event = Simulator::ScheduleNow (&SteadyStateRandomWaypointDoughnutMobilityModel::Start, this);
    }
}
Vector
SteadyStateRandomWaypointDoughnutMobilityModel::DoGetVelocity (void) const
{
  return m_helper.GetVelocity ();
}
int64_t
SteadyStateRandomWaypointDoughnutMobilityModel::DoAssignStreams (int64_t stream)
{
  int64_t positionStreamsAllocated = 0;
  m_speed->SetStream (stream);
  m_pause->SetStream (stream + 1);
  m_x1_r->SetStream (stream + 2);
  m_y1_r->SetStream (stream + 3);
  m_x2_r->SetStream (stream + 4);
  m_y2_r->SetStream (stream + 5);
  m_u_r->SetStream (stream + 6);
  m_x->SetStream (stream + 7);
  m_y->SetStream (stream + 8);
  positionStreamsAllocated = m_position->AssignStreams (stream + 9);
  return (9 + positionStreamsAllocated);
}

} // namespace ns3
