#include <stdio.h>
#include <termios.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdint.h>
#include "packet.h"
#include "serial.h"
#include "serialize.h"
#include "constants.h"

#define PORT_NAME			"/dev/ttyACM0"
#define BAUD_RATE			B9600

static char prevch = '0'; 		// to prevent flooding of commands
bool manual = true;
bool opened = false;
int exitFlag=0;
sem_t _xmitSema;

char autotrans() { // non blocking keyboard input, removes the need for pressing enter
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0)
        perror("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
        perror("tcsetattr ~ICANON");
    return (buf);
}

void handleError(TResult error)
{
	switch(error)
	{
		case PACKET_BAD:
			printf("ERROR: Bad Magic Number\n");
			break;

		case PACKET_CHECKSUM_BAD:
			printf("ERROR: Bad checksum\n");
			break;

		default:
			printf("ERROR: UNKNOWN ERROR\n");
	}
}

void handleStatus(TPacket *packet)
{
	printf("\n ------- ALEX STATUS REPORT ------- \n\n");
	printf("Left Forward Ticks:\t\t%d\n", packet->params[0]);
	printf("Right Forward Ticks:\t\t%d\n", packet->params[1]);
	printf("Left Reverse Ticks:\t\t%d\n", packet->params[2]);
	printf("Right Reverse Ticks:\t\t%d\n", packet->params[3]);
	printf("Left Forward Ticks Turns:\t%d\n", packet->params[4]);
	printf("Right Forward Ticks Turns:\t%d\n", packet->params[5]);
	printf("Left Reverse Ticks Turns:\t%d\n", packet->params[6]);
	printf("Right Reverse Ticks Turns:\t%d\n", packet->params[7]);
	printf("Forward Distance:\t\t%d\n", packet->params[8]);
	printf("Reverse Distance:\t\t%d\n", packet->params[9]);
	printf("\n---------------------------------------\n\n");
}

void handleResponse(TPacket *packet)
{
	// The response code is stored in command
	switch(packet->command)
	{
		case RESP_OK:
			if (!opened){
				printf("CLOSE\n");
			}
			else{
				printf("OPEN\n");
			}
			break;
		case RESP_STATUS:
			handleStatus(packet);
		break;

		default:
			printf("Arduino is confused\n");
	}
}

void handleErrorResponse(TPacket *packet)
{
	// The error code is returned in command
	switch(packet->command)
	{
		case RESP_BAD_PACKET:
			printf("Arduino received bad magic number\n");
		break;

		case RESP_BAD_CHECKSUM:
			printf("Arduino received bad checksum\n");
		break;

		case RESP_BAD_COMMAND:
			printf("Arduino received bad command\n");
		break;

		case RESP_BAD_RESPONSE:
			printf("Arduino received unexpected response\n");
		break;

		default:
			printf("Arduino reports a weird error\n");
	}
}
/*
void handleMessage(TPacket *packet)
{
	printf("Message from Alex: %s\n", packet->data);
}
*/
void handlePacket(TPacket *packet)
{
	switch(packet->packetType)
	{
		case PACKET_TYPE_COMMAND:
				// Only we send command packets, so ignore
			break;

		case PACKET_TYPE_RESPONSE:
				handleResponse(packet);
			break;

		case PACKET_TYPE_ERROR:
				handleErrorResponse(packet);
			break;
/*
		case PACKET_TYPE_MESSAGE:
				handleMessage(packet);
			break;
*/
	}
}

void sendPacket(TPacket *packet)
{
	char buffer[PACKET_SIZE];
	int len = serialize(buffer, packet, sizeof(TPacket));

	serialWrite(buffer, len);
}

void *receiveThread(void *p)
{
	char buffer[PACKET_SIZE];
	int len;
	TPacket packet;
	TResult result;
	int counter=0;

	while(1)
	{
		len = serialRead(buffer);
		counter+=len;
		if(len > 0)
		{
			result = deserialize(buffer, len, &packet);

			if(result == PACKET_OK)
			{
				counter=0;
				handlePacket(&packet);
			}
			else 
				if(result != PACKET_INCOMPLETE)
				{
					printf("PACKET ERROR\n");
					handleError(result);
				}
		}
	}
}

void flushInput()
{
	char c;

	while((c = getchar()) != '\n' && c != EOF);
}

