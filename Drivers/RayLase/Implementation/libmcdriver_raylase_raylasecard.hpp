/*++

Copyright (C) 2020 Autodesk Inc.

All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the Autodesk Inc. nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 'AS IS' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL AUTODESK INC. BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


Abstract: This is the class declaration of CRaylaseCard

*/


#ifndef __LIBMCDRIVER_RAYLASE_RAYLASECARD
#define __LIBMCDRIVER_RAYLASE_RAYLASECARD

#include "libmcdriver_raylase_interfaces.hpp"
#include "libmcdriver_raylase_sdk.hpp"

// Parent classes
#include "libmcdriver_raylase_base.hpp"
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4250)
#endif

// Include custom headers here.


namespace LibMCDriver_Raylase {
namespace Impl {

class CRaylaseCardImpl;
typedef std::shared_ptr<CRaylaseCardImpl> PRaylaseCardImpl;

class CRaylaseCardImpl {
private:

	LibMCEnv::PDriverEnvironment m_pDriverEnvironment;

	PRaylaseSDK m_pSDK;
	std::string m_sCardName;
	std::string m_sCardIP;
	uint32_t m_nPort;

	double m_dMaxLaserPowerInWatts;

	rlHandle m_Handle;
	bool m_bSimulationMode;

	bool m_bSimulatedPilotIsEnabled;
	bool m_bSimulatedPilotIsArmed;
	bool m_bSimulatedPilotIsAlarm;

public:
	
	static PRaylaseCardImpl connectByIP(PRaylaseSDK pSDK, const std::string& sCardName, const std::string& sCardIP, uint32_t nPort, double dMaxLaserPowerInWatts, bool bSimulationMode, LibMCEnv::PDriverEnvironment pDriverEnvironment);

	CRaylaseCardImpl(PRaylaseSDK pSDK, const std::string& sCardName, const std::string& sCardIP, uint32_t nPort, double dMaxLaserPowerInWatts, bool bSimulationMode, LibMCEnv::PDriverEnvironment pDriverEnvironment);
	virtual ~CRaylaseCardImpl();

	void ResetToSystemDefaults();

	void LaserOn();

	void LaserOff();

	void ArmLaser(const bool bShallBeArmed);

	bool IsLaserArmed();

	void EnablePilot(const bool bShallBeEnabled);

	bool PilotIsEnabled();

	void GetLaserStatus(bool& bPilotIsEnabled, bool& bLaserIsArmed, bool& bLaserAlarm);

	void DrawLayer(const std::string& sStreamUUID, const LibMCDriver_Raylase_uint32 nLayerIndex);

	bool IsConnected();

	void Disconnect();


};



/*************************************************************************************************************************
 Class declaration of CRaylaseCard 
**************************************************************************************************************************/

class CRaylaseCard : public virtual IRaylaseCard, public virtual CBase {
private:
	PRaylaseCardImpl m_pRaylaseCardImpl;

public:

	CRaylaseCard(PRaylaseCardImpl pRaylaseCardImpl);

	virtual ~CRaylaseCard();

	bool IsConnected() override;

	void Disconnect() override;

	void ResetToSystemDefaults() override;

	void LaserOn() override;

	void LaserOff() override;

	void ArmLaser(const bool bShallBeArmed) override;

	bool IsLaserArmed() override;

	void EnablePilot(const bool bShallBeEnabled) override;

	bool PilotIsEnabled() override;

	void GetLaserStatus(bool & bPilotIsEnabled, bool & bLaserIsArmed, bool & bLaserAlarm) override;

	void DrawLayer(const std::string & sStreamUUID, const LibMCDriver_Raylase_uint32 nLayerIndex) override;

};

} // namespace Impl
} // namespace LibMCDriver_Raylase

#ifdef _MSC_VER
#pragma warning(pop)
#endif
#endif // __LIBMCDRIVER_RAYLASE_RAYLASECARD
