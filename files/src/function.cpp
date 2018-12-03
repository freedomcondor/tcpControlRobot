#include <iostream>

#include "packet_control_interface.h"
#include "function.h"
#include "tcp_socket.h"

/* ----------------------------------*/
int tcpOpen();
int tcpGetSpeed(int16_t *L, int16_t *R);
int tcpGetSpeedorPara(int16_t *L, int16_t *R, float *_kp,  float *_ki, float *_kd);
int tcpSendSpeed(int16_t L, int16_t R);
int tcpSendSpeedWithDebug(int16_t L, int16_t R, int16_t Lerror, float Loutput);

int uartOpen();
int uartSendSpeed(int16_t L, int16_t R);
int uartGetSpeed(int16_t *L, int16_t *R);
int uartSendPIDParams(float kp, float ki, float kd);
int uartGetSpeedWithDebug(int16_t *L, int16_t *R, int16_t *LError, float *LOutput);

/* ----------------------------------*/
int16_t tL, tR;
int16_t cL, cR;
int16_t mL, mR;
float Kp,Ki,Kd;

/* ----------------------------------*/
int function_init()
{
	uartOpen();
	uartSendSpeed(0, 0);
	tcpOpen();
	return 0;
}

//#define datadebug

int function_step()
{
	int ret;
	ret = tcpGetSpeedorPara(&tL, &tR, &Kp, &Ki, &Kd);
				#ifdef datadebug
				printf("just get tcp : %d %d %f %f %f\n",tL, tR, Kp, Ki, Kd);
				#endif
	if ((tL != cL) || (tR != cR))
	{
		cL = tL; cR = tR;
		ret = uartSendSpeed(cL,cR);	
				#ifdef datadebug
				printf("just sent speed, %d %d, ret = %d\n", cL, cR, ret);
				#endif
	}
	if ( (Kp != 0) || (Ki != 0) || (Kd != 0) )
	{
		ret = uartSendPIDParams(Kp, Ki, Kd);	
				#ifdef datadebug
				printf("just sent PID %f %f %f ret = %d\n", Kp, Ki, Kd, ret);
				#endif
	}

	int16_t Lerror;
	float Loutput;
	//ret = uartGetSpeedWithDebug(&mL, &mR, &Lerror, &Loutput);
	ret = uartGetSpeed(&mL, &mR);
				#ifdef datadebug
				printf("just get uart: %d %d %d %f, ret = %d\n",mL, mR, Lerror, Loutput, ret);
				#endif
	ret = tcpSendSpeed(mL, mR);
				#ifdef datadebug
				printf("just sent tcp: %d %d %d %f, ret = \n",mL, mR, Lerror, Loutput, ret);
				#endif

	return 0;
}
int function_exit()
{
	printf("i am exit\n");
	uartSendSpeed(0, 0);
	return 0;
}

/*------------------------------------------------------*/
TCPSocket *tcpSocket;
int tcpOpen()
{
	// open a server on 8080
	tcpSocket = new TCPSocket("server",8080);
	printf("server listening...\n");
	tcpSocket->Open();
	printf("server connected!\n");
	return 0;
}

int tcpGetSpeed(int16_t *L, int16_t *R)
{
	char buffer[200];
	int len;
	int16_t data1, data2;
	tcpSocket->Read(buffer,&len,300);
	if (len == 4)
	{
		reinterpret_cast<int16_t&>(data1) = buffer[0]<<8 | buffer[1];
		reinterpret_cast<int16_t&>(data2) =buffer[2]<<8 | buffer[3];
		*L = data1;
		*R = data2;
	}
	else
		return -1;

	return 0;
}

