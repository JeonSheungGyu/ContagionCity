#include<Windows.h>
#include<stdio.h>

VOID *fiber[4];

VOID fiber0(VOID*){
	while (true){
		for (INT i = 0; i < 4; i++){
			SwitchToFiber(fiber[i]);
		}
	}
}
VOID CALLBACK fiber1(VOID*){
	while (true){
		printf("Fiber1\n");
		SwitchToFiber(fiber[0]);
	}
}
VOID CALLBACK fiber2(VOID*){
	while (true){
		printf("Fiber2\n");
		SwitchToFiber(fiber[0]);
	}
}
VOID CALLBACK fiber3(VOID*){
	while (true){
		printf("Fiber3\n");
		SwitchToFiber(fiber[0]);
	}
}

int main(VOID){
	fiber[0] = ConvertThreadToFiber(NULL);
	fiber[1] = CreateFiber(0, fiber1, NULL);
	fiber[2] = CreateFiber(0, fiber2, NULL);
	fiber[3] = CreateFiber(0, fiber3, NULL);
	fiber0(NULL);
}