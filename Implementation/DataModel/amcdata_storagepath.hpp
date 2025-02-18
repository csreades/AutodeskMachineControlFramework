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

*/


#ifndef __AMCDATA_STORAGEPATH
#define __AMCDATA_STORAGEPATH

#include <string>
#include <memory>

namespace AMCData {


/*************************************************************************************************************************
 Class declaration of CStoragePath
**************************************************************************************************************************/

enum class eStorageStreamStatus : int32_t {
    sssNew = 0,
    sssValidated = 1,
    sssArchived = 2
};

class CStoragePath {
private:
    std::string m_sDataPath;
    

public:

    CStoragePath(const std::string & sDataPath);

    std::string getStreamPath(const std::string& sStreamUUID);
    std::string getJournalPath(const std::string& sTimeFileName);
    std::string getJournalDataPath(const std::string& sTimeFileName);
    std::string getJournalFileName(const std::string& sTimeFileName);
    std::string getJournalDataFileName(const std::string& sTimeFileName);

    static std::string storageStreamStatusToString(eStorageStreamStatus eStatus);
    static eStorageStreamStatus stringToStorageStreamStatus(const std::string & sStatus);

};

typedef std::shared_ptr <CStoragePath> PStoragePath;

} // namespace AMCDATA

#endif // __AMCDATA_STORAGEPATH
