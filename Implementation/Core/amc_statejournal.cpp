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

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL AUTODESK INC. BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/


#include "amc_statejournal.hpp"

#include "common_utils.hpp"
#include "common_chrono.hpp"
#include "libmc_exceptiontypes.hpp"
#include "amc_logger.hpp"

#include <map>
#include <thread>
#include <future>
#include <iostream>
#include <mutex>

namespace AMC {

	#define STATEJOURNAL_MAXVARIABLECOUNT  (16 * 1024 * 1024)

	#define STATEJOURNAL_VARIABLE_MINUNITS 1.0E-6
	#define STATEJOURNAL_VARIABLE_MAXUNITS 1.0E6

	class CStateJournalImplVariable {
	protected:
		CStateJournalStream* m_pStream;
		uint32_t m_nID;
		uint32_t m_nStorageIndex;
		std::string m_sName;
	public:
		CStateJournalImplVariable(CStateJournalStream* pStream, const uint32_t nID, const uint32_t nStorageIndex, const std::string& sName)
			: m_pStream(pStream), m_nID (nID), m_sName (sName), m_nStorageIndex (nStorageIndex)
		{
			LibMCAssertNotNull(pStream);
		}

		uint32_t getID()
		{
			return m_nID;
		}

		uint32_t getStorageIndex()
		{
			return m_nStorageIndex;
		}

		std::string getName()
		{
			return m_sName;
		}

		virtual eStateJournalVariableType getType() = 0;

	};


	class CStateJournalImplBoolVariable : public CStateJournalImplVariable {
	private:
		bool m_bCurrentValue;
	public:
		CStateJournalImplBoolVariable(CStateJournalStream* pStream, const uint32_t nID, const uint32_t nStorageIndex, const std::string& sName)
			: CStateJournalImplVariable (pStream, nID, nStorageIndex, sName), m_bCurrentValue (false)
		{

		}

		virtual eStateJournalVariableType getType() 
		{
			return eStateJournalVariableType::vtBoolParameter;
		}

		void setValue_MicroSecond(const bool bValue, const uint64_t nAbsoluteTimeStampInMicroSeconds)
		{
			if (bValue != m_bCurrentValue) {

				m_pStream->writeBool_MicroSecond (nAbsoluteTimeStampInMicroSeconds, m_nStorageIndex, bValue);
				m_bCurrentValue = bValue;
			}
		}

		void setInitialValue(const bool bValue)
		{
			m_bCurrentValue = bValue;
		}

	};


	class CStateJournalImplIntegerVariable : public CStateJournalImplVariable {
	private:
		int64_t m_nCurrentValue;

	public:
		CStateJournalImplIntegerVariable(CStateJournalStream* pStream, const uint32_t nID, const uint32_t nStorageIndex, const std::string& sName)
			: CStateJournalImplVariable(pStream, nID, nStorageIndex, sName), m_nCurrentValue(0)
		{
		}

		virtual eStateJournalVariableType getType()
		{
			return eStateJournalVariableType::vtIntegerParameter;
		}

		void setValue_MicroSecond(const int64_t nValue, const uint64_t nAbsoluteTimeStampInMicroSeconds)
		{
			if (m_nCurrentValue != nValue) {

				m_pStream->writeInt64_MicroSecond(nAbsoluteTimeStampInMicroSeconds, m_nStorageIndex, nValue);
				m_nCurrentValue = nValue;
			}
		}

		void setInitialValue(const int64_t nValue)
		{
			m_nCurrentValue = nValue;
		}


		void readTimeStream(const sStateJournalInterval& interval, std::vector<sJournalTimeStreamInt64Entry>& timeStream)
		{


			m_pStream->readRawIntegerData (m_nStorageIndex, interval, timeStream);


			/*for (auto& timeStreamEntry : m_TimeStream) {
				if (timeStreamEntry.first <= nStartTimeStamp) {
					dStartValue = timeStreamEntry.second;
				}
				else {
					if (timeStreamEntry.first <= nEndTimeStamp) {
						sJournalTimeStreamInt64Entry entry;
						entry.m_nTimeStamp = timeStreamEntry.first;
						entry.m_nValue = timeStreamEntry.second;
						timeStream.push_back(entry);
					}
				}
			} */

		}


	};

