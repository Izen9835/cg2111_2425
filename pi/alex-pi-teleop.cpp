#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <stdint.h>
#include <sys/select.h>   // for select()
#include <sys/time.h>     // for gettimeofday()
#include "packet.h"
#include "serial.h"
#include "serialize.h"
#include "constants.h"

#define PORT_NAME			"/dev/ttyACM0"
#define BAUD_RATE			B9600

int exitFlag=0;
sem_t _xmitSema;

char get_key_select(int timeout_us) {
    char ch = 0;
    fd_set set;
    struct timeval timeout;
    
    FD_ZERO(&set);
    FD_SET(STDIN_FILENO, &set);
    
    timeout.tv_sec = 0;
    timeout.tv_usec = timeout_us;  // e.g., 50000 for 50ms
    
    int rv = select(STDIN_FILENO + 1, &set, NULL, NULL, &timeout);
    if (rv > 0) {
         read(STDIN_FILENO, &ch, 1);
    }
    return ch;
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
			printf("Command OK\n");
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

void handleMessage(TPacket *packet)
{
	printf("Message from Alex: %s\n", packet->data);
}

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

		case PACKET_TYPE_MESSAGE:
				handleMessage(packet);
			break;
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
	printf("Enter distance/angle in cm/degrees (e.g. 50) and power in %% (e.g. 75) separated by space.\n");
	printf("E.g. 50 75 means go at 50 cm at 75%% power for forward/backward, or 50 degrees left or right turn at 75%%  power\n");
	scanf("%d %d", &commandPacket->params[0], &commandPacket->params[1]);
	flushInput();
}

/*void sendCommand(char command)
{
	TPacket commandPacket;

	commandPacket.packetType = PACKET_TYPE_COMMAND;

	switch(command)
	{
		case 'f':
		case 'F':
			getParams(&commandPacket);
			commandPacket.command = COMMAND_FORWARD;
			sendPacket(&commandPacket);
			break;

		case 'b':
		case 'B':
			getParams(&commandPacket);
			commandPacket.command = COMMAND_REVERSE;
			sendPacket(&commandPacket);
			break;

		case 'l':
		case 'L':
			getParams(&commandPacket);
			commandPacket.command = COMMAND_TURN_LEFT;
			sendPacket(&commandPacket);
			break;

		case 'r':
		case 'R':
			getParams(&commandPacket);
			commandPacket.command = COMMAND_TURN_RIGHT;
			sendPacket(&commandPacket);
			break;

		case 's':
		case 'S':
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

		case 'q':
		case 'Q':
			exitFlag=1;
			break;

		default:
			printf("Bad command\n");

	}
}*/

void sendCommand(char command) {
    TPacket commandPacket;
    commandPacket.packetType = PACKET_TYPE_COMMAND;

    // Set max speed and indefinite time.
    commandPacket.params[0] = 0; // 0 signifies indefinite motion, see forward function in robotlib.ino for reference
    commandPacket.params[1] = 100;

    switch (command) {
        case 'w':
        case 'W':
            commandPacket.command = COMMAND_FORWARD;
            break;
        case 'a':
        case 'A':
            commandPacket.command = COMMAND_TURN_LEFT;
            break;
        case 's':
        case 'S':
            commandPacket.command = COMMAND_REVERSE;
            break;
        case 'd':
        case 'D':
            commandPacket.command = COMMAND_TURN_RIGHT;
            break;
        case ' ':
            commandPacket.command = COMMAND_STOP;
            break;
		case 'c':
		case 'C':
			commandPacket.command = COMMAND_CLEAR_STATS;
			commandPacket.params[0] = 0;
			sendPacket(&commandPacket);
			return;
		case 'g':
		case 'G':
			commandPacket.command = COMMAND_GET_STATS;
			sendPacket(&commandPacket);
			return;
        case 'q':
        case 'Q':
            exitFlag = 1;
            return;
        default:
            printf("Invalid key\n");
            return;
    }
    sendPacket(&commandPacket);
}

/*int main()
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
		printf("Command (w=forward, s=reverse, a=turn left, d=turn right, s=stop, c=clear stats, g=get stats q=exit)\n");
		scanf("%c", &ch);

		// Purge extraneous characters from input stream
		flushInput();

		sendCommand(ch);
	}

	printf("Closing connection to Arduino.\n");
	endSerial();
}*/

int main()
{
	// Connect to the Arduino.
	startSerial(PORT_NAME, BAUD_RATE, 8, 'N', 1, 5);

	// Wait two seconds for the Arduino to reboot.
	printf("WAITING TWO SECONDS FOR ARDUINO TO REBOOT\n");
	sleep(2);
	printf("DONE\n");

	// Spawn receiver thread.
	pthread_t recv;
	pthread_create(&recv, NULL, receiveThread, NULL);

	// Send a hello packet.
	TPacket helloPacket;
	helloPacket.packetType = PACKET_TYPE_HELLO;
	sendPacket(&helloPacket);

	// Set terminal to raw mode (disable canonical mode and echo) once.
	struct termios oldt, newt;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);

	printf("Use WASD to move. Release key to stop. Press 'Q' to quit.\n");

	int is_moving = 0;           // Whether we are in a moving state.
	struct timeval lastKeyTime;  // Time of the last valid movement key.
	struct timeval currentTime;
	gettimeofday(&lastKeyTime, NULL);

	// Main loop: poll key events with a 50ms timeout.
	while (!exitFlag) {
        char key = get_key_select(50000);  // Wait up to 50ms for key press.
        
        // If quit key is pressed, exit.
        if (key == 'q' || key == 'Q') {
            exitFlag = 1;
            break;
        }

        // If a valid movement key is received:
        if (key == 'w' || key == 'a' || key == 's' || key == 'd') {
            // Update timestamp.
            gettimeofday(&lastKeyTime, NULL);
            // If we weren't already moving, send the command.
            if (!is_moving) {
                sendCommand(key);
                is_moving = 1;
            }
        }
        
        // Check the elapsed time since the last key event.
        gettimeofday(&currentTime, NULL);
        long elapsed_ms = (currentTime.tv_sec - lastKeyTime.tv_sec) * 1000 +
                          (currentTime.tv_usec - lastKeyTime.tv_usec) / 1000;
        // If more than 100ms have passed with no key event, assume release.
        if (is_moving && elapsed_ms > 100) {
            sendCommand(' ');  // Send stop command.
            is_moving = 0;
        }

        usleep(10000);  // Small delay to avoid CPU overload.
    }

	// Restore terminal settings.
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

	printf("Closing connection to Arduino.\n");
	endSerial();

	return 0;
}