void getParams(TPacket *commandPacket)
{
	if (manual){
		printf("Enter distance/angle in cm/degrees (e.g. 50) and power in %% (e.g. 75) separated by space.\n");
		printf("E.g. 50 75 means go at 50 cm at 75%% power for forward/backward, or 50 degrees left or right turn at 75%%  power\n");
	}
	else{
		printf("Enter straightline speed for fwd n reverse THEN, enter traverse speed\n");
	}
	scanf("%d %d", &commandPacket->params[0], &commandPacket->params[1]);
	printf("sent %d %d\n", commandPacket->params[0], commandPacket->params[1]);
	flushInput();
}

void getservoparams(TPacket *commandPacket)
{
	printf("RAW angle for servo in degrees 0 - 180 \n");
	scanf("%d", &commandPacket->params[0]);
	flushInput();
}

void sendCommand(char command)
{
	TPacket commandPacket;

	commandPacket.packetType = PACKET_TYPE_COMMAND;

	switch(command)
	{
		case 'w':
		case 'W':
			if (manual){
				getParams(&commandPacket);
			}
			commandPacket.command = COMMAND_FORWARD;
			sendPacket(&commandPacket);
			break;

		case 's':
		case 'S':
			if (manual){
				getParams(&commandPacket);
			}
			commandPacket.command = COMMAND_REVERSE;
			sendPacket(&commandPacket);
			break;

		case 'a':
		case 'A':
			if (manual){
				getParams(&commandPacket);
			}
			commandPacket.command = COMMAND_TURN_LEFT;
			sendPacket(&commandPacket);
			break;

		case 'd':
		case 'D':
			if (manual){
				getParams(&commandPacket);
			}
			commandPacket.command = COMMAND_TURN_RIGHT;
			sendPacket(&commandPacket);
			break;

		case ' ':
			commandPacket.command = COMMAND_STOP;
			sendPacket(&commandPacket);
			break;

		case 'c':
		case 'C':
			commandPacket.command = COMMAND_CLEAR_STATS;
			commandPacket.params[0] = 0;
			sendPacket(&commandPacket);
			break;

		case 'g':
		case 'G':
			commandPacket.command = COMMAND_GET_STATS;
			sendPacket(&commandPacket);
			break;
		
		case 'O':
		case 'o':
			//getservoparams(&commandPacket); // get RAW servo angle
			//printf("servo angle set to %d\n", commandPacket.params[0]); // print set servo angle
			commandPacket.command = COMMAND_SERVO;
			sendPacket(&commandPacket);
			opened =!opened;
			break;
		
		case 'm':
		case 'M': // toggles between manual and auto
			manual = !manual;
			if (manual){
				printf("set to manual\n");
			}
			else{
				printf("set to automatic\n");
				getParams(&commandPacket); // param[0] = straight line speed, fwd n rev & param[1] = traverse speed, cw and ccw
			}
			commandPacket.command = COMMAND_MANUAL;
			sendPacket(&commandPacket);
			break;
		
		case 'p': // MEDPAK
		case 'P':
			commandPacket.command = COMMAND_MEDPAK;
			sendPacket(&commandPacket);
			break;

		case 'q':
		case 'Q':
			exitFlag=1;
			break;

		default:
			printf("Bad command. WASD for movement SPACE=stop, m=toggle trans, q=exit, o=servo, c=medpak\n");

	}
}

int main()
{
	// Connect to the Arduino
	startSerial(PORT_NAME, BAUD_RATE, 8, 'N', 1, 5);

	// Sleep for two seconds
	printf("WAITING TWO SECONDS FOR ARDUINO TO REBOOT\n");
	sleep(2);
	printf("DONE\n");

	// Spawn receiver thread
	pthread_t recv;

	pthread_create(&recv, NULL, receiveThread, NULL);

	// Send a hello packet
	TPacket helloPacket;

	helloPacket.packetType = PACKET_TYPE_HELLO;
	sendPacket(&helloPacket);

	while(!exitFlag)
	{
		char ch;
		if (manual){
			printf("WASD for movement SPACE=stop, m=toggle trans, q=exit, o=servo, c=medpak\n");
			scanf("%c", &ch);
			flushInput();		// Purge extraneous characters from input stream
		}
		else{
			ch = autotrans();
			if (ch == prevch){
				continue;
			}
			prevch = ch;
		}
		sendCommand(ch);
	}

	printf("Closing connection to Arduino.\n");
	endSerial();
}