	class CStateJournalImplDoubleVariable : public CStateJournalImplVariable {
	private:
		int64_t m_nCurrentValueInUnits;
		double m_dUnits;
		bool m_bHasUnits;

	public:
		CStateJournalImplDoubleVariable(CStateJournalStream* pStream, const uint32_t nID, const uint32_t nStorageIndex, const std::string& sName)
			: CStateJournalImplVariable(pStream, nID, nStorageIndex, sName), m_nCurrentValueInUnits(0), m_dUnits (0.0001), m_bHasUnits (false)
		{
		}

		virtual eStateJournalVariableType getType()
		{
			return eStateJournalVariableType::vtDoubleParameter;
		}

		void setUnits(const double dUnits)
		{
			if ((dUnits < STATEJOURNAL_VARIABLE_MINUNITS) || (dUnits > STATEJOURNAL_VARIABLE_MAXUNITS))
				throw ELibMCCustomException(LIBMC_ERROR_INVALIDVARIABLEUNITS, m_sName);
			if (m_bHasUnits)
				throw ELibMCCustomException(LIBMC_ERROR_UNITSHAVEALREADYBEENSET, m_sName);

			m_dUnits = dUnits;
			m_bHasUnits = true;

		}

		void setValue_MicroSecond(const double dValue, const uint64_t nAbsoluteTimeStampInMicroSeconds)
		{
			if (!m_bHasUnits)
				throw ELibMCCustomException(LIBMC_ERROR_UNITSHAVENOTBEENSET, m_sName);

			int64_t nValueInUnits = (int64_t) (dValue / m_dUnits);
			if (m_nCurrentValueInUnits != nValueInUnits) {

				m_pStream->writeDouble_MicroSecond (nAbsoluteTimeStampInMicroSeconds, m_nStorageIndex, nValueInUnits);

				m_nCurrentValueInUnits = nValueInUnits;

			}

		}

		void setInitialValue(const double dValue)
		{
			if (!m_bHasUnits)
				throw ELibMCCustomException(LIBMC_ERROR_UNITSHAVENOTBEENSET, m_sName);

			int64_t nValueInUnits = (int64_t)(dValue / m_dUnits);
			m_nCurrentValueInUnits = nValueInUnits;

		}

		void readTimeStream(const sStateJournalInterval & interval, std::vector<sJournalTimeStreamDoubleEntry>& timeStream)
		{
			if (!m_bHasUnits)
				throw ELibMCCustomException(LIBMC_ERROR_UNITSHAVENOTBEENSET, m_sName);

			m_pStream->readRawDoubleData (m_nStorageIndex, interval, m_dUnits, timeStream);

		}


	};

	class CStateJournalImplStringVariable : public CStateJournalImplVariable {
	private:
		std::string m_sCurrentValue;
	public:
		CStateJournalImplStringVariable(CStateJournalStream* pStream, const uint32_t nID, const uint32_t nStorageIndex, const std::string& sName)
			: CStateJournalImplVariable(pStream, nID, nStorageIndex, sName), m_sCurrentValue("")
		{
		}

		virtual eStateJournalVariableType getType()
		{
			return eStateJournalVariableType::vtStringParameter;
		}

		void setValue_MicroSecond(const std::string & sValue, const uint64_t nAbsoluteTimeStamp_MicroSecond)
		{
			if (m_sCurrentValue != sValue) {

				// TODO
				m_sCurrentValue = sValue;
			}
		}

		void setInitialValue(const std::string& sValue)
		{
			m_sCurrentValue = sValue;
		}


	};

