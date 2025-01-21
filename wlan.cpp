#include <windows.h>
#include <wlanapi.h>

extern "C" {
#include <stdio.h>
#include <stdlib.h>
}

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#pragma comment(lib, "wlanapi.lib")
#pragma comment(lib, "ole32.lib")

void printSSID(DOT11_SSID ssid) {
    // O SSID não é garantido como string nula. Vamos exibir corretamente os bytes.
    if (ssid.uSSIDLength > 0) {
        printf("SSID: ");
        for (unsigned int i = 0; i < ssid.uSSIDLength; i++) {
            printf("%c", ssid.ucSSID[i]);
        }
        printf("\n");
    } else {
        printf("SSID: (sem nome)\n");
    }
}

void writeSSIDToFile(std::ofstream &file, const DOT11_SSID &ssid) {
    if (ssid.uSSIDLength > 0) {
        file << "SSID: ";
        for (unsigned int i = 0; i < ssid.uSSIDLength; i++) {
            file << static_cast<char>(ssid.ucSSID[i]);
        }
        file << "\n";
    } else {
        file << "SSID: (sem nome)\n";
    }
}

int main() {
    HANDLE hClient = NULL;
    DWORD dwMaxClient = 2; // Versão cliente
    DWORD dwCurVersion = 0;
    DWORD dwResult = 0;
    DWORD dwRetVal = 0;

    unsigned int i, j;
    PWLAN_INTERFACE_INFO_LIST pIfList = NULL;
    PWLAN_INTERFACE_INFO pIfInfo = NULL;
    PWLAN_AVAILABLE_NETWORK_LIST pBssList = NULL;
    PWLAN_AVAILABLE_NETWORK pBssEntry = NULL;

    // Abre um handle para a WLAN API
    dwResult = WlanOpenHandle(dwMaxClient, NULL, &dwCurVersion, &hClient);
    if (dwResult != ERROR_SUCCESS) {
        printf("WlanOpenHandle falhou com erro: %u\n", dwResult);
        return 1;
    }

    // Obtém a lista de interfaces de rede disponíveis
    dwResult = WlanEnumInterfaces(hClient, NULL, &pIfList);
    if (dwResult != ERROR_SUCCESS) {
        printf("WlanEnumInterfaces falhou com erro: %u\n", dwResult);
        return 1;
    }

    static PWLAN_AVAILABLE_NETWORK gen;
    std::ofstream _of("wlan_file.txt", std::ios::out);

    for (i = 0; i < pIfList->dwNumberOfItems; i++) {
        pIfInfo = (PWLAN_INTERFACE_INFO)&pIfList->InterfaceInfo[i];

        wprintf(L"\nInterface encontrada: %ws\n", pIfInfo->strInterfaceDescription);

        // Obtém as redes disponíveis para essa interface
        dwResult = WlanGetAvailableNetworkList(
            hClient,
            &pIfInfo->InterfaceGuid,
            0,
            NULL,
            &pBssList
        );

        if (dwResult != ERROR_SUCCESS) {
            printf("WlanGetAvailableNetworkList falhou com erro: %u\n", dwResult);
            continue;
        }

        // Lista as redes disponíveis
        for (j = 0; j < pBssList->dwNumberOfItems; j++) {
            pBssEntry = (PWLAN_AVAILABLE_NETWORK)&pBssList->Network[j];

            gen = pBssEntry;

            if(gen->dot11Ssid.uSSIDLength > 0) {
                _of << "\n";
                _of << "SSID: " << gen->dot11Ssid.ucSSID << "\n";
                _of << "Signal: " << gen->wlanSignalQuality << "% \n";
                _of << "Security: " << (gen->bSecurityEnabled ? "Sim" : "Não") << "\n";

            }

            // Imprime o SSID usando a função personalizada
            printSSID(pBssEntry->dot11Ssid);

            wprintf(L"Tipo de Rede: %u\n", pBssEntry->dot11BssType);
            wprintf(L"Força do Sinal: %ld%%\n", pBssEntry->wlanSignalQuality);
            wprintf(L"Segurança Ativada: %s\n", pBssEntry->bSecurityEnabled ? L"Sim" : L"Não");
        }

        WlanFreeMemory(pBssList);
        pBssList = NULL;
    }

    _of.close();

    // Libera a memória alocada para a lista de interfaces
    if (pIfList != NULL) {
        WlanFreeMemory(pIfList);
        pIfList = NULL;
    }

    // Fecha o handle da WLAN API
    WlanCloseHandle(hClient, NULL);

    return 0;
}
