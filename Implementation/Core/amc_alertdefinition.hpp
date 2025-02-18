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


#ifndef __AMC_ALERTDEFINITION
#define __AMC_ALERTDEFINITION

#include <string>
#include <memory>

#include "libmcdata_dynamic.hpp"
#include "amc_languagestring.hpp"
#include "amc_accesspermission.hpp"

namespace AMC {

	class CAlertDefinition;
	typedef std::shared_ptr<CAlertDefinition> PAlertDefinition;


	class CAlertDefinition {
	private:
	
		std::string m_sIdentifier;
		LibMCData::eAlertLevel m_AlertLevel;
		CLanguageString m_sDescription;
		
		bool m_bNeedAcknowledgement;
		std::string m_sAckPermissionIdentifier;
		
		
	public:

		CAlertDefinition (const std::string & sIdentifier, LibMCData::eAlertLevel alertLevel, const CLanguageString & sDescription, bool bNeedsAcknowledgement);
		
		virtual ~CAlertDefinition();

		std::string getIdentifier ();

		LibMCData::eAlertLevel getAlertLevel ();

		CLanguageString getDescription();

		std::string getTranslatedDescription(CLanguageDefinition * pLanguage);
		
		bool needsAcknowledgement ();

		void setAckPermissionIdentifier (std::string & sAckPermissionIdentifier);

		std::string getAckPermissionIdentifier ();


	};

	
}


#endif //__AMC_ALERTDEFINITION

