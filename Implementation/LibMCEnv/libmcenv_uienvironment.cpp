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


Abstract: This is a stub class definition of CUIEnvironment

*/

#include "libmcenv_uienvironment.hpp"
#include "libmcenv_interfaceexception.hpp"
#include "libmcenv_xmldocument.hpp"
#include "libmcenv_discretefielddata2d.hpp"
#include "libmcenv_usermanagementhandler.hpp"
#include "libmcenv_journalhandler.hpp"
#include "libmcenv_dataseries.hpp"
#include "libmcenv_meshobject.hpp"
#include "libmcenv_alert.hpp"

#include "amc_systemstate.hpp"
#include "amc_accesscontrol.hpp"
#include "amc_meshhandler.hpp"
#include "amc_dataserieshandler.hpp"
#include "libmcenv_signaltrigger.hpp"
#include "libmcenv_imagedata.hpp"
#include "libmcenv_testenvironment.hpp"
#include "libmcenv_build.hpp"
#include "libmcenv_journalvariable.hpp"

#include "amc_toolpathhandler.hpp"
#include "amc_logger.hpp"
#include "amc_statemachinedata.hpp"
#include "amc_ui_handler.hpp"
#include "libmcdata_dynamic.hpp"

// Include custom headers here.
#include "common_utils.hpp"

#include <cmath>

using namespace LibMCEnv::Impl;

/*************************************************************************************************************************
 Class definition of CUIEnvironment 
**************************************************************************************************************************/

uint32_t colorRGBtoInteger(const LibMCEnv::sColorRGB Color)
{
    int32_t nColorRed = (int32_t)round(Color.m_Red * 255.0);
    if (nColorRed < 0)
        nColorRed = 0;
    if (nColorRed > 255)
        nColorRed = 255;

    int32_t nColorGreen = (int32_t)round(Color.m_Green * 255.0);
    if (nColorGreen < 0)
        nColorGreen = 0;
    if (nColorGreen > 255)
        nColorGreen = 255;

    int32_t nColorBlue = (int32_t)round(Color.m_Blue * 255.0);
    if (nColorBlue < 0)
        nColorBlue = 0;
    if (nColorBlue > 255)
        nColorBlue = 255;

    return (uint32_t)nColorRed + (uint32_t)nColorGreen * 256 + (uint32_t)nColorBlue * 65536;
}


CUIEnvironment::CUIEnvironment(AMC::CUIHandler* pUIHandler, const std::string& sSenderUUID, const std::string& sSenderName, AMC::PParameterHandler pClientVariableHandler, const std::string& sTestEnvironmentPath, AMC::PUserInformation pUserInformation)
    : 
      m_pUIHandler (pUIHandler),
      m_sLogSubSystem ("ui"),
      m_sSenderName (sSenderName),
      m_pClientVariableHandler (pClientVariableHandler),
      m_pUserInformation (pUserInformation)
{

    if (pUserInformation.get () == nullptr)
        throw ELibMCEnvInterfaceException(LIBMCENV_ERROR_INVALIDPARAM);
    if (pUIHandler == nullptr)
        throw ELibMCEnvInterfaceException(LIBMCENV_ERROR_INVALIDPARAM);
    if (pClientVariableHandler.get() == nullptr)
        throw ELibMCEnvInterfaceException(LIBMCENV_ERROR_INVALIDPARAM);

    m_pUISystemState = pUIHandler->getUISystemState();
    m_pLogger = m_pUISystemState->getLogger();

    if (!sSenderUUID.empty()) {
        m_sSenderUUID = (AMCCommon::CUtils::normalizeUUIDString(sSenderUUID));
    }
    else {
        m_sSenderUUID = AMCCommon::CUtils::createEmptyUUID();
    }

}

CUIEnvironment::~CUIEnvironment()
{

}

void CUIEnvironment::ActivateModalDialog(const std::string& sDialogName)
{
    m_ClientActions.push_back(std::make_shared<AMC::CUIClientAction_ActivateModalDialog> (sDialogName));
}

