#include <Windows.h>
#include <iostream>
#include <string>
#include "clip.h"

void cpy_clipboard_set(const std::wstring& text) {
    if (!OpenClipboard(nullptr)) {
        std::cout << "ERR: Failed to open clipboard \n";
        return;
    }

    EmptyClipboard();

    size_t sz = (text.size() +1) * sizeof (wchar_t);

    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, sz);
    if (!hGlobal) {
        CloseClipboard();
        std::cout << "ERR: Failed to allocation text in the board \n";
        return;
    }

    void* pGlobal = GlobalLock(hGlobal);
    if (pGlobal) {
        std::memcpy(pGlobal, text.c_str(), sz);
        GlobalUnlock(hGlobal);
    }
    else {
        GlobalFree(hGlobal);
        CloseClipboard();
        std::cout << "Failed to insert data in clipboard \n";
        return;
    }

    if (!SetClipboardData(CF_UNICODETEXT, hGlobal)) {
        GlobalFree(hGlobal);
        CloseClipboard();
    }

    HANDLE hGetData = GetClipboardData(CF_UNICODETEXT);
    if (hGetData) {
        std::wcout << past_cpy_clipboard(text) << std::endl;
    }

    CloseClipboard();
}

int main() {
    std::wstring text = L"Hello, World. I am DIOGO";
    cpy_clipboard_set(text);

    return 0;
}