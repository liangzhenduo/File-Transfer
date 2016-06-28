#Socket File Transfer
##Usage
	
	./server <PORT>
	
	./client <IP> <PORT>
	>>> send <SRC_FILE> <DST_FILE>
	>>> exit
	
##Example

	./server 23333
	
	./client 127.0.0.1 23333
	>>> send README.md COPY.md
	>>> exit