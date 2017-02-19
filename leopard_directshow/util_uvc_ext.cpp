#include "util_uvc_ext.h"
#include <Vidcap.h>
#include <ksmedia.h>
#include <ksproxy.h>
#include <thread>
#include <windows.h>
#include <initguid.h>
#include <ks.h>


// {78E321E1-C8AC-40A5-8AC9-75A2A02C74FB}
DEFINE_GUID(GUID_EXTENSION_UNIT_DESCRIPTOR,
	0x78e321e1, 0xc8ac, 0x40a5, 0x8a, 0xc9, 0x75, 0xa2, 0xa0, 0x2c, 0x74, 0xfb);

int util_uvc_ext::write_to_uvc_extension(IBaseFilter* camera, int property_id, BYTE* bytes, int length, ULONG* ulBytesReturned)
{
	HRESULT hr;
	IKsTopologyInfo *pKsTopologyInfo;
	hr = camera->QueryInterface(__uuidof(IKsTopologyInfo), (void **)&pKsTopologyInfo);

	DWORD numberOfNodes;	hr = pKsTopologyInfo->get_NumNodes(&numberOfNodes);

	DWORD i;	GUID nodeGuid;
	for (i = 0; i < numberOfNodes; i++)
	{
		if (SUCCEEDED(pKsTopologyInfo->get_NodeType(i, &nodeGuid)))
		{
			if (nodeGuid == KSNODETYPE_DEV_SPECIFIC)
			{ // Found the extension node
				DWORD pNodeId = i;
				IKsNodeControl *pUnk;
				IKsControl *pKsControl;

				// create node instance
				hr = pKsTopologyInfo->CreateNodeInstance(i, __uuidof(IUnknown), (VOID**)&pUnk);
				hr = pUnk->QueryInterface(__uuidof(IKsControl), (VOID**)&pKsControl);

				KSP_NODE  s;	//ULONG  ulBytesReturned;

								// this is guid of our device extension unit
				s.Property.Set = GUID_EXTENSION_UNIT_DESCRIPTOR;
				s.Property.Id = property_id;
				s.Property.Flags = KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_TOPOLOGY;
				s.NodeId = i;
				hr = pKsControl->KsProperty((PKSPROPERTY)&s, sizeof(s), bytes, length, ulBytesReturned);

				if (hr == S_OK)	return 0;
				else return -1;
			}
		}
	}

	return -1;
}


int util_uvc_ext::read_from_uvc_extension(IBaseFilter* camera, int property_id, BYTE* bytes, int length, ULONG* ulBytesReturned)
{
	HRESULT hr;
	IKsTopologyInfo *pKsTopologyInfo;
	hr = camera->QueryInterface(__uuidof(IKsTopologyInfo), (void **)&pKsTopologyInfo);

	DWORD numberOfNodes;
	hr = pKsTopologyInfo->get_NumNodes(&numberOfNodes);

	DWORD i;	GUID nodeGuid;
	for (i = 0; i < numberOfNodes; i++)
	{
		if (SUCCEEDED(pKsTopologyInfo->get_NodeType(i, &nodeGuid)))
		{
			if (nodeGuid == KSNODETYPE_DEV_SPECIFIC)
			{ // Found the extension node
				DWORD pNodeId = i;
				IKsNodeControl *pUnk;
				IKsControl *pKsControl;

				// create node instance
				hr = pKsTopologyInfo->CreateNodeInstance(i, __uuidof(IUnknown), (VOID**)&pUnk);
				hr = pUnk->QueryInterface(__uuidof(IKsControl), (VOID**)&pKsControl);

				KSP_NODE  s;

				// this is guid of our device extension unit
				s.Property.Set = GUID_EXTENSION_UNIT_DESCRIPTOR;
				s.Property.Id = property_id;
				s.Property.Flags = KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_TOPOLOGY;
				s.NodeId = i;
				hr = pKsControl->KsProperty((PKSPROPERTY)&s, sizeof(s), bytes, length, ulBytesReturned);

				return 0;
			}
		}
	}

	return -1;
}