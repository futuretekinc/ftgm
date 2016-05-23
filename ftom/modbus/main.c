#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include "../ftom.h"
#include "../ftom_node_modbus_client.h"

int main(int argc, char **argv) 
{
		void *handle;
		FTOM_NODE_MBC_DESCRIPT_PTR	pClient;
		char *error;

		handle = dlopen ("libmodbus.so", RTLD_LAZY);

		handle = dlopen ("./ftom_node_modbus_test.so", RTLD_LAZY);
		if (!handle) {
				fputs (dlerror(), stderr);
				exit(1);
		}

		pClient = dlsym(handle, "pMODBUSClient");
		if ((error = dlerror()) != NULL)  {
				fputs(error, stderr);
				exit(1);
		}

		fprintf(stdout, "MODEL : %s\n", pClient->pModel);
		dlclose(handle);

		return	0;
}