	typedef std::shared_ptr<CStateJournalImplVariable> PStateJournalImplVariable;
	typedef std::shared_ptr<CStateJournalImplBoolVariable> PStateJournalImplBoolVariable;
	typedef std::shared_ptr<CStateJournalImplIntegerVariable> PStateJournalImplIntegerVariable;
	typedef std::shared_ptr<CStateJournalImplDoubleVariable> PStateJournalImplDoubleVariable;
	typedef std::shared_ptr<CStateJournalImplStringVariable> PStateJournalImplStringVariable;

	class CStateJournalImpl {
	private:
		std::map<std::string, PStateJournalImplVariable> m_VariableStringMap;
		std::map<uint32_t, PStateJournalImplVariable> m_VariableIDMap;
		std::vector<PStateJournalImplVariable> m_VariableList;

		uint32_t m_nChunkIntervalInMilliseconds;
		uint32_t m_nChunkWriteIntervalInSeconds;

		eStateJournalMode m_JournalMode;
		AMCCommon::CChrono m_Chrono;

		PStateJournalStream m_pStream;
		PLogger m_pLogger;

		std::mutex m_Mutex;

		std::atomic<bool> m_ThreadStopFlag;
		std::future<void> m_ThreadFuture;

	public:

		CStateJournalImpl(PStateJournalStream pStream);

		PStateJournalImplVariable generateVariable(const eStateJournalVariableType eVariableType, const std::string& sName);

		void startRecording();
		void finishRecording();

		void updateBoolValue(const uint32_t nVariableID, const bool bValue);
		void updateIntegerValue(const uint32_t nVariableID, const int64_t nValue);
		void updateStringValue(const uint32_t nVariableID, const std::string& sValue);
		void updateDoubleValue(const uint32_t nVariableID, const double dValue);

		uint64_t retrieveTimeStamp_MicroSecond();

		void readDoubleTimeStream(const std::string& sName, const sStateJournalInterval& interval, std::vector<sJournalTimeStreamDoubleEntry>& timeStream);

		void recordingThread();
	};


	CStateJournalImpl::CStateJournalImpl(PStateJournalStream pStream)
		: m_JournalMode(eStateJournalMode::sjmInitialising),
	     m_Chrono (false),
		m_pStream (pStream),
		m_nChunkIntervalInMilliseconds(0),
		m_nChunkWriteIntervalInSeconds (1),
		m_ThreadStopFlag (false)

	{
		LibMCAssertNotNull(pStream.get());
		m_nChunkIntervalInMilliseconds = 60000;
		m_nChunkWriteIntervalInSeconds = 10;
	}


	PStateJournalImplVariable CStateJournalImpl::generateVariable(const eStateJournalVariableType eVariableType, const std::string& sName)
	{

		if (m_JournalMode != eStateJournalMode::sjmInitialising)
			throw ELibMCCustomException(LIBMC_ERROR_JOURNALISNOTINITIALISING, sName);

		if (m_VariableList.size () >= STATEJOURNAL_MAXVARIABLECOUNT)
			throw ELibMCCustomException(LIBMC_ERROR_TOOMANYJOURNALVARIABLES, sName);

		PStateJournalImplVariable pVariable;
		uint32_t nVariableIndex = (uint32_t)m_VariableList.size();
		uint32_t nVariableID = (uint32_t)(m_VariableList.size() + 1);

		switch (eVariableType) {
		case eStateJournalVariableType::vtBoolParameter:			
			pVariable = std::make_shared<CStateJournalImplBoolVariable>(m_pStream.get(), nVariableID, nVariableIndex, sName);
			break;

		case eStateJournalVariableType::vtIntegerParameter:
			pVariable = std::make_shared<CStateJournalImplIntegerVariable>(m_pStream.get(), nVariableID, nVariableIndex, sName);
			break;

		case eStateJournalVariableType::vtDoubleParameter:
			pVariable = std::make_shared<CStateJournalImplDoubleVariable>(m_pStream.get(), nVariableID, nVariableIndex, sName);
			break;

		case eStateJournalVariableType::vtStringParameter:
			pVariable = std::make_shared<CStateJournalImplStringVariable>(m_pStream.get(), nVariableID, nVariableIndex, sName);
			break;

		default:
			throw ELibMCCustomException(LIBMC_ERROR_INVALIDVARIABLETYPE, sName);
		}
		
		m_VariableIDMap.insert (std::make_pair (pVariable->getID (), pVariable));
		m_VariableStringMap.insert(std::make_pair(sName, pVariable));
		m_VariableList.push_back(pVariable);

		return pVariable;

	}



