
CDriver
====================================================================================================


.. cpp:class:: LibMCDriver_LibOAPC::CDriver : public CBase 

	An abstract Machine Control Driver




	.. cpp:function:: std::string GetName()

		returns the name identifier of the driver

		:returns: Name of the driver.


	.. cpp:function:: std::string GetType()

		returns the type identifier of the driver

		:returns: Type of the driver.


	.. cpp:function:: void GetVersion(LibMCDriver_LibOAPC_uint32 & nMajor, LibMCDriver_LibOAPC_uint32 & nMinor, LibMCDriver_LibOAPC_uint32 & nMicro, std::string & sBuild)

		returns the version identifiers of the driver

		:param nMajor: Major version. 
		:param nMinor: Minor version. 
		:param nMicro: Micro version. 
		:param sBuild: Build identifier. 


	.. cpp:function:: void GetHeaderInformation(std::string & sNameSpace, std::string & sBaseName)

		returns the header information

		:param sNameSpace: NameSpace of the driver. 
		:param sBaseName: BaseName of the driver. 


	.. cpp:function:: void QueryParameters()

		Stores the driver parameters in the driver environment.



.. cpp:type:: std::shared_ptr<CDriver> LibMCDriver_LibOAPC::PDriver

	Shared pointer to CDriver to easily allow reference counting.

