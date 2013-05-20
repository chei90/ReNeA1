/*
 * Identifyers.h
 *
 *  Created on: 14.05.2013
 *      Author: christoph
 */


#ifndef IDENTIFYERS_H_
#define IDENTIFYERS_H_

	enum magicNumbers{
		//Client connection request
		CL_CON_REQ = 1,
		//Server connection reply
		SV_CON_REP,
		//Server connection msg to all clients
		SV_CON_AMSG,
		//Message from client
		CL_MSG ,
		//Forwarding of client message
		SV_AMSG,
		//Client disconnect request
		CL_DISC_REQ,
		//Server disconnect reply
		SV_DISC_REP,
		//Server disconnect notification to other clients
		SV_DISC_AMSG,
		//Server ping request
		SV_PING_REQ,
		//Client ping reply
		CL_PING_REP,
		//Server Message
		SV_MSG,
	};

	uint8_t conreq = 1;

#endif
