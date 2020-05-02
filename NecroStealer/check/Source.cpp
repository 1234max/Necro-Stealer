#include <windows.h>
#include <wingdi.h>

PBYTE doScreenshot(DWORD* dw)
{
	HDC hdcScr, hdcMem;
	HBITMAP hbmScr;
	BITMAP bmp;
	int iXRes, iYRes;

	hdcScr = CreateDCA("DISPLAY", NULL, NULL, NULL);
	hdcMem = CreateCompatibleDC(hdcScr);
	iXRes = GetDeviceCaps(hdcScr, HORZRES);
	iYRes = GetDeviceCaps(hdcScr, VERTRES);
	hbmScr = CreateCompatibleBitmap(hdcScr, iXRes, iYRes);
	if (hbmScr == 0) return 0;
	if (!SelectObject(hdcMem, hbmScr)) return 0;
	if (!StretchBlt(hdcMem, 0, 0, iXRes, iYRes, hdcScr, 0, 0, iXRes, iYRes, SRCCOPY)) return 0;

	PBITMAPINFO pbmi;
	WORD cClrBits;

	if (!GetObjectW(hbmScr, sizeof(BITMAP), (LPSTR)&bmp)) return 0;

	cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel);
	if (cClrBits == 1) cClrBits = 1;
	else if (cClrBits <= 4) cClrBits = 4;
	else if (cClrBits <= 8) cClrBits = 8;
	else if (cClrBits <= 16) cClrBits = 16;
	else if (cClrBits <= 24) cClrBits = 24;
	else cClrBits = 32;
	if (cClrBits != 24)
		pbmi = (PBITMAPINFO)LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * (1 << cClrBits));

	else
		pbmi = (PBITMAPINFO)LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER));

	pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pbmi->bmiHeader.biWidth = bmp.bmWidth;
	pbmi->bmiHeader.biHeight = bmp.bmHeight;
	pbmi->bmiHeader.biPlanes = bmp.bmPlanes;
	pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel;
	if (cClrBits < 24) pbmi->bmiHeader.biClrUsed = (1 << cClrBits);

	pbmi->bmiHeader.biCompression = BI_RGB;
	pbmi->bmiHeader.biSizeImage = (pbmi->bmiHeader.biWidth + 7) / 8
		* pbmi->bmiHeader.biHeight * cClrBits;
	pbmi->bmiHeader.biClrImportant = 0;

	HANDLE hf;
	BITMAPFILEHEADER hdr;
	PBITMAPINFOHEADER pbih;
	LPBYTE lpBits;
	DWORD dwTotal;
	DWORD cb;
	BYTE *hp;
	DWORD dwTmp;

	pbih = (PBITMAPINFOHEADER)pbmi;
	lpBits = (LPBYTE)GlobalAlloc(GMEM_FIXED, pbih->biSizeImage);

	if (!lpBits) return 0;
	if (!GetDIBits(hdcMem, hbmScr, 0, (WORD)pbih->biHeight, lpBits, pbmi, DIB_RGB_COLORS)) return 0;

	hdr.bfType = 0x4d42;
	hdr.bfSize = (DWORD)(sizeof(BITMAPFILEHEADER) + pbih->biSize + pbih->biClrUsed * sizeof(RGBQUAD) + pbih->biSizeImage);
	hdr.bfReserved1 = 0;
	hdr.bfReserved2 = 0;
	hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) + pbih->biSize + pbih->biClrUsed * sizeof(RGBQUAD);

	*dw = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (pbih->biClrUsed * sizeof(RGBQUAD) + pbih->biSizeImage);
	PBYTE screenMem = (PBYTE)malloc(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + pbih->biSizeImage + (sizeof(BITMAPINFOHEADER) + pbih->biClrUsed * sizeof(RGBQUAD)));
	memcpy(screenMem, (LPVOID)&hdr, sizeof(BITMAPFILEHEADER));
	memcpy(screenMem + sizeof(BITMAPFILEHEADER), (LPVOID)pbih, sizeof(BITMAPINFOHEADER) + pbih->biClrUsed * sizeof(RGBQUAD));

	dwTotal = cb = pbih->biSizeImage;
	hp = lpBits;
	memcpy(screenMem + sizeof(BITMAPFILEHEADER) + (sizeof(BITMAPINFOHEADER) + pbih->biClrUsed * sizeof(RGBQUAD)), (LPSTR)hp, (int)cb);

	GlobalFree((HGLOBAL)lpBits);
	ReleaseDC(0, hdcScr);
	ReleaseDC(0, hdcMem);

	return screenMem;
}

int main() {
	DWORD l = 0;
	PBYTE scr = doScreenshot(&l);

	DWORD dw;
	HANDLE h = CreateFileA("pic.bmp", GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	WriteFile(h, scr, l, &dw, 0);
	CloseHandle(h);

	return 0;
}