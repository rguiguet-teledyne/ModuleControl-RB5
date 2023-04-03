#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fstream>
#include "i2c.hpp"
#include "i2c.cpp"
#include "ModuleControl_v230.hpp"
#include "ModuleControl_v230.cpp"

void printHelp();
int read_sensor_feedback(ModuleCtrl *moduleCtrl);
int start_MultiFocus_3dist(ModuleCtrl *moduleCtrl, int PdaRegValue1, int PdaRegValue2, int PdaRegValue3, float speed );

/************************************************
*Main fuction
************************************************/

int main(){
	//app variable
	int isRunning = 1;
	char input, stop;
	int err =0;

	ModuleCtrl *moduleCtrl;
	float tint=0;
	float again=0;
	float dgain=0;
	int regAddress=0;
	int regValue=0;
	int state=0;
	int dist1=0;
	int dist2=0;
	int dist3=0;
        int PdaRegValue;
        double PdaVoltageValue;
	double localTemp, remoteTemp;
	int tmpMode=0;

	moduleCtrl = new ModuleCtrl();

	moduleCtrl->ModuleControlInit(); // init ic2
	//system("v4l2-ctl -d /dev/video0 --set-fmt-video=width=1920,height=1080,pixelformat='GREY' --set-ctrl sensor_mode=2");
	printHelp();
	while (isRunning){
		printf(">");
		scanf(" %c",&input);
		switch (input)
		{
			//General functions
			case 'h': //print help
				printHelp();
				break;
			case 'q': //quit menu
				isRunning=0;
				break;
			case 'c': //clear terminal
				system("clear");
				break;
			//Application specific
			case 'r': //read register
				printf("Register address (hexa): ");
				scanf("%x",&regAddress);
        			if(moduleCtrl->readReg(regAddress, &regValue)==0) printf("@0x%02x -> 0x%04x (%d)\n", regAddress, regValue, regValue);
				break;
			case 'w': //write register
				printf("Register address (hexa) : ");
				scanf("%x",&regAddress);
				printf("Register value (hexa) : ");
				scanf("%x",&regValue);
        			moduleCtrl->writeReg(regAddress, regValue);
				break;
			case 't': //setup tint
				printf("Exposition time in ms : ");
				scanf("%f",&tint);
           			moduleCtrl->setTint(tint);
				break;
			case 'a': //setup analog gain
				printf("Set Analog gain\nAuthorized values : x1, x1.2, x1.5, x2, x3, x4, x6, x7, x8, x9, x10, x12, x16\nValue:");
				scanf("%f",&again);
            			moduleCtrl->setAnalogGain(again);
				break;
			case 'g': //setup digital gain
				printf("Set digital gain (from x0.004 to x16)\nValue: ");
				scanf("%f",&dgain);
				moduleCtrl->setDigitalGain(dgain);
				break;
			case 's': //Read sensor state
				err = moduleCtrl->read_sensor_state(&state);
				if(err < 0)
				{
					printf("read sensor state error=%d\n",err);
				}
				else
				{
					printf("reg_state=0x%04x (%d)",state,state);
					state=state/256;
					printf("\nglobal_state=0x%x (%d)",state,state);
					if (state==0x100) printf(" => SDTBY \n");
					else if (state==0x02) printf(" => WAKE_UP_IF \n");
					else if (state==0x04) printf(" => IDLE_IF \n");
					else if (state==0x08) printf(" => WAKE_UP_ALL \n");
					else if (state==0x10) printf(" => IDLE \n");
					else if (state==0x20) printf(" => ACQUISITION \n");
					else if (state==0x40) printf(" => STOP_STD \n");
					else if (state==0x80) printf(" => WAIT_END_CHAIN \n");
					else printf(" \n");	
				}
				break;
			case '1': //write PDA50 DAC value
				printf("Enter PDA50 TLENS register value (Range:-91 to 879): ");
				scanf("%d",&PdaRegValue); 
				moduleCtrl->write_VdacPda(PdaRegValue);
				break;
			case '2': //read PDA50 DAC value
            			moduleCtrl->read_VdacPda(&PdaRegValue, &PdaVoltageValue);
				printf("PDA50 TLens: VdacPdaValue=%d, PdaVoltageValue=%.2fV\n", PdaRegValue, PdaVoltageValue);
				break;
/*			case '3':
				err = dumpPda50Reg(devicepda, bus);
				break;

			case 'd':
				err = dumpReg(device, bus);
				break;


			case 'e':
				err = writeRegSafe(device, bus);
				break;
*/			case 'f': //Read feedback registers
				read_sensor_feedback(moduleCtrl);
				break;
			case 'm':
				printf("Enter distance 1: ");
				scanf("%d",&dist1);
				printf("Enter distance 2: ");
				scanf("%d",&dist2); 
				printf("Enter distance 3: ");
				scanf("%d",&dist3); 
				printf("Multifocus start with vdac=[%d, %d, %d]\n=>press CTRL+C to stop\n",dist1,dist2,dist3);
				sleep(2);
				while(1) start_MultiFocus_3dist(moduleCtrl, dist1, dist2, dist3, 1);
			case 'z':
				err=moduleCtrl->read_Temp(&localTemp,&remoteTemp,tmpMode);
				if(err<0)
				{
					printf("Read temp error=%d\n",err);
				}
				else
				{
					printf("LocalTemp=%0.2f°C\n",localTemp);
					printf("RemoteTemp=%0.2f°C\n",remoteTemp);
				}
				break;
			case 'x':
				err=moduleCtrl->get_TempMode(&tmpMode);
				if(err<0)
				{
					printf("Read temp error=%d\n",err);
				}
				else
				{
					printf("TempMode=%d ",tmpMode);
					if(tmpMode==0) printf("=> standard range (0:127°C)\n");
					else if(tmpMode==1) printf("=> extended range (-55:+150°C)\n");
					else printf("=> undefined mode\n");
				}
				break;
			case 'y':
				printf("Enter temperature range mode:\n\t0: standard range (0:127°C)\n\t1: extended range (-55:+150°C)\n");
				scanf("%d",&tmpMode);
				err=moduleCtrl->set_TempMode(tmpMode);
				if(err<0)
				{
					printf("Write temp error=%d\n",err);
				}
				else
				{
					//printf("TempMode=%d \n",tmpMode);
				}
				break;

			default:
				printf("Wrong input!\n\n");
				printHelp();
		}
		//if (err != 0) isRunning = 0;
	}

	delete moduleCtrl;
	return 0;
}