int tcpGetSpeedorPara(int16_t *L, int16_t *R, float *_kp,  float *_ki, float *_kd)
{
	char buffer[200];
	int len;
	int16_t data1, data2;
	float data3, data4, data5;
	tcpSocket->Read(buffer,&len,300);
	if (len == 4)
	{
		reinterpret_cast<int16_t&>(data1) = buffer[0]<<8 | buffer[1];
		reinterpret_cast<int16_t&>(data2) =buffer[2]<<8 | buffer[3];
		*L = data1;
		*R = data2;
		*_kp = 0;
		*_ki = 0;
		*_kd = 0;
	}
	else if (len == 12)
	{
		uint32_t nData1, nData2, nData3, nData4, nData32;
		nData1 = 0xFF & buffer[0];
		nData2 = 0xFF & buffer[1];
		nData3 = 0xFF & buffer[2];
		nData4 = 0xFF & buffer[3];
		nData32 = nData1<<24 | nData2<<16 | nData3<<8 | nData4;
		data3 = *(reinterpret_cast<float*>(&nData32));
		nData1 = 0xFF & buffer[4];
		nData2 = 0xFF & buffer[5];
		nData3 = 0xFF & buffer[6];
		nData4 = 0xFF & buffer[7];
		nData32 = nData1<<24 | nData2<<16 | nData3<<8 | nData4;
		data4 = *(reinterpret_cast<float*>(&nData32));
		nData1 = 0xFF & buffer[8];
		nData2 = 0xFF & buffer[9];
		nData3 = 0xFF & buffer[10];
		nData4 = 0xFF & buffer[11];
		nData32 = nData1<<24 | nData2<<16 | nData3<<8 | nData4;
		data5 = *(reinterpret_cast<float*>(&nData32));

		*_kp = data3;
		*_ki = data4;
		*_kd = data5;
	}
}

int tcpSendSpeed(int16_t L, int16_t R)
{
	char buffer2[200] = {
		reinterpret_cast<char*>(&mL)[1],
		reinterpret_cast<char*>(&mL)[0],
		reinterpret_cast<char*>(&mR)[1],
		reinterpret_cast<char*>(&mR)[0],
	};

	tcpSocket->Write(buffer2,4);

	return 0;
}

int tcpSendSpeedWithDebug(int16_t L, int16_t R, int16_t Lerror, float Loutput)
{
	char buffer2[200] = {
		reinterpret_cast<char*>(&L)[1],
		reinterpret_cast<char*>(&L)[0],
		reinterpret_cast<char*>(&R)[1],
		reinterpret_cast<char*>(&R)[0],
		reinterpret_cast<char*>(&Lerror)[1],
		reinterpret_cast<char*>(&Lerror)[0],

		reinterpret_cast<char*>(&Loutput)[3],
		reinterpret_cast<char*>(&Loutput)[2],
		reinterpret_cast<char*>(&Loutput)[1],
		reinterpret_cast<char*>(&Loutput)[0],
	};

	tcpSocket->Write(buffer2,10);

	return 0;
}

/*------------------------------------------------------*/
CPacketControlInterface *ddsInterface, *pmInterface;
int uartOpen()
{
	pmInterface =
		new CPacketControlInterface("dds", "/dev/ttySC0", 57600);
	ddsInterface =
		new CPacketControlInterface("pm", "/dev/ttySC1", 57600);
	if (!ddsInterface->Open())
		{   printf("SC0 not open\n");   return -1;  }
	if (!pmInterface->Open())
		{   printf("SC1 not open\n");   return -1;  }
	printf("---Establish Connection with MCUs---------------------\n");

	printf("---Initialize Actuator---------------------\n");
	enum class EActuatorInputLimit : uint8_t {
		LAUTO = 0, L100 = 1, L150 = 2, L500 = 3, L900 = 4
	};

	/* Override actuator input limit to 100mA */
	pmInterface->SendPacket(
			CPacketControlInterface::CPacket::EType::SET_ACTUATOR_INPUT_LIMIT_OVERRIDE,
			static_cast<const uint8_t>(EActuatorInputLimit::L900));

	/* Enable the actuator power domain */
	pmInterface->SendPacket(
			CPacketControlInterface::CPacket::EType::SET_ACTUATOR_POWER_ENABLE, 
			true);
				      
	/* Power up the differential drive system */
	ddsInterface->SendPacket(CPacketControlInterface::CPacket::EType::SET_DDS_ENABLE, true);

	/* Initialize the differential drive system */
	uint8_t pnStopDriveSystemData[] = {0, 0, 0, 0};
	ddsInterface->SendPacket(
			CPacketControlInterface::CPacket::EType::SET_DDS_SPEED,
			pnStopDriveSystemData,
			sizeof(pnStopDriveSystemData));
	printf("initialize complete\n");

	return 0;
}
int uartSendSpeed(int16_t L, int16_t R)
{
	uint8_t uartData[] = {
		reinterpret_cast<uint8_t*>(&L)[1],
		reinterpret_cast<uint8_t*>(&L)[0],
		reinterpret_cast<uint8_t*>(&R)[1],
		reinterpret_cast<uint8_t*>(&R)[0],
	};
	ddsInterface->SendPacket(
		CPacketControlInterface::CPacket::EType::SET_DDS_SPEED,
		uartData,
		sizeof(uartData));

	return 0;
}
int uartGetSpeed(int16_t *L, int16_t *R)
{
	int16_t getL, getR;
	ddsInterface->SendPacket(CPacketControlInterface::CPacket::EType::GET_DDS_SPEED);
	if(ddsInterface->WaitForPacket(200, 3)) 
	{
		if(ddsInterface->GetState() == CPacketControlInterface::EState::RECV_COMMAND)
		{
			//printf("received command\n");
			const CPacketControlInterface::CPacket& cPacket = ddsInterface->GetPacket();
			if (cPacket.GetType() == CPacketControlInterface::CPacket::EType::GET_DDS_SPEED)
			{
				//printf("packettype: 0x%x\n",(int)cPacket.GetType());
				if(cPacket.GetDataLength() == 4) 
				{
					const uint8_t* punPacketData = cPacket.GetDataPointer();
					reinterpret_cast<int16_t&>(getL) = punPacketData[0]<<8 | punPacketData[1];
					reinterpret_cast<int16_t&>(getR) =punPacketData[2]<<8 | punPacketData[3];
					*L = getL; *R = getR;
					return 0;
				} 
			}
		}
	}
	return -1;
}