	void CStateJournalImpl::startRecording()
	{
		{ 
			std::lock_guard<std::mutex> lockGuard(m_Mutex);

			if (m_JournalMode != eStateJournalMode::sjmInitialising)
				throw ELibMCInterfaceException(LIBMC_ERROR_JOURNALISNOTINITIALISING);

			m_JournalMode = eStateJournalMode::sjmRecording;
		}

		m_pStream->setVariableCount(m_VariableList.size());

		m_ThreadStopFlag = false;
		m_ThreadFuture = std::async(std::launch::async, [this] { this->recordingThread(); });


	}

	void CStateJournalImpl::finishRecording()
	{
		
		// Close thread
		m_ThreadStopFlag = true;
		if (m_ThreadFuture.valid())
			m_ThreadFuture.wait();
		m_ThreadStopFlag = false;

		std::lock_guard<std::mutex> lockGuard(m_Mutex);

		if (m_JournalMode != eStateJournalMode::sjmRecording)
			throw ELibMCInterfaceException(LIBMC_ERROR_JOURNALISNOTRECORDING);

		m_JournalMode = eStateJournalMode::sjmFinished;

	}

	void CStateJournalImpl::recordingThread()
	{
		while (!m_ThreadStopFlag) {
			try {
				m_pStream->serializeChunksThreaded();
			}
			catch (std::exception & E) {
				m_pLogger->logMessage ("could not serialize journal chunk: " + std::string (E.what ()), LOG_SUBSYSTEM_SYSTEM, AMC::eLogLevel::FatalError);
				throw;
			}


			try {
				m_pStream->writeChunksToDiskThreaded();
			}
			catch (std::exception& E) {
				m_pLogger->logMessage("could not write journal chunk to disk: " + std::string(E.what()), LOG_SUBSYSTEM_SYSTEM, AMC::eLogLevel::FatalError);
				throw;
			}

			std::this_thread::sleep_for(std::chrono::seconds (m_nChunkWriteIntervalInSeconds));
		}
	}


	void CStateJournalImpl::updateBoolValue(const uint32_t nVariableID, const bool bValue)
	{

		std::lock_guard<std::mutex> lockGuard(m_Mutex);

		if (m_JournalMode != eStateJournalMode::sjmRecording)
			throw ELibMCInterfaceException(LIBMC_ERROR_JOURNALISNOTRECORDING);

		auto iIter = m_VariableIDMap.find(nVariableID);
		auto pBoolVariable = std::dynamic_pointer_cast<CStateJournalImplBoolVariable> (iIter->second);
		if (pBoolVariable.get () == nullptr)
			throw ELibMCInterfaceException(LIBMC_ERROR_INVALIDVARIABLETYPE);

		pBoolVariable->setValue_MicroSecond (bValue, retrieveTimeStamp_MicroSecond());
	}

	void CStateJournalImpl::updateIntegerValue(const uint32_t nVariableID, const int64_t nValue)
	{
		std::lock_guard<std::mutex> lockGuard(m_Mutex);

		if (m_JournalMode != eStateJournalMode::sjmRecording)
			throw ELibMCInterfaceException(LIBMC_ERROR_JOURNALISNOTRECORDING);

		auto iIter = m_VariableIDMap.find(nVariableID);
		auto pIntegerVariable = std::dynamic_pointer_cast<CStateJournalImplIntegerVariable> (iIter->second);
		if (pIntegerVariable.get() == nullptr)
			throw ELibMCInterfaceException(LIBMC_ERROR_INVALIDVARIABLETYPE);

		pIntegerVariable->setValue_MicroSecond(nValue, retrieveTimeStamp_MicroSecond());

	}

