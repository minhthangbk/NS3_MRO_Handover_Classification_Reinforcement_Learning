/*
 * epc-cson-sap.h
 *
 *  Created on: Nov 12, 2015
 *      Author: thang
 */

#ifndef EPC_CSON_SAP_H_
#define EPC_CSON_SAP_H_

#include <map>
#include <set>
#include <stdint.h>

#include "epc-enb-s1-sap.h"
#include "son-common.h"

namespace ns3 {
/*
 * MME forward received notification being sent from DSON to CSON entity/application (OAM layer)
 */
class EpcMmeCSONSapUser{
public:
	virtual ~EpcMmeCSONSapUser();
	virtual void MmeSendPMReportNotifyCSON(uint16_t cellId, SONCommon::SONMsgInfo msgInfo, SONCommon::PerformanceReport pmReport) = 0;
	virtual void MmeSendDSONParamRespCSON(uint16_t cellId, SONCommon::SONMsgInfo msgInfo, SONCommon::SONParamList paramList) = 0;
};

/*
 * Implement Member CSON SAP User
 */
template <class C>
class MemberEpcMmeCSONSapUser : public EpcMmeCSONSapUser{
public:
	MemberEpcMmeCSONSapUser(C* owner);
	virtual void MmeSendPMReportNotifyCSON(uint16_t cellId, SONCommon::SONMsgInfo msgInfo, SONCommon::PerformanceReport pmReport);
	virtual void MmeSendDSONParamRespCSON(uint16_t cellId, SONCommon::SONMsgInfo msgInfo, SONCommon::SONParamList paramList);
	C* m_owner;
};

template <class C>
MemberEpcMmeCSONSapUser<C>::MemberEpcMmeCSONSapUser(C* owner){
	m_owner = owner;
}

template <class C>
void
MemberEpcMmeCSONSapUser<C>::MmeSendPMReportNotifyCSON(uint16_t cellId, SONCommon::SONMsgInfo msgInfo, SONCommon::PerformanceReport pmReport){
	m_owner->CSONRecvPMReportNotify(cellId, msgInfo, pmReport);
}
template <class C>
void
MemberEpcMmeCSONSapUser<C>::MmeSendDSONParamRespCSON(uint16_t cellId, SONCommon::SONMsgInfo msgInfo, SONCommon::SONParamList paramList){
	m_owner->CSONRecvDSONParamResp(cellId,msgInfo,paramList);
}

/*
 * CSON forwards Configuration to MME, then MME forwards to DSON via S1-AP interface
 */
class EpcMmeCSONSapProvider{
public:
	virtual ~EpcMmeCSONSapProvider();
	/*
	 * Dev by THANG
	 * CSON send updated parameters to MME
	 */
	virtual void CSONSendUpdateParamConfigMme(uint16_t cellId, SONCommon::SONMsgInfo msgInfo, SONCommon::SONParamList paramList) = 0;
	/*
	 * CSON Send this msg to update current settings from DSON
	 * (at the beginning of the simulation)
	 */
	virtual void CSONSendDSONParamReqMme(SONCommon::SONMsgInfo msgInfo) = 0;
};


/*
 * Implement Member CSON SAP Provider
 */
template <class C>
class MemberEpcMmeCSONSapProvider : public EpcMmeCSONSapProvider{
public:
	MemberEpcMmeCSONSapProvider(C* owner);
	virtual void CSONSendUpdateParamConfigMme(uint16_t cellId, SONCommon::SONMsgInfo msgInfo, SONCommon::SONParamList paramList);
	virtual void CSONSendDSONParamReqMme(SONCommon::SONMsgInfo msgInfo);
	C* m_owner;
};

template <class C>
MemberEpcMmeCSONSapProvider<C>::MemberEpcMmeCSONSapProvider(C* owner){
	m_owner = owner;
};

template <class C>
void
MemberEpcMmeCSONSapProvider<C>::CSONSendUpdateParamConfigMme(uint16_t cellId, SONCommon::SONMsgInfo msgInfo, SONCommon::SONParamList paramList){
	m_owner->MmeRecvUpdateParamConfig(cellId, msgInfo, paramList);
}

template <class C>
void
MemberEpcMmeCSONSapProvider<C>::CSONSendDSONParamReqMme(SONCommon::SONMsgInfo msgInfo){
	m_owner->MmeRecvDSONParamReq(msgInfo);
}



}  // namespace ns3


#endif /* EPC_CSON_SAP_H_ */
