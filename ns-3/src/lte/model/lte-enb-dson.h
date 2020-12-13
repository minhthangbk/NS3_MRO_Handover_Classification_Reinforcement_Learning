/*
 * lte-dson.h
 *
 *  Created on: Nov 3, 2015
 *      Author: thang
 *      Distributed SON implementation
 *      + MRO
 */

#ifndef LTE_ENB_DSON_H_
#define LTE_ENB_DSON_H_

#include <ns3/nstime.h>
#include <ns3/object.h>
#include <ns3/lte-enb-dson-sap.h>
#include <ns3/event-id.h>

#include <fstream> //THANG for printing out message
#include <map>
#include <string>
#include <sstream> //For converting integer to string
#include <set>
#include "ptr.h"
#include <ns3/random-variable-stream.h>
#include "son-common.h"
namespace ns3{

//class HoOperatingPoint1 : public Object{
//public:
//	HoOperatingPoint1(uint8_t maxA3OffId, uint8_t maxTTTId, uint8_t a3OffId, uint8_t tttId);
//	virtual ~HoOperatingPoint1();
//
//	uint8_t m_maxA3OffId;
//	uint8_t m_maxTTTId;
//
//	uint8_t m_A3OffId;
//	uint8_t m_TTTId;
//
//	void MoveRight();
//	void MoveLeft();
//	void MoveUp();
//	void MoveDown();
//
//	bool IsMovingRightFeasible();
//	bool IsMovingLeftFeasible();
//	bool IsMovingUpFeasible();
//	bool IsMovingDownFeasible();
//
//};
/*
 * Diagonal mapping TTT-A3Offset
 * According to 1687-1499-2011-98
 */
//class HoOperatingPoint3 : public Object{
//public:
//	HoOperatingPoint3(uint8_t maxId,uint8_t a3OffId, uint8_t tttId);
//	virtual ~HoOperatingPoint3();
//
//	uint8_t m_maxId;
//
//	uint8_t m_A3OffId;
//	uint8_t m_TTTId;
//
//	void MoveUp();
//	void MoveDown();
//
//	bool IsMovingUpFeasible();
//	bool IsMovingDownFeasible();
//
//};
/*
 * Zigzag mapping
 * According to 1687-1499-2011-98
 */
//class HoOperatingPoint4 : public Object{
//public:
//	HoOperatingPoint4(uint8_t maxA3OffId, uint8_t maxTTTId, uint8_t a3OffId, uint8_t tttId);
//	virtual ~HoOperatingPoint4();
//
//	uint8_t m_maxA3OffId;
//	uint8_t m_maxTTTId;
//
//	struct HoParamMap{
//		uint8_t a3OffId;
//		uint8_t tttId;
//	};
//	std::map<uint8_t, HoParamMap> m_paramMap;
//	uint8_t m_curPos;
//
//	void MoveForward();
//	void MoveBackward();
//
//	bool IsMovingForwardFeasible();
//	bool IsMovingBackwardFeasible();
//
//	HoParamMap GetNewPoint();
//
//};

class LteEnbDSON : public Object{
//friend class LteEnbDSONSapUser;
//friend class LteEnbDSONSapProvider;
public:
	LteEnbDSON(); //Construction
	virtual ~LteEnbDSON (); //Destruction method

protected:
    virtual void DoDispose (void);
public:
  	static TypeId GetTypeId (void);
//  	Ptr<UniformRandomVariable> m_cioRandomVariable;

//	LteEnbDSONSapUser* m_dSONSapUser;
	LteEnbDSONSapUser* m_dSONSapUser; //avoid memory leak
	LteEnbDSONSapProvider* m_dSONSapProvider;
//	Ptr<LteEnbDSONSapProvider> m_dSONSapProvider;
	/*
	 * Handover Parameters
	 */

	std::map<uint8_t,double> m_a3OffVals;
	std::map<uint8_t,uint16_t> m_tttVals;
	std::map<uint8_t, int8_t> m_cioList;

	std::map<uint16_t,int8_t> m_curCellsToAddModMap;
	double m_curA3Offset;
	double m_curHyst;
	uint16_t m_curTTT;
//	/*
//	 * Algorithm1 Implementation
//	 */
//	void MroAlgorithm1(LteEnbDSONSapUser::HoInfoBundle hoInfoBundle);//Adaptive {TTT, A3Ooffset}
//	bool m_enableMroAlgorithm1;
//	Time m_MroAlgorithm1Period;
//	Ptr<HoOperatingPoint1> m_hoOPManager1;
	/*
	 * Algorithm2 Implementation
	 */
//	void MroAlgorithm2(LteEnbDSONSapUser::HoInfoBundle hoInfoBundle);//Adaptive CIO
//	bool m_enableMroAlgorithm2;
//	Time m_MroAlgorithm2Period;
	/*
	 * MroAlgorithm3
	 * Previous work
	 */
//	void MroAlgorithm3(LteEnbDSONSapUser::HoInfoBundle hoInfoBundle);//Adaptive CIO
//	bool m_enableMroAlgorithm3;
//	Time m_MroAlgorithm3Period;
//	Ptr<HoOperatingPoint3> m_hoOPManager3;
	/*
		 * MroAlgorithm4
		 * Previous work
		 */
//	void MroAlgorithm4(LteEnbDSONSapUser::HoInfoBundle hoInfoBundle);//Adaptive CIO
//	bool m_enableMroAlgorithm4;
//	Ptr<HoOperatingPoint4> m_hoOPManager4;
	/*
	 * DSON SAP User interface
	 */
//	void DoUpdateHoInfo(LteEnbDSONSapUser::HoInfoBundle hoInfoBundle);
	void DoSetUpInitialHandoverParams(double hyst, double a3Off, uint16_t ttt, std::map<uint16_t,int8_t> cioList);

	/*
	* Set Sap provider, used by LteHelper
	*/
	void SetEnbDSONSapProvider(LteEnbDSONSapProvider* provider);

	/*
	* Get Sap User, used by LteHelper
	*/
	LteEnbDSONSapUser* GetEnbDSONSapUser();

	/*
	 * Start SON Algorithms
	 */
	void StartDSONMroAlgorithm();

	Time m_CSONPeriod;
	void StartCSONMroAlgorithm();
	/*
	 * SON API RECEIVE
	 */
	void DSONRecvUpdateParamConfig(SONCommon::SONMsgInfo msgInfo, SONCommon::SONParamList paramList);
	void DSONRecvDSONParamReq(SONCommon::SONMsgInfo msgInfo);
};//End Class LteEnbDSON


}//End name space


#endif //End dson class
/* LTE_DSON_H_ */
