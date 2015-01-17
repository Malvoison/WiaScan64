// dllmain.h : Declaration of module class.

class CUdvcDriverModule : public ATL::CAtlDllModuleT< CUdvcDriverModule >
{
public :
	DECLARE_LIBID(LIBID_UdvcDriverLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_UDVCDRIVER, "{5366B88E-6230-4FC7-90EB-EFA2F34CFE5D}")
};

extern class CUdvcDriverModule _AtlModule;
