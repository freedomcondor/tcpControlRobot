#include<math.h>
#include"function.h"
#include"packet_control_interface.h"

#include <iostream>

#define pi 3.1415926

#define Max_plot 100000

///////////////////  function definations /////////////////
//
//
///////////////////  dds  /////////////////////////
CPacketControlInterface *ddsInterface, *pmInterface;

int16_t currentLeftSpeed, currentRightSpeed;
int16_t targetLeftSpeed, targetRightSpeed;
int16_t fbLeftSpeed, fbRightSpeed;

unsigned int uptime = 0;

///////////////////  functions  ///////////////////////////
int function_exit()
{
	uint8_t pnStopDriveSystemData[] = { 0,0};

	ddsInterface->SendPacket(
			CPacketControlInterface::CPacket::EType::SET_DDS_SPEED_LEFT,
			pnStopDriveSystemData,
			sizeof(pnStopDriveSystemData));
	ddsInterface->SendPacket(
			CPacketControlInterface::CPacket::EType::SET_DDS_SPEED_RIGHT,
			pnStopDriveSystemData,
			sizeof(pnStopDriveSystemData));
}

int function_init()
{
	ddsInterface = 
		      new CPacketControlInterface("dds", "/dev/ttySC0", 57600);

	pmInterface = 
		      new CPacketControlInterface("pm", "/dev/ttySC1", 57600);

	printf("---Establish Connection with MCUs---------------------\n");

	if (!ddsInterface->Open())
	{	printf("SC0 not open\n");	return -1;	}

	if (!pmInterface->Open())
	{	printf("SC1 not open\n");	return -1;	}
	printf("Both interfaces are open\n");

	// get uptime and know which is which
	ddsInterface->SendPacket(CPacketControlInterface::CPacket::EType::GET_UPTIME);
	printf("ID check:\n");
	while(1)
	{
		ddsInterface->ProcessInput();

		if(ddsInterface->GetState() == CPacketControlInterface::EState::RECV_COMMAND)
		{
			const CPacketControlInterface::CPacket& cPacket = ddsInterface->GetPacket();
			if (cPacket.GetType() == CPacketControlInterface::CPacket::EType::GET_UPTIME)
			{
				if(cPacket.GetDataLength() == 4) 
				{
					const uint8_t* punPacketData = cPacket.GetDataPointer();
					uint32_t unUptime = 
						(punPacketData[0] << 24) |
						(punPacketData[1] << 16) |
						(punPacketData[2] << 8)  |
						(punPacketData[3] << 0); 
					printf("dds uptime: %u\n",unUptime);
					if (unUptime != 0)
					{
						// means reverse, exchange pm and dds
						printf("ddreinterpret_cast<uint16_ts get non-0 uptime, it should be pm\n");
						CPacketControlInterface* tempInterface;
						tempInterface = pmInterface;
						pmInterface = ddsInterface;
						ddsInterface = tempInterface;
					}
				} 
				break;
			}
		}
	}
	printf("ID check complete:\n");

	// Read Battery
	pmInterface->SendPacket(CPacketControlInterface::CPacket::EType::GET_BATT_LVL);
	if(pmInterface->WaitForPacket(1000, 5)) 
	//while (1)
	{
		const CPacketControlInterface::CPacket& cPacket = pmInterface->GetPacket();
		if(cPacket.GetType() == CPacketControlInterface::CPacket::EType::GET_BATT_LVL &&
				cPacket.GetDataLength() == 2) 
		{
			std::cout << "System battery: "
		              << 17u * cPacket.GetDataPointer()[0]
					  << "mV" << std::endl;
			std::cout << "Actuator battery: "
					  << 17u * cPacket.GetDataPointer()[1]
					  << "mV" << std::endl;
			//break;
		}
	}
	///*
	else 
	{
		std::cout << "Warning: Could not read the system/actuator battery levels" << std::endl;
	}
	//*/

	// Speed
	printf("---Initialize Actuator---------------------\n");
	enum class EActuatorInputLimit : uint8_t {
		LAUTO = 0, L100 = 1, L150 = 2, L500 = 3, L900 = 4
	};

	/* Override actuator input limit to 100mA */
	pmInterface->SendPacket(
			CPacketControlInterface::CPacket::EType::SET_ACTUATOR_INPUT_LIMIT_OVERRIDE,
			static_cast<const uint8_t>(EActuatorInputLimit::L100));

	/* Enable the actuator power domain */
	pmInterface->SendPacket(
			CPacketControlInterface::CPacket::EType::SET_ACTUATOR_POWER_ENABLE, 
			true);
				      
	/* Power up the differential drive system */
	ddsInterface->SendPacket(CPacketControlInterface::CPacket::EType::SET_DDS_ENABLE, true);

	/* Initialize the differential drive system */
	uint8_t pnStopDriveSystemData[] = {0, 0};
	ddsInterface->SendPacket(
			CPacketControlInterface::CPacket::EType::SET_DDS_SPEED_LEFT,
			pnStopDriveSystemData,
			sizeof(pnStopDriveSystemData));
	ddsInterface->SendPacket(
			CPacketControlInterface::CPacket::EType::SET_DDS_SPEED_RIGHT,
			pnStopDriveSystemData,
			sizeof(pnStopDriveSystemData));
	printf("initialize complete\n");

	// prepare speed
	currentRightSpeed = 0;
	currentLeftSpeed = 0;

	return 0;
}

