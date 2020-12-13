/*
 * epc-cson.h
 *
 *  Created on: Nov 12, 2015
 *      Author: thang
 */

#ifndef EPC_CSON_H_
#define EPC_CSON_H_

#include <ns3/object.h>
#include <ns3/epc-s1ap-sap.h>
#include <ns3/epc-s11-sap.h>
#include "epc-mme-cson-sap.h"
#include <ns3/traced-callback.h>
#include <map>
#include <list>
#include "epc-enb-s1-sap.h"
#include "son-common.h" //common SON data structures
#include <ns3/simulator.h>
namespace ns3 {

class EpcMmeCSON : public Object{
public:
	EpcMmeCSON();
	virtual ~EpcMmeCSON();
	// inherited from Object
	static TypeId GetTypeId (void);
protected:
	virtual void DoDispose ();
	/*
	 * Receive report from DSONs
	 */
	void DoReportHandoverPerformance(uint16_t cellid, double kpi);
	bool m_enableMroAlgorithm; //enable/disable mro algorithm
public:
	EpcMmeCSONSapProvider* m_CSONSapProvider;
	EpcMmeCSONSapUser* m_CSONSapUser;
	EpcMmeCSONSapUser* GetEpcMmeCSONSapUser();
	void SetEpcMmeCSONSapProvider(EpcMmeCSONSapProvider* sapProvider);

	/*
	 * Callback (return the result to main function)
	 * input: cellId, hysteresis,
	 */
//	TracedCallback<uint16_t, SONCommon::SONParamList> m_MroAlgorithmOutputs;
	/*
	 * Adaptive CIO, TTT, a3
	 */
//	void CSONMroAlgorithm1(uint16_t cellId,SONCommon::PerformanceReport pmReport);
	/*
	 * SON API RECEIVE
	 */
	void CSONRecvPMReportNotify(uint16_t cellId, SONCommon::SONMsgInfo msgInfo, SONCommon::PerformanceReport pmReport);
	void ETRIAlgorithm(uint16_t cellId, SONCommon::SONMsgInfo msgInfo, SONCommon::PerformanceReport pmReport);
	void StdDevHysteresisAlgorithm(uint16_t cellId, SONCommon::SONMsgInfo msgInfo, SONCommon::PerformanceReport pmReport);
	void CSONRecvDSONParamResp(uint16_t cellId, SONCommon::SONMsgInfo msgInfo, SONCommon::SONParamList paramList);
	/*
	 * SON API SEND
	 */
	void CSONSendUpdateParamConfigMme(uint16_t cellId, SONCommon::SONMsgInfo msgInfo, SONCommon::SONParamList paramList);
	/*
	 *	Parameters store for each eNB
	 */
	std::map<uint16_t, SONCommon::SONParamList> m_eNBsParamStore;
	/*
	 * Store CIO values according to 3GPP
	 */
	std::map<uint8_t, int8_t> m_cioArray;
	/*
	 * Store TTT according to 3GPP
	 */
	std::map<uint8_t,uint16_t> m_tttVals;
	/*
	 * Store a3Offset
	 */
	std::map<uint8_t,double> m_a3OffVals;

	void SetUpParamInitialValues();
	void SaveHOPerformance(uint16_t cellId,uint16_t numLHO, uint16_t numEHO, uint16_t numWHO, uint16_t numPPHO, uint16_t totalHO);
	bool m_firstWrite;
};



}  // namespace ns3

#endif /* EPC_CSON_H_ */
