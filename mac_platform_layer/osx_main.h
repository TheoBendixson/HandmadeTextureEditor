/*

   Handmade Texture Editor
   2020 Ted Bendixson
   Mooselutions, LLC

    mac_os_main.h

    A platform layer for the Mac.
*/

const unsigned short AKeyCode = 0x00;
const unsigned short CKeyCode = 0x08;
const unsigned short SKeyCode = 0x01;
const unsigned short FKeyCode = 0x03;
const unsigned short PKeyCode = 0x23;
const unsigned short YKeyCode = 0x10;
const unsigned short HKeyCode = 0x04;
const unsigned short JKeyCode = 0x26;
const unsigned short KKeyCode = 0x28;
const unsigned short LKeyCode = 0x25;
const unsigned short MKeyCode = 0x2E;
const unsigned short NKeyCode = 0x2D;
const unsigned short LeftArrowKeyCode = 0x7B;
const unsigned short RightArrowKeyCode = 0x7C;
const unsigned short DownArrowKeyCode = 0x7D;
const unsigned short UpArrowKeyCode = 0x7E;
const unsigned short F1KeyCode = 0x7A;
const unsigned short F2KeyCode = 0x78;
const unsigned short F4KeyCode = 0x76;
const unsigned short F5KeyCode = 0x60;
const unsigned short F6KeyCode = 0x61;
const unsigned short F7KeyCode = 0x62;
const unsigned short ReturnKeyCode = 0x24;

struct mac_game_controller
{
    int32 LeftThumbXUsageID;
    int32 LeftThumbYUsageID;
    int32 ButtonAUsageID;
    int32 ButtonBUsageID;
    int32 ButtonXUsageID;
    int32 ButtonYUsageID;
    int32 ButtonLeftShoulderUsageID;
    int32 ButtonRightShoulderUsageID;
    int32 ButtonLeftShoulder2UsageID;
    int32 ButtonRightShoulder2UsageID;

    int32 ButtonStartUsageID;
    int32 ButtonSelectUsageID;
    
    // Values
    real32 LeftThumbstickX;
    real32 LeftThumbstickY;
    bool32 UsesHatSwitch;

    int32 DPadX;
    int32 DPadY;

    bool32 ButtonAState;
    bool32 ButtonBState;
    bool32 ButtonXState;
    bool32 ButtonYState;
    bool32 ButtonLeftShoulderState;
    bool32 ButtonRightShoulderState;
    bool32 ButtonLeftShoulder2State;
    bool32 ButtonRightShoulder2State;

    bool32 ButtonStartState;
    bool32 ButtonSelectState;
};

#define MAC_MAX_FILENAME_SIZE 4096

struct mac_app_path
{
    char Filename[MAC_MAX_FILENAME_SIZE];
    char *OnePastLastAppFileNameSlash;
};

struct mac_state
{
    mac_app_path *Path;

	char ResourcesDirectory[MAC_MAX_FILENAME_SIZE];
	int ResourcesDirectorySize;

};