	void CStateJournalImpl::updateStringValue(const uint32_t nVariableID, const std::string& sValue)
	{
		std::lock_guard<std::mutex> lockGuard(m_Mutex);

		if (m_JournalMode != eStateJournalMode::sjmRecording)
			throw ELibMCInterfaceException(LIBMC_ERROR_JOURNALISNOTRECORDING);

		auto iIter = m_VariableIDMap.find(nVariableID);
		auto pStringVariable = std::dynamic_pointer_cast<CStateJournalImplStringVariable> (iIter->second);
		if (pStringVariable.get() == nullptr)
			throw ELibMCInterfaceException(LIBMC_ERROR_INVALIDVARIABLETYPE);

		pStringVariable->setValue_MicroSecond(sValue, retrieveTimeStamp_MicroSecond());
	}

	void CStateJournalImpl::updateDoubleValue(const uint32_t nVariableID, const double dValue)
	{
		std::lock_guard<std::mutex> lockGuard(m_Mutex);

		if (m_JournalMode != eStateJournalMode::sjmRecording)
			throw ELibMCInterfaceException(LIBMC_ERROR_JOURNALISNOTRECORDING);

		auto iIter = m_VariableIDMap.find(nVariableID);
		auto pDoubleVariable = std::dynamic_pointer_cast<CStateJournalImplDoubleVariable> (iIter->second);
		if (pDoubleVariable.get() == nullptr)
			throw ELibMCInterfaceException(LIBMC_ERROR_INVALIDVARIABLETYPE);

		pDoubleVariable->setValue_MicroSecond(dValue, retrieveTimeStamp_MicroSecond());

	}

	void CStateJournalImpl::readDoubleTimeStream(const std::string& sName, const sStateJournalInterval& interval, std::vector<sJournalTimeStreamDoubleEntry>& timeStream)
	{
		std::lock_guard<std::mutex> lockGuard(m_Mutex);
		if (m_JournalMode != eStateJournalMode::sjmRecording)
			throw ELibMCInterfaceException(LIBMC_ERROR_JOURNALISNOTRECORDING);

		auto iIter = m_VariableStringMap.find(sName);
		auto pDoubleVariable = std::dynamic_pointer_cast<CStateJournalImplDoubleVariable> (iIter->second);
		if (pDoubleVariable.get() == nullptr)
			throw ELibMCCustomException(LIBMC_ERROR_INVALIDVARIABLETYPE, sName);

		pDoubleVariable->readTimeStream(interval,  timeStream);

	}


	uint64_t CStateJournalImpl::retrieveTimeStamp_MicroSecond()
	{
		return m_Chrono.getExistenceTimeInMicroseconds();
	}



	
	CStateJournal::CStateJournal(PStateJournalStream pStream)
		: m_pImpl(std::make_shared<CStateJournalImpl> (pStream))
	{
	
	}

	CStateJournal::~CStateJournal()
	{

	}

	void CStateJournal::startRecording()
	{
		m_pImpl->startRecording();
	}

	void CStateJournal::finishRecording()
	{
		m_pImpl->finishRecording();
	}

	uint32_t CStateJournal::registerBooleanValue(const std::string& sName, const bool bInitialValue)
	{
		auto pVariable = m_pImpl->generateVariable(eStateJournalVariableType::vtBoolParameter, sName);
		auto pBoolVariable = std::dynamic_pointer_cast<CStateJournalImplBoolVariable> (pVariable);
		if (pBoolVariable.get() == nullptr)
			throw ELibMCInterfaceException(LIBMC_ERROR_INVALIDVARIABLETYPE);

		pBoolVariable->setInitialValue(bInitialValue);

		return pVariable->getID ();
	}