void CUIEnvironment::CloseModalDialog()
{
    m_ClientActions.push_back(std::make_shared<AMC::CUIClientAction_CloseModalDialog>());
}

void CUIEnvironment::ActivatePage(const std::string& sPageName)
{
    m_ClientActions.push_back(std::make_shared<AMC::CUIClientAction_ActivatePage>(sPageName));
}

std::string CUIEnvironment::RetrieveEventSender()
{
    return m_sSenderName;
}

std::string CUIEnvironment::RetrieveEventSenderPage()
{
    std::vector<std::string> sPathNames;
    AMCCommon::CUtils::splitString(m_sSenderName, ".", sPathNames);

    if (sPathNames.empty())
        return "";

    return sPathNames.at(0);
}


std::string CUIEnvironment::RetrieveEventSenderUUID()
{
    return m_sSenderUUID;
}


ISignalTrigger * CUIEnvironment::PrepareSignal(const std::string & sMachineInstance, const std::string & sSignalName)
{
    if (!m_pUISystemState->getSignalHandler()->hasSignalDefinition(sMachineInstance, sSignalName))
        throw ELibMCEnvInterfaceException(LIBMCENV_ERROR_COULDNOTFINDSIGNALDEFINITON);

    return new CSignalTrigger(m_pUISystemState->getSignalHandler(), sMachineInstance, sSignalName);
}

std::string CUIEnvironment::GetMachineState(const std::string & sMachineInstance)
{
    return m_pUISystemState->getStateMachineData()->getInstanceStateName (sMachineInstance);
}

void CUIEnvironment::LogMessage(const std::string & sLogString)
{	
    m_pLogger->logMessage(sLogString, m_sLogSubSystem, AMC::eLogLevel::Message);
}

void CUIEnvironment::LogWarning(const std::string & sLogString)
{	
    m_pLogger->logMessage(sLogString, m_sLogSubSystem, AMC::eLogLevel::Warning);
}

void CUIEnvironment::LogInfo(const std::string & sLogString)
{	
    m_pLogger->logMessage(sLogString, m_sLogSubSystem, AMC::eLogLevel::Info);
}



std::string CUIEnvironment::GetMachineParameter(const std::string& sMachineInstance, const std::string& sParameterGroup, const std::string& sParameterName)
{
    auto pParameterHandler = m_pUISystemState->getStateMachineData ()->getParameterHandler(sMachineInstance);
    auto pGroup = pParameterHandler->findGroup(sParameterGroup, true);
    return pGroup->getParameterValueByName(sParameterName);
}

std::string CUIEnvironment::GetMachineParameterAsUUID(const std::string& sMachineInstance, const std::string& sParameterGroup, const std::string& sParameterName)
{
    auto pParameterHandler = m_pUISystemState->getStateMachineData()->getParameterHandler(sMachineInstance);
    auto pGroup = pParameterHandler->findGroup(sParameterGroup, true);
    return pGroup->getParameterValueByName(sParameterName);
}

LibMCEnv_double CUIEnvironment::GetMachineParameterAsDouble(const std::string& sMachineInstance, const std::string& sParameterGroup, const std::string& sParameterName)
{
    auto pParameterHandler = m_pUISystemState->getStateMachineData()->getParameterHandler(sMachineInstance);
    auto pGroup = pParameterHandler->findGroup(sParameterGroup, true);
    return pGroup->getDoubleParameterValueByName(sParameterName);
}

LibMCEnv_int64 CUIEnvironment::GetMachineParameterAsInteger(const std::string& sMachineInstance, const std::string& sParameterGroup, const std::string& sParameterName)
{
    auto pParameterHandler = m_pUISystemState->getStateMachineData()->getParameterHandler(sMachineInstance);
    auto pGroup = pParameterHandler->findGroup(sParameterGroup, true);
    return pGroup->getIntParameterValueByName(sParameterName);
}