/************************************************
*Print help
************************************************/
void printHelp(){
	system("clear");
	printf("\nTELEDYNE-E2V OPTIMOM 2M Module running with Nvidia Jetson Nano\n");
	printf("\nMODULE CONTROL\n");
	printf("Tools menu:\n");
	printf("\tt:\tchange exposition time\n");
	printf("\ta:\tchange analog gain\n");
	printf("\tg:\tchange digital gain\n");
	printf("\tr:\tread a register\n");
	printf("\tw:\twrite in a register\n");
	printf("\ts:\tread sensor state\n");
	printf("\tf:\tread feedback registers\n");
	//printf("\td:\tdump register values\n");
	printf("\t1:\tlens focus: write VDac register from -91 to 879\n");
	printf("\t2:\tlens focus: read VDac register\n");
	printf("\tm:\tstart Multifocus 3 distances\n");
	printf("\tx:\tget temp mode\n");
	printf("\ty:\tset temp mode\n");
	printf("\tz:\tread temperature\n");
	printf("\tc:\tclear window\n");
	printf("\th:\tprint this help\n");
	printf("\tq:\tquit\n");
}

int read_sensor_feedback(ModuleCtrl *moduleCtrl)
{

	// PARAMETERS
	int Address;
	int Value;

	//READ CHIP ID
	Address = 0x007F;
	Value = 0;
        moduleCtrl->readReg(Address, &Value);	
	printf("@0x%04x SensorChipID=0x%04x", Address, Value);
	if (Value>0x8000) printf(" => TOPAZ 2M \n");
	else if (Value>0x6000) printf("\n");
	else if (Value>0x5200) printf(" => SNAPPY 5M \n");
	else if (Value>0x5100) printf(" => SNAPPY 2M \n");
	else if (Value>0x4200) printf(" => EMERALD 10M/8.9M \n");
	else if (Value>0x4100) printf(" => EMERALD 16M/12M \n");
	else printf(" \n");	

	//READ SENSOR STATE
	Address = 0x0053;
	Value = 0;
	moduleCtrl->readReg(Address, &Value);
	printf("reg_state=0x%04x (%d)",Value,Value);
	Value=Value/256;
	printf("\nglobal_state=0x%x (%d)",Value,Value);
	if (Value==0x100) printf(" => SDTBY \n");
	else if (Value==0x02) printf(" => WAKE_UP_IF \n");
	else if (Value==0x04) printf(" => IDLE_IF \n");
	else if (Value==0x08) printf(" => WAKE_UP_ALL \n");
	else if (Value==0x10) printf(" => IDLE \n");
	else if (Value==0x20) printf(" => ACQUISITION \n");
	else if (Value==0x40) printf(" => STOP_STD \n");
	else if (Value==0x80) printf(" => WAIT_END_CHAIN \n");
	else printf(" \n");		
	

	//Read line length fb
	Address = 0x0052;
	Value = 0;
	moduleCtrl->readReg(Address, &Value);
	printf("@0x%04x fb_line_length=0x%04x / %d\n", Address, Value, Value);
	//Read error fb
	Address = 0x0054;
	Value = 0;
	moduleCtrl->readReg(Address, &Value);
	printf("@0x%04x fb_error=0x%04x / %d", Address, Value, Value);
	if (Value==0x0001) printf(" => trigger error \n");
	else if (Value==0x0002) printf(" => not(fb_fuse_supply_ena_pi) \n");
	else if (Value==0x0004) printf(" => frame period error in video mode \n");
	else if (Value==0x0008) printf(" => error_ll_overflow \n");
	else if (Value==0x0010) printf(" => error_ll_read \n");
	else if (Value==0x0020) printf(" => error_ll_conv \n");
	else if (Value==0x0040) printf(" => error_ll_mipi \n");
	else printf(" \n");		

	//Read tint fb
	Address = 0x0055;
	Value = 0;
	moduleCtrl->readReg(Address, &Value);
	printf("@0x%04x fb_reg_tint_ll=0x%04x / %d\n", Address, Value, Value);
	//Read spy_tint_1_ll fb
	Address = 0x0057;
	Value = 0;
	moduleCtrl->readReg(Address, &Value);
	printf("@0x%04x fb_reg_spy_tint_1_ll=0x%04x / %d\n", Address, Value, Value);
	//Read spy_tint_1_ck fb
	Address = 0x0058;
	Value = 0;
	moduleCtrl->readReg(Address, &Value);
	printf("@0x%04x fb_reg_spy_tint_1_ck=0x%04x / %d\n", Address, Value, Value);
	//Read frame_period fb
	Address = 0x0056;
	Value = 0;
	moduleCtrl->readReg(Address, &Value);
	printf("@0x%04x fb_reg_frame_period=0x%04x / %d\n", Address, Value, Value);

	return 0;
}



