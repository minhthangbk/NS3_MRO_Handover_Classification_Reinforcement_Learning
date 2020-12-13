/*
 * lte-dson-sap.h
 *
 *  Created on: Nov 3, 2015
 *      Author: thang
 */

#ifndef LTE_ENB_DSON_SAP_H_
#define LTE_ENB_DSON_SAP_H_

#include <map>
#include <set>
#include <stdint.h>
#include "son-common.h"
namespace ns3{

class LteEnbDSONSapUser{
public:

	struct HandoverIssueInfo{
		bool m_lhoDetected;
		uint16_t m_numLHO;
		bool m_ehoDetected;
		uint16_t m_numEHO;
		bool m_whoDetected;
		uint16_t m_numWHO;
		/*
		 * Add pingpong
		 */
		bool m_ppDetected;
		uint16_t m_numPingpong;
	};

	virtual ~LteEnbDSONSapUser();
	virtual void SetUpInitialHandoverParams(double hyst, double a3Off, uint16_t ttt, std::map<uint16_t,int8_t> cioList)=0;
	/*
	 * SON API SEND TO DSON
	 */
	virtual void EnbRrcSendUpdateParamConfigDSON(SONCommon::SONMsgInfo msgInfo, SONCommon::SONParamList paramList) = 0;
	virtual void EnbRrcSendDSONParamReqDSON(SONCommon::SONMsgInfo msgInfo) = 0;
};


class LteEnbDSONSapProvider{
public:

	virtual ~LteEnbDSONSapProvider();
	/*
	 * SON API SEND to Enb RRC
	 */
	virtual void DSONPMReportNotifyEnbRrc(SONCommon::SONMsgInfo msgInfo) = 0;
	virtual void DSONSendDSONParamRespEnbRrc(SONCommon::SONMsgInfo msgInfo, SONCommon::SONParamList paramList) = 0;
	struct MroParameters{
		std::map<uint16_t,int8_t> cioList;
		double a3Offset;
		double hyst;
		uint16_t ttt;
	};
	virtual void DSONSetParamEnbRrc(MroParameters mroParam)=0;
};


/*
 *  * \brief Template for the implementation of the LteEnbDSONSapUser
 *        as a member of an owner class of type C to which all methods are
 *        forwarded.
 */
template <class C>
class MemberLteEnbDSONSapProvider : public LteEnbDSONSapProvider{
public:
	MemberLteEnbDSONSapProvider(C* owner);
	virtual void DSONPMReportNotifyEnbRrc(SONCommon::SONMsgInfo msgInfo);
	virtual void DSONSendDSONParamRespEnbRrc(SONCommon::SONMsgInfo msgInfo, SONCommon::SONParamList paramList);
	virtual void DSONSetParamEnbRrc(MroParameters mroParam);
	C* m_owner;
};

template <class C>
MemberLteEnbDSONSapProvider<C>::MemberLteEnbDSONSapProvider (C* owner)
  : m_owner (owner)
{
}

/*
 * SON API send to ENB RRC
 */
template <class C>
void
MemberLteEnbDSONSapProvider<C>::DSONPMReportNotifyEnbRrc(SONCommon::SONMsgInfo msgInfo){
	m_owner->EnbRrcRcvPMReportNotify(msgInfo);
}

template <class C>
void
MemberLteEnbDSONSapProvider<C>::DSONSendDSONParamRespEnbRrc(SONCommon::SONMsgInfo msgInfo, SONCommon::SONParamList paramList){
	m_owner->EnbRrcRecvDSONParamResp(msgInfo,paramList);
}

template <class C>
void
MemberLteEnbDSONSapProvider<C>::DSONSetParamEnbRrc(MroParameters mroParam){
	m_owner->EnbRrcSetMroParams(mroParam);
}
/*
 * Implement Skeleton of Member of LteEnbDSONSapUser
 *
 *
 */
template <class C>
class MemberLteEnbDSONSapUser : public LteEnbDSONSapUser{
public:
	MemberLteEnbDSONSapUser(C* owner);
	virtual void SetUpInitialHandoverParams(double hyst, double a3Off, uint16_t ttt, std::map<uint16_t,int8_t> cioList);
	/*
	 * SON API SEND to DSON
	 */
	virtual void EnbRrcSendUpdateParamConfigDSON(SONCommon::SONMsgInfo msgInfo, SONCommon::SONParamList paramList);
	virtual void EnbRrcSendDSONParamReqDSON(SONCommon::SONMsgInfo msgInfo);
	C* m_owner;
};

template <class C>
MemberLteEnbDSONSapUser<C>::MemberLteEnbDSONSapUser (C* owner)
  : m_owner (owner)
{
}


template <class C>
void
MemberLteEnbDSONSapUser<C>::SetUpInitialHandoverParams(double hyst, double a3Off, uint16_t ttt, std::map<uint16_t,int8_t> cioList){
	m_owner->DoSetUpInitialHandoverParams(hyst,a3Off,ttt,cioList);
}

template <class C>
void
MemberLteEnbDSONSapUser<C>::EnbRrcSendUpdateParamConfigDSON(SONCommon::SONMsgInfo msgInfo, SONCommon::SONParamList paramList){
	m_owner->DSONRecvUpdateParamConfig(msgInfo,paramList);
}

template <class C>
void
MemberLteEnbDSONSapUser<C>::EnbRrcSendDSONParamReqDSON(SONCommon::SONMsgInfo msgInfo){
	m_owner->DSONRecvDSONParamReq(msgInfo);
}

}//End name space NS3

#endif /* LTE_DSON_SAP_H_ */