bool CUIEnvironment::GetMachineParameterAsBool(const std::string& sMachineInstance, const std::string& sParameterGroup, const std::string& sParameterName)
{
    auto pParameterHandler = m_pUISystemState->getStateMachineData()->getParameterHandler(sMachineInstance);
    auto pGroup = pParameterHandler->findGroup(sParameterGroup, true);
    return pGroup->getBoolParameterValueByName(sParameterName);
}


std::string CUIEnvironment::GetUIProperty(const std::string& sElementPath, const std::string& sPropertyName) 
{   
    if (m_pClientVariableHandler.get() == nullptr)
        throw ELibMCEnvInterfaceException (LIBMCENV_ERROR_COULDNNOTACCESSCLIENTVARIABLES);

    auto pGroup = m_pClientVariableHandler->findGroup(sElementPath, true);
    return pGroup->getParameterValueByName(sPropertyName);
}

std::string CUIEnvironment::GetUIPropertyAsUUID(const std::string& sElementPath, const std::string& sPropertyName)
{
    if (m_pClientVariableHandler.get() == nullptr)
        throw ELibMCEnvInterfaceException(LIBMCENV_ERROR_COULDNNOTACCESSCLIENTVARIABLES);

    auto pGroup = m_pClientVariableHandler->findGroup(sElementPath, true);
    return pGroup->getParameterValueByName(sPropertyName);

}

LibMCEnv_double CUIEnvironment::GetUIPropertyAsDouble(const std::string& sElementPath, const std::string& sPropertyName)
{
    if (m_pClientVariableHandler.get() == nullptr)
        throw ELibMCEnvInterfaceException(LIBMCENV_ERROR_COULDNNOTACCESSCLIENTVARIABLES);

    auto pGroup = m_pClientVariableHandler->findGroup(sElementPath, true);
    return pGroup->getDoubleParameterValueByName(sPropertyName);
}

LibMCEnv_int64 CUIEnvironment::GetUIPropertyAsInteger(const std::string& sElementPath, const std::string& sPropertyName)
{
    if (m_pClientVariableHandler.get() == nullptr)
        throw ELibMCEnvInterfaceException(LIBMCENV_ERROR_COULDNNOTACCESSCLIENTVARIABLES);

    auto pGroup = m_pClientVariableHandler->findGroup(sElementPath, true);
    return pGroup->getIntParameterValueByName(sPropertyName);
}

bool CUIEnvironment::GetUIPropertyAsBool(const std::string& sElementPath, const std::string& sPropertyName)
{
    if (m_pClientVariableHandler.get() == nullptr)
        throw ELibMCEnvInterfaceException(LIBMCENV_ERROR_COULDNNOTACCESSCLIENTVARIABLES);

    auto pGroup = m_pClientVariableHandler->findGroup(sElementPath, true);
    return pGroup->getBoolParameterValueByName(sPropertyName);
}

void CUIEnvironment::SetUIProperty(const std::string& sElementPath, const std::string& sPropertyName, const std::string& sValue)
{
    if (m_pClientVariableHandler.get() == nullptr)
        throw ELibMCEnvInterfaceException(LIBMCENV_ERROR_COULDNNOTACCESSCLIENTVARIABLES);

    auto pGroup = m_pClientVariableHandler->findGroup(sElementPath, true);
    pGroup->setParameterValueByName(sPropertyName, sValue);
}

void CUIEnvironment::SetUIPropertyAsUUID(const std::string& sElementPath, const std::string& sPropertyName, const std::string& sValue)
{
    if (m_pClientVariableHandler.get() == nullptr)
        throw ELibMCEnvInterfaceException(LIBMCENV_ERROR_COULDNNOTACCESSCLIENTVARIABLES);


    auto pGroup = m_pClientVariableHandler->findGroup(sElementPath, true);
    if (!sValue.empty())
        pGroup->setParameterValueByName(sPropertyName, AMCCommon::CUtils::normalizeUUIDString (sValue));
    else
        pGroup->setParameterValueByName(sPropertyName, AMCCommon::CUtils::createEmptyUUID());
}

