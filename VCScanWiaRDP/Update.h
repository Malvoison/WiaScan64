//  Filename:  Update.h

#pragma once

//  UPDATE commands
#define PM_GETVERSION           WM_APP + 0x0501
#define PM_UPDATEVERSION        WM_APP + 0x0502

void DataArrivalUpdate(LPBYTE pBuf, USHORT usLength);