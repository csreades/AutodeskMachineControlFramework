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


#ifndef __AMC_AUTH
#define __AMC_AUTH


#include <memory>
#include <string>

namespace AMC {

	class CAPIAuth;
	typedef std::shared_ptr<CAPIAuth> PAPIAuth;

	class CParameterHandler;
	typedef std::shared_ptr<CParameterHandler> PParameterHandler;

	class CUserInformation;
	typedef std::shared_ptr<CUserInformation> PUserInformation;

	class CAPIAuth {
	private:

		std::string m_sSessionUUID;
		std::string m_sSessionKey;
		bool m_bIsAuthorized;

		PUserInformation m_pUserInformation;
		PParameterHandler m_pClientVariableHandler;

	protected:

	public:

		CAPIAuth(const std::string& sSessionUUID, const std::string& sSessionKey, PUserInformation pUserInformation, bool bIsAuthorized, PParameterHandler pClientVariableHandler);

		virtual ~CAPIAuth();
		
		PUserInformation getUserInformation ();

		std::string getUserName();

		std::string getSessionUUID();

		std::string getSessionKey();

		bool userIsAuthorized();

		bool contextUUIDIsAuthorized(std::string & sContextUUID);

		PParameterHandler getClientVariableHandler ();
								
	};

	
}


#endif //__AMC_AUTH