void CUIEnvironment::SetUIPropertyAsDouble(const std::string& sElementPath, const std::string& sPropertyName, const LibMCEnv_double dValue) 
{
    if (m_pClientVariableHandler.get() == nullptr)
        throw ELibMCEnvInterfaceException(LIBMCENV_ERROR_COULDNNOTACCESSCLIENTVARIABLES);

    auto pGroup = m_pClientVariableHandler->findGroup(sElementPath, true);
    pGroup->setDoubleParameterValueByName(sPropertyName, dValue);

}

void CUIEnvironment::SetUIPropertyAsInteger(const std::string& sElementPath, const std::string& sPropertyName, const LibMCEnv_int64 nValue) 
{
    if (m_pClientVariableHandler.get() == nullptr)
        throw ELibMCEnvInterfaceException(LIBMCENV_ERROR_COULDNNOTACCESSCLIENTVARIABLES);

    auto pGroup = m_pClientVariableHandler->findGroup(sElementPath, true);
    pGroup->setIntParameterValueByName(sPropertyName, nValue);
}

void CUIEnvironment::SetUIPropertyAsBool(const std::string& sElementPath, const std::string& sPropertyName, const bool bValue)
{
    if (m_pClientVariableHandler.get() == nullptr)
        throw ELibMCEnvInterfaceException(LIBMCENV_ERROR_COULDNNOTACCESSCLIENTVARIABLES);

    auto pGroup = m_pClientVariableHandler->findGroup(sElementPath, true);
    pGroup->setBoolParameterValueByName(sPropertyName, bValue);
}



IImageData* CUIEnvironment::CreateEmptyImage(const LibMCEnv_uint32 nPixelSizeX, const LibMCEnv_uint32 nPixelSizeY, const LibMCEnv_double dDPIValueX, const LibMCEnv_double dDPIValueY, const LibMCEnv::eImagePixelFormat ePixelFormat)
{
    return CImageData::createEmpty(nPixelSizeX, nPixelSizeY, dDPIValueX, dDPIValueY, ePixelFormat);
}

IImageData* CUIEnvironment::LoadPNGImage(const LibMCEnv_uint64 nPNGDataBufferSize, const LibMCEnv_uint8* pPNGDataBuffer, const LibMCEnv_double dDPIValueX, const LibMCEnv_double dDPIValueY, const LibMCEnv::eImagePixelFormat ePixelFormat)
{
    return CImageData::createFromPNG(pPNGDataBuffer, nPNGDataBufferSize, dDPIValueX, dDPIValueY, ePixelFormat);
}

LibMCEnv_uint64 CUIEnvironment::GetGlobalTimerInMilliseconds()
{
    return m_pUISystemState->getGlobalChronoInstance()->getExistenceTimeInMilliseconds();
}

LibMCEnv_uint64 CUIEnvironment::GetGlobalTimerInMicroseconds()
{
    return m_pUISystemState->getGlobalChronoInstance()->getExistenceTimeInMicroseconds();
}

void CUIEnvironment::LogOut()
{
    m_ClientActions.push_back(std::make_shared<AMC::CUIClientAction_Logout>());
}

void CUIEnvironment::ShowHint(const std::string& sHint, const LibMCEnv_uint32 nTimeoutInMS)
{
    m_ClientActions.push_back(std::make_shared<AMC::CUIClientAction_ShowHint>(sHint, nTimeoutInMS, -1, -1));
}


void CUIEnvironment::ShowHintColored(const std::string& sHint, const LibMCEnv_uint32 nTimeoutInMS, const LibMCEnv::sColorRGB Color, const LibMCEnv::sColorRGB FontColor)
{

    int32_t nColorValue = (int32_t)colorRGBtoInteger (Color);
    int32_t nFontColorValue = (int32_t)colorRGBtoInteger(FontColor);

    m_ClientActions.push_back(std::make_shared<AMC::CUIClientAction_ShowHint>(sHint, nTimeoutInMS, nColorValue, nFontColorValue));
}

