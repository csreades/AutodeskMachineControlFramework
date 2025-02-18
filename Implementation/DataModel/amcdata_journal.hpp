/*++

Copyright (C) 2023 Autodesk Inc.

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


#ifndef __AMCDATA_JOURNAL
#define __AMCDATA_JOURNAL

#include <memory>
#include <list>
#include <vector>
#include <mutex>
#include <atomic>

#include "amcdata_sqlhandler.hpp"
#include "common_exportstream_native.hpp"
#include "libmcdata_types.hpp"

namespace AMCData {
	

	class CJournal {
	private:

		AMCData::PSQLHandler m_pSQLHandler;
		std::mutex m_LogMutex;
		std::mutex m_JournalMutex;
		std::atomic<uint32_t> m_LogID;
		std::atomic<uint32_t> m_AlertID;		

		AMCCommon::PExportStream_Native m_pJournalStream;
		
	public:

		CJournal(const std::string& sJournalPath, const std::string& sJournalDataPath);

		virtual ~CJournal();

		uint32_t getSchemaVersion();

		void AddEntry(const std::string& sMessage, const std::string& sSubSystem, const LibMCData::eLogLevel logLevel, const std::string& sTimestamp);

		LibMCData_uint32 GetMaxLogEntryID();

		void WriteJournalChunkIntegerData(const LibMCData_uint32 nChunkIndex, const LibMCData_uint64 nStartTimeStamp, const LibMCData_uint64 nEndTimeStamp, const LibMCData_uint64 nVariableInfoBufferSize, const LibMCData::sJournalChunkVariableInfo* pVariableInfoBuffer, const LibMCData_uint64 nEntryDataBufferSize, const LibMCData::sJournalChunkIntegerEntry* pEntryDataBuffer);

		AMCData::PSQLHandler getSQLHandler();

		void addAlert(const std::string& sUUID, const std::string& sIdentifier, const LibMCData::eAlertLevel eLevel, const std::string& sDescription, const std::string& sDescriptionIdentifier, const std::string& sReadableContextInformation, const bool bNeedsAcknowledgement, const std::string& sTimestampUTC);


		bool hasAlert(const std::string& sUUID);

		void getAlertInformation(const std::string& sUUID, std::string& sIdentifier, LibMCData::eAlertLevel& eLevel, std::string& sDescription, std::string& sDescriptionIdentifier, std::string& sReadableContextInformation, bool& bNeedsAcknowledgement, std::string& sTimestampUTC);

		void acknowledgeAlert(const std::string& sUUID, const std::string& sUserUUID, const std::string& sUserComment, const std::string & sTimeStampUTC);

		bool alertHasBeenAcknowledged(const std::string& sUUID);

		void getAcknowledgementInformation(const std::string& sUUID, std::string& sUserUUID, std::string& sUserComment, std::string& sTimestampUTC);


	};

	typedef std::shared_ptr<CJournal> PJournal;
	
}


#endif //__AMCDATA_JOURNAL