int uartGetSpeedWithDebug(int16_t *L, int16_t *R, int16_t *LError, float *LOutput)
{
	int16_t getL, getR, getLError;
	float getLOutput;
	ddsInterface->SendPacket(CPacketControlInterface::CPacket::EType::GET_DDS_SPEED);
	if(ddsInterface->WaitForPacket(200, 3)) 
	{
		if(ddsInterface->GetState() == CPacketControlInterface::EState::RECV_COMMAND)
		{
			//printf("received command\n");
			const CPacketControlInterface::CPacket& cPacket = ddsInterface->GetPacket();
			if (cPacket.GetType() == CPacketControlInterface::CPacket::EType::GET_DDS_SPEED)
			{
				//printf("packettype: 0x%x\n",(int)cPacket.GetType());
				if(cPacket.GetDataLength() == 10) 
				{
					const uint8_t* punPacketData = cPacket.GetDataPointer();
					reinterpret_cast<int16_t&>(getL) = punPacketData[0]<<8 | punPacketData[1];
					reinterpret_cast<int16_t&>(getR) =punPacketData[2]<<8 | punPacketData[3];
					reinterpret_cast<int16_t&>(getLError) = punPacketData[4]<<8 | punPacketData[5];
					reinterpret_cast<int32_t&>(getLOutput) = punPacketData[6]<<24 | 
															punPacketData[7]<<16 |
															punPacketData[8]<<8 |
															punPacketData[9];
					*L = getL; *R = getR;
					*LError = getLError;
					*LOutput = getLOutput;
					return 0;
				} 
			}
		}
	}
	return -1;
}

int uartSendPIDParams(float kp, float ki, float kd)
{
	uint8_t uartData[] = {
		reinterpret_cast<uint8_t*>(&kp)[3],
		reinterpret_cast<uint8_t*>(&kp)[2],
		reinterpret_cast<uint8_t*>(&kp)[1],
		reinterpret_cast<uint8_t*>(&kp)[0],

		reinterpret_cast<uint8_t*>(&ki)[3],
		reinterpret_cast<uint8_t*>(&ki)[2],
		reinterpret_cast<uint8_t*>(&ki)[1],
		reinterpret_cast<uint8_t*>(&ki)[0],

		reinterpret_cast<uint8_t*>(&kd)[3],
		reinterpret_cast<uint8_t*>(&kd)[2],
		reinterpret_cast<uint8_t*>(&kd)[1],
		reinterpret_cast<uint8_t*>(&kd)[0],
	};
	ddsInterface->SendPacket(
		CPacketControlInterface::CPacket::EType::SET_DDS_PARAMS,
		uartData,
		sizeof(uartData));

	return 0;
}

