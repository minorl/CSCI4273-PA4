 #include "message.h"
using namespace std;
Message::Message(){
	msglen = 0;
}

Message::Message(char* msg, size_t len){
	struct messageNode msgstruct;
	msgstruct.msgptr = msg;
	msgstruct.len = len;
	msglen = len;
	messageList.push_front(msgstruct);
}

Message::~Message( ){
	// delete msg_content;
}

void Message::msgAddHdr(char *hdr, size_t length){
	struct messageNode msgstruct;
	msgstruct.msgptr = hdr;
	msgstruct.len = length;
	messageList.push_front(msgstruct);
	msglen += length;
}

char* Message::msgStripHdr(size_t len){
	char* stripped_content;
	if ((msglen < len) || (len == 0)) return NULL;
	else if (len == msglen){
		//return full message
		if(messageList.size() == 1){
			stripped_content = messageList.front().msgptr;
			messageList.front().msgptr = NULL;
			messageList.front().len = 0;
			msglen = 0;
		}
		else{
			stripped_content = new char[len];
			msgFlat(stripped_content);
			messageList.front().msgptr = NULL;
			messageList.front().len = 0;
			msglen = 0;		
		}
	}
	else{
		//len < msglen
		stripped_content = new char[len];
		size_t idx = len;
		size_t copied = 0;
		std::list<struct messageNode>::iterator tar = findN(idx);
		while(messageList.begin() != tar){
			memcpy(stripped_content + copied, (*messageList.begin()).msgptr, (*messageList.begin()).len);
			copied += (*messageList.begin()).len;
			messageList.pop_front();
		}
		// on the node with overlap
		if(idx != 0){
			memcpy(stripped_content + copied, (*messageList.begin()).msgptr, idx);
			copied += idx;
			(*messageList.begin()).msgptr += idx;
			(*messageList.begin()).len -= idx;
		}
		if(copied != len){
			printf("strip header: %zu copied, expected %zu.\n", copied, len);
		}
		msglen -= copied;
	}

	return stripped_content;
}
std::list<struct messageNode>::iterator Message::findN(size_t &n){
	size_t numFound = 0;
	size_t prevFound = 0;
	std::list<struct messageNode>::iterator it = 
		messageList.begin();
	while(numFound < n){
		prevFound = numFound;
		numFound += (*it).len;
		it++;
	}
	it--;
	//this should be the index of nth character of message
	n -= prevFound;
	return it;
}

int Message::msgSplit(Message& secondMsg, size_t len)
{
	if ((len == 0) || (len > msglen)) return 0;
	//truncate this message at len, rem in second
	size_t idx = len;
	// it pting at overlap node
	std::list<struct messageNode>::iterator it = findN(idx);
	//overlap node now in second message
	secondMsg.messageList.splice(secondMsg.messageList.begin(),
		messageList, it, messageList.end());
	// printf("Second message list len: %d, first message list len %d. Overlap at idx: %zu\n",
		// secondMsg.messageList.size(), messageList.size(), idx);
	if(idx != 0){
		//duplicate overlap string, can't dupe node pts to same str
		struct messageNode firstPart;
		firstPart.msgptr = new char[idx];
		firstPart.len = idx;
		memcpy(firstPart.msgptr, 
			secondMsg.messageList.front().msgptr, idx);
		messageList.push_back(firstPart);

		secondMsg.msglen = msglen - len;
		msglen = len;

		secondMsg.messageList.front().msgptr += idx;
		secondMsg.messageList.front().len -= idx;

	}


	return 1;
}

void Message::msgJoin(Message& secondMsg)
{
	//combine lists and empty second message
	messageList.splice(messageList.end(), secondMsg.messageList);
	msglen += secondMsg.msglen;
	secondMsg.msglen = 0;

}

size_t Message::msgLen( )
{
	return msglen;
}

void Message::msgFlat(char *buffer)
{
	//Assume that sufficient memory has been allocated in buffer

	size_t copied = 0;

	for(std::list<struct messageNode>::iterator it = messageList.begin(); 
		it != messageList.end(); it++){
		memcpy(buffer + copied, (*it).msgptr, (*it).len);
		copied += (*it).len;
	}
}