void CUIEnvironment::HideHint()
{
    m_ClientActions.push_back(std::make_shared<AMC::CUIClientAction_HideHint>());
}

std::string CUIEnvironment::ShowMessageDlg(const std::string& sCaption, const std::string& sTitle, const LibMCEnv::eMessageDialogType eDialogType, const std::string& sYesEvent, const std::string& sNoEvent, const std::string& sCancelEvent)
{
    std::string sDialogType;
    std::string sDialogUUID = AMCCommon::CUtils::createUUID();

    switch (eDialogType) {
    case LibMCEnv::eMessageDialogType::DialogOK:
        sDialogType = "ok"; break;
    case LibMCEnv::eMessageDialogType::DialogOKCancel:
        sDialogType = "okcancel"; break;
    case LibMCEnv::eMessageDialogType::DialogYesNo:
        sDialogType = "yesno"; break;
    case LibMCEnv::eMessageDialogType::DialogYesNoCancel:
        sDialogType = "yesnocancel"; break;        
    }

    if (!sYesEvent.empty())
        m_pUIHandler->ensureUIEventExists(sYesEvent);
    if (!sNoEvent.empty())
        m_pUIHandler->ensureUIEventExists(sNoEvent);
    if (!sCancelEvent.empty())
        m_pUIHandler->ensureUIEventExists(sCancelEvent);

    m_ClientActions.push_back(std::make_shared<AMC::CUIClientAction_ShowMessageDlg> (sDialogUUID, sDialogType, sCaption, sTitle, sYesEvent, sNoEvent, sCancelEvent));

    return sDialogUUID;
}


std::vector<AMC::PUIClientAction>& CUIEnvironment::getClientActions()
{
    return m_ClientActions;
}


ITestEnvironment* CUIEnvironment::GetTestEnvironment()
{
    return new CTestEnvironment(m_pUISystemState->getTestOutputPath ());
}

LibMCEnv::Impl::IXMLDocument* CUIEnvironment::CreateXMLDocument(const std::string& sRootNodeName, const std::string& sDefaultNamespace)
{
    auto pDocument = std::make_shared<AMC::CXMLDocumentInstance> ();

    pDocument->createEmptyDocument(sRootNodeName, sDefaultNamespace);

    return new CXMLDocument(pDocument);
}

LibMCEnv::Impl::IXMLDocument* CUIEnvironment::ParseXMLString(const std::string& sXMLString)
{
    auto pDocument = std::make_shared<AMC::CXMLDocumentInstance>();

    try {
        pDocument->parseXMLString(sXMLString);
    } 
    catch (std::exception& E) {
        throw ELibMCEnvInterfaceException(LIBMCENV_ERROR_COULDNOTPARSEXMLSTRING, "could not parse XML string: " + std::string (E.what()));
    }

    return new CXMLDocument(pDocument);

}

LibMCEnv::Impl::IXMLDocument* CUIEnvironment::ParseXMLData(const LibMCEnv_uint64 nXMLDataBufferSize, const LibMCEnv_uint8* pXMLDataBuffer)
{
    auto pDocument = std::make_shared<AMC::CXMLDocumentInstance>();

    try {
        pDocument->parseXMLData(nXMLDataBufferSize, pXMLDataBuffer);
    }
    catch (std::exception& E) {
        throw ELibMCEnvInterfaceException(LIBMCENV_ERROR_COULDNOTPARSEXMLDATA, "could not parse XML data: " + std::string(E.what()));
    }

    return new CXMLDocument(pDocument);

}



bool CUIEnvironment::HasBuildJob(const std::string& sBuildUUID)
{
    std::string sNormalizedBuildUUID = AMCCommon::CUtils::normalizeUUIDString(sBuildUUID);

    try {
        m_pUISystemState->getBuildJobHandler()->RetrieveJob(sNormalizedBuildUUID);
        return true;
    }
    catch (std::exception) {
        return false;
    }
}