int function_step()
{
	if ((targetLeftSpeed != currentLeftSpeed) || (targetRightSpeed != currentRightSpeed))
	{
		currentLeftSpeed = targetLeftSpeed;
		currentRightSpeed = targetRightSpeed;

		uint8_t pnStopDriveSystemDataLeft[] = {
			reinterpret_cast<uint8_t*>(&currentLeftSpeed)[1],
			reinterpret_cast<uint8_t*>(&currentLeftSpeed)[0],
		};
		ddsInterface->SendPacket(
			CPacketControlInterface::CPacket::EType::SET_DDS_SPEED_LEFT,
			pnStopDriveSystemDataLeft,
			sizeof(pnStopDriveSystemDataLeft));

		uint8_t pnStopDriveSystemDataRight[] = {
			reinterpret_cast<uint8_t*>(&currentRightSpeed)[1],
			reinterpret_cast<uint8_t*>(&currentRightSpeed)[0],
		};
		ddsInterface->SendPacket(
			CPacketControlInterface::CPacket::EType::SET_DDS_SPEED_RIGHT,
			pnStopDriveSystemDataRight,
			sizeof(pnStopDriveSystemDataRight));
	}

	printf("target speed : %d %d\n",currentLeftSpeed,currentRightSpeed);

	printf("Asking speed\n");
	ddsInterface->SendPacket(CPacketControlInterface::CPacket::EType::GET_DDS_SPEED);
	if(ddsInterface->WaitForPacket(200, 3)) 
	{
		if(ddsInterface->GetState() == CPacketControlInterface::EState::RECV_COMMAND)
		{
			printf("received command\n");
			const CPacketControlInterface::CPacket& cPacket = ddsInterface->GetPacket();
			if (cPacket.GetType() == CPacketControlInterface::CPacket::EType::GET_DDS_SPEED)
			{
				printf("packettype: 0x%x\n",(int)cPacket.GetType());
				if(cPacket.GetDataLength() == 4) 
				{
					const uint8_t* punPacketData = cPacket.GetDataPointer();
					reinterpret_cast<int16_t&>(fbLeftSpeed) = punPacketData[0]<<8 | punPacketData[1];
					reinterpret_cast<int16_t&>(fbRightSpeed) =punPacketData[2]<<8 | punPacketData[3];
					printf("measure speed : %d %d\n",fbLeftSpeed,fbRightSpeed);
				} 
			}
		}
	}

	return 0;
}