	uint32_t CStateJournal::registerIntegerValue(const std::string& sName, const int64_t nInitialValue)
	{
		auto pVariable = m_pImpl->generateVariable(eStateJournalVariableType::vtIntegerParameter, sName);		
		auto pIntegerVariable = std::dynamic_pointer_cast<CStateJournalImplIntegerVariable> (pVariable);
		if (pIntegerVariable.get() == nullptr)
			throw ELibMCInterfaceException(LIBMC_ERROR_INVALIDVARIABLETYPE);

		pIntegerVariable->setInitialValue(nInitialValue);

		return pVariable->getID();
	}

	uint32_t CStateJournal::registerStringValue(const std::string& sName, const std::string & sInitialValue)
	{
		auto pVariable = m_pImpl->generateVariable(eStateJournalVariableType::vtStringParameter, sName);	
		auto pStringVariable = std::dynamic_pointer_cast<CStateJournalImplStringVariable> (pVariable);
		if (pStringVariable.get() == nullptr)
			throw ELibMCInterfaceException(LIBMC_ERROR_INVALIDVARIABLETYPE);

		pStringVariable->setInitialValue(sInitialValue);

		return pVariable->getID();
	}

	uint32_t CStateJournal::registerDoubleValue(const std::string& sName, const double dInitialValue, double dUnits)
	{
		dUnits = 1.0;
		if ((dUnits < STATEJOURNAL_VARIABLE_MINUNITS) || (dUnits > STATEJOURNAL_VARIABLE_MAXUNITS))
			throw ELibMCInterfaceException(LIBMC_ERROR_INVALIDVARIABLEUNITS);

		auto pVariable = m_pImpl->generateVariable(eStateJournalVariableType::vtDoubleParameter, sName);
		auto pDoubleVariable = std::dynamic_pointer_cast<CStateJournalImplDoubleVariable> (pVariable);
		if (pDoubleVariable.get() == nullptr)
			throw ELibMCInterfaceException(LIBMC_ERROR_INTERNALERROR);

		pDoubleVariable->setUnits(dUnits);
		pDoubleVariable->setInitialValue(dInitialValue);
		return pVariable->getID();
	}



	void CStateJournal::updateBoolValue(const uint32_t nVariableID, const bool bValue)
	{
		m_pImpl->updateBoolValue(nVariableID, bValue);
	}

	void CStateJournal::updateIntegerValue(const uint32_t nVariableID, const int64_t nValue)
	{
		m_pImpl->updateIntegerValue(nVariableID, nValue);
	}

	void CStateJournal::updateStringValue(const uint32_t nVariableID, const std::string& sValue)
	{
		m_pImpl->updateStringValue(nVariableID, sValue);
	}

	void CStateJournal::updateDoubleValue(const uint32_t nVariableID, const double dValue)
	{
		m_pImpl->updateDoubleValue(nVariableID, dValue);
	}

	void CStateJournal::readDoubleTimeStream(const std::string& sName, const sStateJournalInterval& interval, std::vector<sJournalTimeStreamDoubleEntry>& timeStream)
	{
		m_pImpl->readDoubleTimeStream(sName, interval, timeStream);
	}