IBuild* CUIEnvironment::GetBuildJob(const std::string& sBuildUUID)
{
    std::string sNormalizedBuildUUID = AMCCommon::CUtils::normalizeUUIDString(sBuildUUID);

    auto pBuildJob = m_pUISystemState->getBuildJobHandler()->RetrieveJob(sNormalizedBuildUUID);
    return new CBuild(pBuildJob, m_pUISystemState->getToolpathHandler(), m_pUISystemState->getStorage(), m_pUISystemState->getSystemUserID());
}


IDiscreteFieldData2D* CUIEnvironment::CreateDiscreteField2D(const LibMCEnv_uint32 nPixelSizeX, const LibMCEnv_uint32 nPixelSizeY, const LibMCEnv_double dDPIValueX, const LibMCEnv_double dDPIValueY, const LibMCEnv_double dOriginX, const LibMCEnv_double dOriginY, const LibMCEnv_double dDefaultValue)
{
    AMC::PDiscreteFieldData2DInstance pInstance = std::make_shared<AMC::CDiscreteFieldData2DInstance>(nPixelSizeX, nPixelSizeY, dDPIValueX, dDPIValueY, dOriginX, dOriginY, dDefaultValue, true);
    return new CDiscreteFieldData2D(pInstance);
}

IDiscreteFieldData2D* CUIEnvironment::CreateDiscreteField2DFromImage(IImageData* pImageDataInstance, const LibMCEnv_double dBlackValue, const LibMCEnv_double dWhiteValue, const LibMCEnv_double dOriginX, const LibMCEnv_double dOriginY)
{
    if (pImageDataInstance == nullptr)
        throw ELibMCEnvInterfaceException(LIBMCENV_ERROR_INVALIDPARAM);

    auto pImageDataImpl = dynamic_cast<CImageData*> (pImageDataInstance);
    if (pImageDataImpl == nullptr)
        throw ELibMCEnvInterfaceException(LIBMCENV_ERROR_INVALIDCAST);

    uint32_t nPixelSizeX, nPixelSizeY;
    pImageDataImpl->GetSizeInPixels(nPixelSizeX, nPixelSizeY);

    double dDPIValueX, dDPIValueY;
    pImageDataImpl->GetDPI(dDPIValueX, dDPIValueY);

    AMC::PDiscreteFieldData2DInstance pFieldInstance = std::make_shared<AMC::CDiscreteFieldData2DInstance>(nPixelSizeX, nPixelSizeY, dDPIValueX, dDPIValueY, dOriginX, dOriginY, 0.0, false);

    auto pixelFormat = pImageDataImpl->GetPixelFormat();
    auto& rawPixelData = pImageDataImpl->getPixelData();
    pFieldInstance->loadFromRawPixelData (rawPixelData, pixelFormat, dBlackValue, dWhiteValue);
    
    return new CDiscreteFieldData2D(pFieldInstance);

}


bool CUIEnvironment::CheckPermission(const std::string& sPermissionIdentifier)
{
    return m_pUISystemState->getAccessControl()->checkPermissionInRole(m_pUserInformation->getRoleIdentifier(), sPermissionIdentifier);
}

std::string CUIEnvironment::GetCurrentUserLogin()
{
    return m_pUserInformation->getLogin();
}

std::string CUIEnvironment::GetCurrentUserDescription()
{
    return m_pUserInformation->getDescription();
}

std::string CUIEnvironment::GetCurrentUserRole()
{
    return m_pUserInformation->getRoleIdentifier();

}

std::string CUIEnvironment::GetCurrentUserLanguage()
{
    return m_pUserInformation->getLanguageIdentifier();

}

std::string CUIEnvironment::GetCurrentUserUUID()
{
    return m_pUserInformation->getUUID();

}

