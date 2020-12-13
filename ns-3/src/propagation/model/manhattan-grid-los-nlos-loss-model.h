/*
 * manhattan-grid-los-nlos-loss-model.h
 *
 *  Created on: Sep 7, 2017
 *      Author: thang
 */

#ifndef MANHATTAN_GRID_LOS_NLOS_LOSS_MODEL_H_
#define MANHATTAN_GRID_LOS_NLOS_LOSS_MODEL_H_

#include "ns3/nstime.h"
#include "ns3/propagation-loss-model.h"
namespace ns3 {

/**
 The pathloss is proposed by ETRI
 *
 */

class ManhattanLOSNLOSLossModel : public PropagationLossModel
{

public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  ManhattanLOSNLOSLossModel ();

  /**
   * Get the propagation loss
   * \param a the mobility model of the source
   * \param b the mobility model of the destination
   * \returns the propagation loss (in dBm)
   */
  double GetLoss (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;

  double GetShadowing (void) const;
  /**
   * Set the shadowing value
   * \param shadowing the shadowing value
   */
  void SetShadowing (double shadowing);

  void SetDist1 (double dist1);
  double GetDist1 () const;

  void SetDist2 (double dist2);
  double GetDist2 () const;

  void SetMinimumDistance (double minimumDistance);
  double GetMinimumDistance (void) const;
private:
  /**
   * \brief Copy constructor
   *
   * Defined and unimplemented to avoid misuse
   */
  ManhattanLOSNLOSLossModel (const ManhattanLOSNLOSLossModel &);
  /**
   * \brief Copy constructor
   *
   * Defined and unimplemented to avoid misuse
   * \returns
   */
  ManhattanLOSNLOSLossModel & operator = (const ManhattanLOSNLOSLossModel &);

  virtual double DoCalcRxPower (double txPowerDbm, Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;
  virtual int64_t DoAssignStreams (int64_t stream);
  double m_shadowing; //!< Shadowing loss [dB]
  Ptr<UniformRandomVariable> rndGen;
  double m_minimumDistance;

  double m_dist1;
  double m_dist2;

};

}



#endif /* MANHATTAN_GRID_LOS_NLOS_LOSS_MODEL_H_ */
