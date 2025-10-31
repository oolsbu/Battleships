#include "udp_communication.h"


bool isMyTurn(){
    String message = receiveMessage();
    if(message == "myTurn"){
        return true;
    }
    else{
        return false;
    }
}