IUserManagementHandler* CUIEnvironment::CreateUserManagement()
{
    return new CUserManagementHandler(m_pUISystemState->getLoginHandler(), m_pUISystemState->getAccessControl(), m_pUISystemState->getLanguageHandler());
}

IJournalHandler* CUIEnvironment::GetCurrentJournal()
{
    return new CJournalHandler(m_pUISystemState->getStateJournal());
}

IMeshObject* CUIEnvironment::RegisterMeshFrom3MFResource(const std::string& sResourceName, const std::string& sMeshUUID)
{

    auto pResourcePackage = m_pUIHandler->getCoreResourcePackage();
    if (pResourcePackage.get() == nullptr)
        throw ELibMCEnvInterfaceException(LIBMCENV_ERROR_INTERNALERROR);

    auto pResourceEntry = pResourcePackage->findEntryByName(sResourceName, true);

    auto pMeshHandler = m_pUISystemState->getMeshHandler();
    auto pLib3MFWrapper = m_pUISystemState->getToolpathHandler()->getLib3MFWrapper();

    auto pMeshEntity = pMeshHandler->register3MFResource(pLib3MFWrapper.get(), pResourcePackage.get(), sResourceName);

    return new CMeshObject(pMeshHandler, pMeshEntity->getUUID());
}

bool CUIEnvironment::MeshIsRegistered(const std::string& sMeshUUID)
{
    auto pMeshHandler = m_pUISystemState->getMeshHandler();
    return pMeshHandler->hasMeshEntity(sMeshUUID);
}

IMeshObject* CUIEnvironment::FindRegisteredMesh(const std::string& sMeshUUID)
{
    auto pMeshHandler = m_pUISystemState->getMeshHandler();
    auto pMeshEntity = pMeshHandler->findMeshEntity(sMeshUUID, false);

    if (pMeshEntity.get() == nullptr)
        throw ELibMCEnvInterfaceException(LIBMCENV_ERROR_MESHISNOTREGISTERED, "mesh is not registered: " + sMeshUUID);

    return new CMeshObject(pMeshHandler, pMeshEntity->getUUID());
}

IDataSeries* CUIEnvironment::CreateDataSeries(const std::string& sName, const bool bBoundToLogin)
{    
    auto pDataSeriesHandler = m_pUISystemState->getDataSeriesHandler();
    auto pDataSeries = pDataSeriesHandler->createDataSeries(sName);

    return new CDataSeries(pDataSeries);

}

bool CUIEnvironment::HasDataSeries(const std::string& sDataSeriesUUID)
{
    auto pDataSeriesHandler = m_pUISystemState->getDataSeriesHandler();
    return pDataSeriesHandler->hasDataSeries(sDataSeriesUUID);
}

IDataSeries* CUIEnvironment::FindDataSeries(const std::string& sDataSeriesUUID)
{
    auto pDataSeries = m_pUISystemState->getDataSeriesHandler()->findDataSeries(sDataSeriesUUID, true);

    return new CDataSeries(pDataSeries);


}

void CUIEnvironment::ReleaseDataSeries(const std::string& sDataSeriesUUID)
{
    auto pDataSeriesHandler = m_pUISystemState->getDataSeriesHandler();
    pDataSeriesHandler->unloadDataSeries(sDataSeriesUUID);

}

IAlert* CUIEnvironment::CreateAlert(const std::string& sIdentifier, const std::string& sReadableContextInformation)
{
    throw ELibMCEnvInterfaceException(LIBMCENV_ERROR_NOTIMPLEMENTED);
    //return new CAlert ();
}

IAlert* CUIEnvironment::FindAlert(const std::string& sUUID)
{
    return nullptr;
}

void CUIEnvironment::AcknowledgeAlert(const std::string& sAlertUUID, const std::string& sUserComment)
{

}

void CUIEnvironment::AcknowledgeAlertForUser(const std::string& sAlertUUID, const std::string& sUserUUID, const std::string& sUserComment)
{

}

