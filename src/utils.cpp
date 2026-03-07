// utils.cpp
#include "utils.h"
#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")

using namespace Gdiplus;

Image* LoadImageFromResource(HINSTANCE hInstance, int resourceId, IStream** outStream) {
    HRSRC hResource = FindResourceW(hInstance, MAKEINTRESOURCEW(resourceId), (LPCWSTR)RT_RCDATA);
    if (!hResource) return nullptr;

    DWORD imageSize = SizeofResource(hInstance, hResource);
    HGLOBAL hGlobal = LoadResource(hInstance, hResource);
    void* pData = LockResource(hGlobal);

    HGLOBAL hBuffer = GlobalAlloc(GMEM_MOVEABLE, imageSize);
    if (!hBuffer) return nullptr;

    void* pBuffer = GlobalLock(hBuffer);
    CopyMemory(pBuffer, pData, imageSize);
    GlobalUnlock(hBuffer);

    IStream* pStream = nullptr;
    if (CreateStreamOnHGlobal(hBuffer, TRUE, &pStream) == S_OK) {
        if (outStream) *outStream = pStream; 
        return Image::FromStream(pStream);
    }
    
    GlobalFree(hBuffer);
    return nullptr;
}