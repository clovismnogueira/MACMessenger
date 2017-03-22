buildTest: ChatClient.c ChatServer.c serverStressTest.c
	gcc -pthread ChatClient.c -o ChatClient.exe
	gcc -pthread ChatServer.c -lcrypt -o ChatServer.exe
	gcc serverStressTest.c -o serverStressTest.exe

clean:
	rm -f ChatClient.exe ChatServer.exe serverStressTest.exe

