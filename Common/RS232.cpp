#include <RS232.h>

#include <serial.h>

RS232::RS232(){
	port = NULL;
}

RS232::~RS232(){
	close();
}

bool RS232::open(int baudRate, std::string device, bool hwFlowControl, bool readBlocking){
	close();
	SerialBaudType baudFlag;
	int res = getBaudFlag(baudRate, &baudFlag);
	if(res==0){
		port = new SerialPort();
		res = openTerm(device.c_str(), baudFlag, port, hwFlowControl, readBlocking);
		if(res!=0){delete port; port = NULL;}
	}
	return res==0;
}

bool RS232::send(const char* buf, uint32_t len){
	if(port){
		int res = sendWholeTermData(port, buf, len);
		return res==(int)len;
	}
	return false;
}

int32_t RS232::recv(char* buf, uint32_t buflen){
	if(port){
		return recvTermData(port, buf, buflen);
	}
	return -1;
}

void RS232::close(){
	if(port){
		closeTerm(port);
		delete port;
		port = NULL;
	}
}

