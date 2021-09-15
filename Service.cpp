#include "MainHeader.h"


LPCWSTR serviceName = L"Mormanti";


void InstallService() {
	SC_HANDLE serviceControlManager = OpenSCManager(0, 0, SC_MANAGER_CREATE_SERVICE);
	if (serviceControlManager) {
		TCHAR path[_MAX_PATH + 1];
		if (GetModuleFileName(0, path, sizeof(path) / sizeof(path[0])) > 0) {

			// Install
			SC_HANDLE service = CreateService(serviceControlManager, serviceName, serviceName,
				SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
				SERVICE_AUTO_START, SERVICE_ERROR_IGNORE, path,
				0, 0, 0, 0, 0);

			if (service)
				CloseServiceHandle(service);

			// Uninstall
			service = OpenService(serviceControlManager, serviceName, SERVICE_QUERY_STATUS | DELETE);
			if (service) {
				std::cout << service << "\n";

				const char* TEST = "XXXXXX";
				std::ostringstream ss;
				ss << "&HANDLE=";
				ss << &service << TEST;
				std::cout << ss.str() << "\n";

				SERVICE_STATUS serviceStatus;
				if (QueryServiceStatus(service, &serviceStatus)) {
					if (serviceStatus.dwCurrentState == SERVICE_STOPPED) {
						std::cout << service;
						DeleteService(service);
					}
				}
				CloseServiceHandle(service);
			}
			CloseServiceHandle(serviceControlManager);
		}
		CloseServiceHandle(serviceControlManager);
	}
	else {
		std::cout << "FAIL! Admin priv needed to install service";

	}
}


int service() {
	InstallService();
	return 0;
}