int start_MultiFocus_3dist(ModuleCtrl *moduleCtrl, int PdaRegValue1, int PdaRegValue2, int PdaRegValue3, float speed )
{
	printf("vdac=");
	moduleCtrl->write_VdacPda(PdaRegValue1);
	printf("%d\n",PdaRegValue1);
	sleep(speed);
	printf("vdac=");
	moduleCtrl->write_VdacPda(PdaRegValue2);
	printf("%d\n",PdaRegValue2);
	sleep(speed);
	printf("vdac=");
	moduleCtrl->write_VdacPda(PdaRegValue3);
	printf("%d\n",PdaRegValue3);
	sleep(speed);
	return 0;
}

/*


/************************************************
*Dump
************************************************
int dumpReg(I2CDevice device, int bus){
	
	int b,page_b;
	unsigned char buffer[2];
	ssize_t size = sizeof(buffer);
	int reg =0;
	int j = 0;
	int line = 0;
	memset(buffer,0,size);

	device.page_bytes = 256;
	
	printf("          0    1    2    3    4    5    6    7");
	printf("\n%02x:    ", line);
	
	while (reg<=0xff){
		if((i2c_ioctl_read(&device,reg,buffer,size)) != size){
			fprintf(stderr, "Can't read %x reg!\n", reg);
			i2c_close(bus);
			exit(-3);
		}

		if (j==8){
			line++;
			
			if ((line%2)) printf("\n%02x:    ", (reg&0xf0)+0x08);
			else printf("\n%02x:    ", (reg&0xf0));
			j = 0;
		}

		printf("%02x%02x ", *buffer, *(buffer+1));
		memset(buffer,0,size);
		reg++;
		j++;
	}
	printf("\n");

	return 0;
}

int dumpPda50Reg(I2CDevice device, int bus){
	
	int b,page_b;
	unsigned char buffer[1];
	ssize_t size = sizeof(buffer);
	int reg =0;
	int j = 0;
	int line = 0;
	memset(buffer,0,size);

	device.page_bytes = 8;
	
	printf("        0  1  2  3  4  5  6  7");
	printf("\n%02x:    ", line);
	
	while (reg<=0x7){
		if((i2c_ioctl_read(&device,reg,buffer,size)) != size){
			fprintf(stderr, "Can't read %x reg!\n", reg);
			i2c_close(bus);
			exit(-3);
		}
		printf("%02x ", *buffer);
		memset(buffer,0,size);
		reg++;
		j++;
	}
	printf("\n");

	return 0;
}


*/
