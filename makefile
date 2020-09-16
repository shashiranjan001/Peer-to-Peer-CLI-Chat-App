./chat : chat_server.c
		gcc -w chat_server.c -o chat

clean : 
		rm chat

run : 
		./chat