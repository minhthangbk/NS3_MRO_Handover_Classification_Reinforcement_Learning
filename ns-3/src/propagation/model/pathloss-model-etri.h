/*
 * Dev by THANG
 */

#ifndef PATHLOSS_MODEL_ETRI_H
#define PATHLOSS_MODEL_ETRI_H

#include "ns3/nstime.h"
#include "ns3/propagation-loss-model.h"
namespace ns3 {

/**
 The pathloss is proposed by ETRI
 *
 */

class PathlossModelETRI : public PropagationLossModel
{

public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  PathlossModelETRI ();

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

  void SetMinimumDistance (double minimumDistance);
  double GetMinimumDistance (void) const;
private:
  /**
   * \brief Copy constructor
   *
   * Defined and unimplemented to avoid misuse
   */
  PathlossModelETRI (const PathlossModelETRI &);
  /**
   * \brief Copy constructor
   *
   * Defined and unimplemented to avoid misuse
   * \returns
   */
  PathlossModelETRI & operator = (const PathlossModelETRI &);

  virtual double DoCalcRxPower (double txPowerDbm, Ptr<MobilityModel> a, Ptr<MobilityModel> b) const;
  virtual int64_t DoAssignStreams (int64_t stream);
  double m_shadowing; //!< Shadowing loss [dB]
  double m_minimumDistance;

};

}

#endif
