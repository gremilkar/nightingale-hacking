/* vim: set sw=2 :miv */
/*
//
// BEGIN SONGBIRD GPL
// 
// This file is part of the Songbird web player.
//
// Copyright(c) 2005-2008 POTI, Inc.
// http://songbirdnest.com
// 
// This file may be licensed under the terms of of the
// GNU General Public License Version 2 (the "GPL").
// 
// Software distributed under the License is distributed 
// on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either 
// express or implied. See the GPL for the specific language 
// governing rights and limitations.
//
// You should have received a copy of the GPL along with this 
// program. If not, go to http://www.gnu.org/licenses/gpl.html
// or write to the Free Software Foundation, Inc., 
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
// 
// END SONGBIRD GPL
//
*/

#include "sbWPDDeviceController.h"
#include "sbWPDMarshall.h"
#include "sbWPDDevice.h"

#include "sbDeviceCompatibility.h"

#include <nsAutoLock.h>
#include <nsAutoPtr.h>
#include <nsComponentManagerUtils.h>
#include <nsIClassInfoImpl.h>
#include <nsIGenericFactory.h>
#include <nsIProgrammingLanguage.h>
#include <nsIPropertyBag.h>
#include <nsIVariant.h>
#include <nsMemory.h>
#include <nsServiceManagerUtils.h>

NS_IMPL_THREADSAFE_ADDREF(sbWPDDeviceController)
NS_IMPL_THREADSAFE_RELEASE(sbWPDDeviceController)

NS_IMPL_THREADSAFE_QUERY_INTERFACE3(sbWPDDeviceController,
                                    sbIDeviceController,
                                    sbIDeviceEventTarget,
                                    nsIClassInfo)

NS_IMPL_CI_INTERFACE_GETTER2(sbWPDDeviceController,
                             sbIDeviceController,
                             sbIDeviceEventTarget)

NS_DECL_CLASSINFO(sbWPDDeviceController)
NS_IMPL_THREADSAFE_CI(sbWPDDeviceController)

SB_DEVICE_CONTROLLER_REGISTERSELF_IMPL(sbWPDDeviceController, 
                                       SB_WPDCONTROLLER_CONTRACTID)

sbWPDDeviceController::sbWPDDeviceController() 
: mMonitor(nsnull) 
{
  
  mMonitor = nsAutoMonitor::NewMonitor(__FILE__);
  NS_ASSERTION(mMonitor, "Failed to create monitor");

  static nsID const id = SB_WPDCONTROLLER_CID;
  nsresult rv = SetControllerIdInternal(id);
  NS_ASSERTION(NS_SUCCEEDED(rv), "Failed to set controller id");

  static nsID const marshallId = SB_WPDMARSHALL_CID;
  rv = SetMarshallIdInternal(marshallId);
  NS_ASSERTION(NS_SUCCEEDED(rv), "Failed to set controller id");

  rv = SetControllerNameInternal(NS_LITERAL_STRING("sbWPDDeviceController"));
  NS_ASSERTION(NS_SUCCEEDED(rv), "Failed to set controller name");
}

sbWPDDeviceController::~sbWPDDeviceController() 
{
  if(mMonitor) {
    nsAutoMonitor::DestroyMonitor(mMonitor);
  }
}

/* readonly attribute nsIDPtr id; */
NS_IMETHODIMP
sbWPDDeviceController::GetId(nsID * *aId) 
{
  NS_ENSURE_ARG_POINTER(aId);

  *aId = nsnull;

  *aId = static_cast<nsID*>(NS_Alloc(sizeof(nsID)));
  nsresult rv = GetControllerIdInternal(**aId);
  
  if(NS_FAILED(rv)) {
    NS_Free(*aId);
    *aId = nsnull;
  }

  return rv;
}

/* readonly attribute AString name; */
NS_IMETHODIMP 
sbWPDDeviceController::GetName(nsAString & aName) 
{
  return GetControllerNameInternal(aName);
}