	sStateJournalStatistics CStateJournal::computeStatistics(const std::string& sName, const sStateJournalInterval& interval)
	{
		std::vector<sJournalTimeStreamDoubleEntry> timeStream;

		sStateJournalStatistics resultStatistics;
		resultStatistics.m_Interval = interval;
		resultStatistics.m_dMinValue = 0.0;
		resultStatistics.m_dMaxValue = 0.0;
		resultStatistics.m_dAverageValue = 0.0;
		resultStatistics.m_dAverageSquaredValue = 0.0;
		resultStatistics.m_dVariance = 0.0;


		if (interval.m_nStartTimeInMicroSeconds >= interval.m_nEndTimeInMicroSeconds)
			throw ELibMCInterfaceException(LIBMC_ERROR_INVALIDJOURNALCOMPUTEINTERVAL);

		uint64_t nTimeDifferenceInMicroSeconds = interval.m_nEndTimeInMicroSeconds - interval.m_nStartTimeInMicroSeconds;

		readDoubleTimeStream(sName, interval, timeStream);

		uint64_t nTimeStepCount = timeStream.size();

		if (nTimeStepCount == 0)
			return resultStatistics;

		auto& firstEntry = timeStream.at(0);
		auto& lastEntry = timeStream.at(nTimeStepCount - 1);

		if (firstEntry.m_nTimeStampInMicroSeconds < interval.m_nStartTimeInMicroSeconds)
			throw ELibMCInterfaceException(LIBMC_ERROR_INVALIDJOURNALCOMPUTEINTERVAL);

		if (lastEntry.m_nTimeStampInMicroSeconds > interval.m_nEndTimeInMicroSeconds)
			throw ELibMCInterfaceException(LIBMC_ERROR_INVALIDJOURNALCOMPUTEINTERVAL);

		// First Interval
		double dIntegral = 0;
		double dSquaredIntegral = 0;
		double dCurrentValue = firstEntry.m_dValue;
		double dMinValue = dCurrentValue;
		double dMaxValue = dCurrentValue;

		uint64_t nCurrentTime = interval.m_nStartTimeInMicroSeconds;

		for (uint64_t nIndex = 0; nIndex < nTimeStepCount; nIndex++) {
			auto& nextEntry = timeStream.at(nIndex);

			if (nextEntry.m_nTimeStampInMicroSeconds < nCurrentTime)
				throw ELibMCInterfaceException(LIBMC_ERROR_INVALIDJOURNALCOMPUTEDATA);

			uint64_t nDeltaTime = nextEntry.m_nTimeStampInMicroSeconds - nCurrentTime;

			dIntegral += dCurrentValue * nDeltaTime;
			dSquaredIntegral += dCurrentValue * dCurrentValue * nDeltaTime;

			nCurrentTime = nextEntry.m_nTimeStampInMicroSeconds;
			dCurrentValue = nextEntry.m_dValue;

			if (dCurrentValue < dMinValue)
				dMinValue = dCurrentValue;
			if (dCurrentValue > dMaxValue)
				dMaxValue = dCurrentValue;
		}

		dIntegral += dCurrentValue * (interval.m_nEndTimeInMicroSeconds - nCurrentTime);

		resultStatistics.m_dMinValue = dMinValue;
		resultStatistics.m_dMaxValue = dMaxValue;
		resultStatistics.m_dAverageValue = dIntegral / ((double)nTimeDifferenceInMicroSeconds);
		resultStatistics.m_dAverageSquaredValue = dSquaredIntegral / ((double)nTimeDifferenceInMicroSeconds);
		resultStatistics.m_dVariance = resultStatistics.m_dAverageSquaredValue - resultStatistics.m_dAverageValue * resultStatistics.m_dAverageValue;

		return resultStatistics;

	}


	void CStateJournal::retrieveRecentInterval(uint64_t nLastMilliSeconds, sStateJournalInterval& interval)
	{
		uint64_t nLastMicroSeconds = nLastMilliSeconds * 1000;

		interval.m_nEndTimeInMicroSeconds = m_pImpl->retrieveTimeStamp_MicroSecond();
		if (nLastMicroSeconds < interval.m_nEndTimeInMicroSeconds)
			interval.m_nStartTimeInMicroSeconds = interval.m_nEndTimeInMicroSeconds - nLastMicroSeconds;
		else
			interval.m_nStartTimeInMicroSeconds = 0;
		
		if (interval.m_nStartTimeInMicroSeconds >= interval.m_nEndTimeInMicroSeconds)
			throw ELibMCInterfaceException(LIBMC_ERROR_INVALIDTIMESTAMPINTERVAL);
	}

}