/* readonly attribute nsIDPtr marshallId; */
NS_IMETHODIMP 
sbWPDDeviceController::GetMarshallId(nsID * *aMarshallId)
{
  NS_ENSURE_ARG_POINTER(aMarshallId);

  *aMarshallId = nsnull;

  *aMarshallId = static_cast<nsID*>(NS_Alloc(sizeof(nsID)));
  nsresult rv = GetMarshallIdInternal(**aMarshallId);

  if(NS_FAILED(rv)) {
    NS_Free(*aMarshallId);
    *aMarshallId = nsnull;
  }

  return rv;
}

/* readonly attribute nsIArray devices; */
NS_IMETHODIMP 
sbWPDDeviceController::GetDevices(nsIArray * *aDevices)
{
  return GetDevicesInternal(aDevices);
}

/* sbIDeviceCompatibility getCompatibility (in nsIPropertyBag aParams); */
NS_IMETHODIMP 
sbWPDDeviceController::GetCompatibility(nsIPropertyBag *aParams, 
                                        sbIDeviceCompatibility **_retval)
{
  NS_ENSURE_ARG_POINTER(aParams);
  NS_ENSURE_ARG_POINTER(_retval);

  nsRefPtr<sbDeviceCompatibility> deviceCompatibility;

  NS_NEWXPCOM(deviceCompatibility, sbDeviceCompatibility);
  NS_ENSURE_TRUE(deviceCompatibility, NS_ERROR_OUT_OF_MEMORY);

  nsCOMPtr<nsIVariant> value;
  nsresult rv = aParams->GetProperty(NS_LITERAL_STRING("DeviceType"), getter_AddRefs(value));

  nsString strValue;
  rv = value->GetAsAString(strValue);
  NS_ENSURE_SUCCESS(rv, rv);

  if(strValue.EqualsLiteral("WPD")) {
    // Should be reading pref from prefs service.
    rv = 
      deviceCompatibility->Init(sbIDeviceCompatibility::COMPATIBLE_ENHANCED_SUPPORT, 
                                sbIDeviceCompatibility::PREFERENCE_SELECTED);
  }
  else {
    // Should be reading pref from prefs service.
    rv = 
      deviceCompatibility->Init(sbIDeviceCompatibility::INCOMPATIBLE, 
                                sbIDeviceCompatibility::PREFERENCE_UNKNOWN);
  }

  NS_ENSURE_SUCCESS(rv, rv);
  NS_ADDREF(*_retval = deviceCompatibility);

  return NS_OK;
}

/* sbIDevice createDevice (in nsIPropertyBag aParams); */
NS_IMETHODIMP 
sbWPDDeviceController::CreateDevice(nsIPropertyBag *aParams, 
                                    sbIDevice **_retval) 
{
  nsRefPtr<sbWPDDevice> device;

  nsID id;
  nsresult rv = GetControllerIdInternal(id);
  NS_ENSURE_SUCCESS(rv, rv);

  nsCOMPtr<nsIPropertyBag2> properties = do_QueryInterface(aParams, &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  device = sbWPDDevice::New(id, properties);
  NS_ENSURE_TRUE(device, NS_ERROR_OUT_OF_MEMORY);

  NS_ADDREF(*_retval = device);

  rv = AddDeviceInternal(device);
  NS_ENSURE_SUCCESS(rv, rv);

  return NS_OK;
}

/* boolean controlsDevice (in sbIDevice aDevice); */
NS_IMETHODIMP 
sbWPDDeviceController::ControlsDevice(sbIDevice *aDevice, 
                                      PRBool *_retval) 
{
  return ControlsDeviceInternal(aDevice, _retval);
}

/* void connectDevices (); */
NS_IMETHODIMP 
sbWPDDeviceController::ConnectDevices() 
{
  return ConnectDevicesInternal();
}

/* void disconnectDevices (); */
NS_IMETHODIMP 
sbWPDDeviceController::DisconnectDevices() 
{
  return DisconnectDevicesInternal();
}

/* void releaseDevice (in sbIDevice aDevice); */
NS_IMETHODIMP 
sbWPDDeviceController::ReleaseDevice(sbIDevice *aDevice) 
{
  return ReleaseDeviceInternal(aDevice);
}

/* void releaseDevices (); */
NS_IMETHODIMP 
sbWPDDeviceController::ReleaseDevices() 
{
  return ReleaseDevicesInternal();
}
