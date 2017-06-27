#define HL_NAME(n) directx_##n
#include <hl.h>
#include <xinput.h>
#include <dinput.h>

#define HAT_CENTERED    0x00
#define HAT_UP          0x01
#define HAT_RIGHT       0x02
#define HAT_DOWN        0x04
#define HAT_LEFT        0x08

typedef struct {
	int num;
	int mask; // for hat only
} dinput_btn;

typedef struct {
	unsigned int guid;
	char *name;
	dinput_btn button[14];
	long axis[6];
} dinput_mapping;


typedef struct _dx_gctrl dx_gctrl;
struct _dx_gctrl {
	int xUID;
	GUID guid;
	LPDIRECTINPUTDEVICE8 device;
	dinput_mapping *mapping;
	dx_gctrl *next;
};

typedef struct {
	hl_type *t;
	dx_gctrl *ptr;
	vstring *name;
	int buttons;
	double lx;
	double ly;
	double rx;
	double ry;
	double lt;
	double rt;
} dx_gctrl_data;


static vclosure *dx_gctrl_onDetect = NULL;
static dx_gctrl *dx_gctrl_all = NULL;

// DirectInput specific
static LPDIRECTINPUT8 dinput = NULL;

// Based on https://github.com/gabomdq/SDL_GameControllerDB
static dinput_mapping gctrl_dinput_mappings[] =
{
	//{0x00120e8f, "Acme"}, //x:b2,a:b0,b:b1,y:b3,back:b8,start:b9,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b5,rightshoulder:b6,righttrigger:b7,leftstick:b10,rightstick:b11,leftx:a0,lefty:a1,rightx:a3,righty:a2,
	//{0x08361a34, "Afterglow PS3 Controller"}, //a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,
	//{0x0000ffff, "GameStop Gamepad"}, //a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,
	//{0xc216046d, "Generic DirectInput Controller"}, //a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,
	//{0x006e0f0d, "HORIPAD 4"}, //a:b1,b:b2,y:b3,x:b0,start:b9,guide:b12,back:b8,leftstick:b10,rightstick:b11,leftshoulder:b4,rightshoulder:b5,dpup:h0.1,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,leftx:a0,lefty:a1,rightx:a2,righty:a3,lefttrigger:b6,righttrigger:b7,
	//{0xc219046d, "Logitech F710 Gamepad"}, //a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,
	//{0x03088888, "PS3 Controller"}, //a:b2,b:b1,back:b8,dpdown:h0.8,dpleft:h0.4,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b9,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:b7,rightx:a3,righty:a4,start:b11,x:b0,y:b3,
	//{0x0268054c, "PS3 Controller"}, //a:b14,b:b13,back:b0,dpdown:b6,dpleft:b7,dpright:b5,dpup:b4,guide:b16,leftshoulder:b10,leftstick:b1,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b11,rightstick:b2,righttrigger:b9,rightx:a2,righty:a3,start:b3,x:b15,y:b12,
	//{0x00050925, "PS3 DualShock"}, //a:b2,b:b1,back:b9,dpdown:h0.8,dpleft:h0.4,dpright:h0.2,dpup:h0.1,guide:,leftshoulder:b6,leftstick:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b11,righttrigger:b5,rightx:a2,righty:a3,start:b8,x:b0,y:b3,
	{0x05c4054c, "Sony DualShock 4", {{0,1},{0,4},{0,8},{0,2},{9,0},{13,0},{10,0},{11,0},{4,0},{5,0},{1,0},{2,0},{0,0},{3,0}}, {0,1,2,5,3,4}},
	//{0x09cc054c, "Sony DualShock 4"}, //a:b1,b:b2,y:b3,x:b0,start:b9,guide:b12,back:b13,leftstick:b10,rightstick:b11,leftshoulder:b4,rightshoulder:b5,dpup:h0.1,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,leftx:a0,lefty:a1,rightx:a2,righty:a5,lefttrigger:a3,righttrigger:a4,
	//{0x0ba0054c, "Sony DualShock 4 Wireless Adaptor"}, //a:b1,b:b2,y:b3,x:b0,start:b9,guide:b12,back:b13,leftstick:b10,rightstick:b11,leftshoulder:b4,rightshoulder:b5,dpup:h0.1,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,leftx:a0,lefty:a1,rightx:a2,righty:a5,lefttrigger:a3,righttrigger:a4,
	//{0xc218046d, "Logitech RumblePad 2 USB"}, //x:b0,a:b1,b:b2,y:b3,back:b8,start:b9,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,leftstick:b10,rightstick:b11,leftx:a0,lefty:a1,rightx:a2,righty:a3,
	//{0x00012836, "OUYA Controller"}, //a:b0,b:b3,y:b2,x:b1,start:b14,guide:b15,leftstick:b6,rightstick:b7,leftshoulder:b4,rightshoulder:b5,dpup:b8,dpleft:b10,dpdown:b9,dpright:b11,leftx:a0,lefty:a1,rightx:a3,righty:a4,lefttrigger:b12,righttrigger:b13,
	//{0xb300044f, "Thrustmaster Firestorm Dual Power"}, //a:b0,b:b2,y:b3,x:b1,start:b10,guide:b8,back:b9,leftstick:b11,rightstick:b12,leftshoulder:b4,rightshoulder:b6,dpup:h0.1,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,leftx:a0,lefty:a1,rightx:a2,righty:a3,lefttrigger:b5,righttrigger:b7,
	//{0x0003f000, "RetroUSB.com RetroPad"}, //a:b1,b:b5,x:b0,y:b4,back:b2,start:b3,leftshoulder:b6,rightshoulder:b7,leftx:a0,lefty:a1,
	//{0x00f1f000, "RetroUSB.com Super RetroPort"}, //a:b1,b:b5,x:b0,y:b4,back:b2,start:b3,leftshoulder:b6,rightshoulder:b7,leftx:a0,lefty:a1,
	//{0x40010428, "GamePad Pro USB"}, //a:b1,b:b2,x:b0,y:b3,back:b8,start:b9,leftshoulder:b4,rightshoulder:b5,leftx:a0,lefty:a1,lefttrigger:b6,righttrigger:b7,
	//{0x333111ff, "SVEN X-PAD"}, //a:b2,b:b3,y:b1,x:b0,start:b5,back:b4,leftshoulder:b6,rightshoulder:b7,dpup:h0.1,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,leftx:a0,lefty:a1,rightx:a2,righty:a4,lefttrigger:b8,righttrigger:b9,
	//{0x00030e8f, "Piranha xtreme"}, //x:b3,a:b2,b:b1,y:b0,back:b8,start:b9,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,dpup:h0.1,leftshoulder:b6,lefttrigger:b4,rightshoulder:b7,righttrigger:b5,leftstick:b10,rightstick:b11,leftx:a0,lefty:a1,rightx:a3,righty:a2,
	//{0x310d0e8f, "Multilaser JS071 USB"}, //a:b1,b:b2,y:b3,x:b0,start:b9,back:b8,leftstick:b10,rightstick:b11,leftshoulder:b4,rightshoulder:b5,dpup:h0.1,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,leftx:a0,lefty:a1,rightx:a2,righty:a3,lefttrigger:b6,righttrigger:b7,
	//{0x00030810, "PS2 USB"}, //a:b2,b:b1,y:b0,x:b3,start:b9,back:b8,leftstick:b10,rightstick:b11,leftshoulder:b6,rightshoulder:b7,dpup:h0.1,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,leftx:a0,lefty:a1,rightx:a4,righty:a2,lefttrigger:b4,righttrigger:b5,
	//{0x00060079, "G-Shark GS-GP702"}, //a:b2,b:b1,x:b3,y:b0,back:b8,start:b9,leftstick:b10,rightstick:b11,leftshoulder:b4,rightshoulder:b5,dpup:h0.1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,leftx:a0,lefty:a1,rightx:a2,righty:a4,lefttrigger:b6,righttrigger:b7,
	//{0x4d01124b, "NYKO AIRFLO"}, //a:b0,b:b1,x:b2,y:b3,back:b8,guide:b10,start:b9,leftstick:a0,rightstick:a2,leftshoulder:a3,rightshoulder:b5,dpup:h0.1,dpdown:h0.0,dpleft:h0.8,dpright:h0.2,leftx:h0.6,lefty:h0.12,rightx:h0.9,righty:h0.4,lefttrigger:b6,righttrigger:b7,
	//{0xca6d20d6, "PowerA Pro Ex"}, //a:b1,b:b2,x:b0,y:b3,back:b8,guide:b12,start:b9,leftstick:b10,rightstick:b11,leftshoulder:b4,rightshoulder:b5,dpup:h0.1,dpdown:h0.0,dpleft:h0.8,dpright:h0.2,leftx:a0,lefty:a1,rightx:a2,righty:a3,lefttrigger:b6,righttrigger:b7,
	//{0xff0c06a3, "Saitek P2500"}, //a:b2,b:b3,y:b1,x:b0,start:b4,guide:b10,back:b5,leftstick:b8,rightstick:b9,leftshoulder:b6,rightshoulder:b7,dpup:h0.1,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,leftx:a0,lefty:a1,rightx:a2,righty:a3,
	//{0xb315044f, "Thrustmaster Dual Analog 3.2"}, //x:b1,a:b0,b:b2,y:b3,back:b8,start:b9,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b5,rightshoulder:b6,righttrigger:b7,leftstick:b10,rightstick:b11,leftx:a0,lefty:a1,rightx:a2,righty:a3,
	//{0x011e0e6f, "Rock Candy Gamepad for PS3"}, //a:b1,b:b2,x:b0,y:b3,back:b8,start:b9,guide:b12,leftshoulder:b4,rightshoulder:b5,leftstick:b10,rightstick:b11,leftx:a0,lefty:a1,rightx:a2,righty:a3,lefttrigger:b6,righttrigger:b7,dpup:h0.1,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,
	//{0x20600583, "iBuffalo USB 2-axis 8-button Gamepad"}, //a:b1,b:b0,y:b2,x:b3,start:b7,back:b6,leftshoulder:b4,rightshoulder:b5,leftx:a0,lefty:a1,
	//{0x00010810, "PS1 USB"}, //a:b2,b:b1,x:b3,y:b0,back:b8,start:b9,leftshoulder:b6,rightshoulder:b7,leftstick:b10,rightstick:b11,leftx:a0,lefty:a1,rightx:a3,righty:a2,lefttrigger:b4,righttrigger:b5,dpup:h0.1,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,
	//{0x04021949, "Ipega PG-9023"}, //a:b0,b:b1,x:b3,y:b4,back:b10,start:b11,leftstick:b13,rightstick:b14,leftshoulder:b6,rightshoulder:b7,dpup:h0.1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,leftx:a0,lefty:a1,rightx:a3,righty:a4,lefttrigger:b8,righttrigger:b9,
	//{0xb323044f, "Dual Trigger 3-in-1"}, //a:b1,b:b2,x:b0,y:b3,back:b8,start:b9,leftstick:b10,rightstick:b11,leftshoulder:b4,rightshoulder:b5,dpup:h0.1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,leftx:a0,lefty:a1,rightx:a2,righty:a5,lefttrigger:b6,righttrigger:b7,
	//{0x00490f0d, "Hatsune Miku Sho Controller"}, //a:b1,b:b2,x:b0,y:b3,back:b8,guide:b12,start:b9,leftstick:b10,rightstick:b11,leftshoulder:b4,rightshoulder:b5,dpup:h0.1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,leftx:a0,lefty:a1,rightx:a2,righty:a3,lefttrigger:b6,righttrigger:b7,
	//{0x18430079, "Mayflash GameCube Controller Adapter"}, //a:b1,b:b2,x:b0,y:b3,back:b0,start:b9,guide:b0,leftshoulder:b4,rightshoulder:b7,leftstick:b0,rightstick:b0,leftx:a0,lefty:a1,rightx:a5,righty:a2,lefttrigger:a3,righttrigger:a4,dpup:h0.1,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,
	//{0x18000079, "Mayflash WiiU Pro Game Controller Adapter (DInput)"}, //a:b1,b:b2,x:b0,y:b3,back:b8,start:b9,leftstick:b10,rightstick:b11,leftshoulder:b4,rightshoulder:b5,dpup:h0.1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,leftx:a0,lefty:a1,rightx:a2,righty:a3,lefttrigger:b6,righttrigger:b7,
	//{0x03e80925, "Mayflash Wii Classic Controller"}, //a:b1,b:b0,x:b3,y:b2,back:b8,guide:b10,start:b9,leftshoulder:b4,rightshoulder:b5,dpup:h0.1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:b11,dpdown:b13,dpleft:b12,dpright:b14,leftx:a0,lefty:a1,rightx:a2,righty:a3,lefttrigger:b6,righttrigger:b7,
	//{0x01100f30, "Saitek P480 Rumble Pad"}, //a:b2,b:b3,x:b0,y:b1,back:b8,start:b9,leftstick:b10,rightstick:b11,leftshoulder:b4,rightshoulder:b6,dpup:h0.1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,leftx:a0,lefty:a1,rightx:a3,righty:a2,lefttrigger:b5,righttrigger:b7,
	//{0x00092810, "8Bitdo SFC30 GamePad"}, //a:b1,b:b0,y:b3,x:b4,start:b11,back:b10,leftshoulder:b6,leftx:a0,lefty:a1,rightshoulder:b7,
	//{0x05232563, "USB Vibration Joystick (BM)"}, //x:b3,a:b2,b:b1,y:b0,back:b8,start:b9,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,leftstick:b10,rightstick:b11,leftx:a0,lefty:a1,rightx:a2,righty:a3,
	//{0x00093820, "8Bitdo NES30 PRO Wireless"}, //a:b0,b:b1,x:b3,y:b4,leftshoulder:b6,rightshoulder:b7,lefttrigger:b8,righttrigger:b9,back:b10,start:b11,leftstick:b13,rightstick:b14,leftx:a0,lefty:a1,rightx:a3,righty:a4,dpup:h0.1,dpright:h0.2,dpdown:h0.4,dpleft:h0.8,
	//{0x90002002, "8Bitdo NES30 PRO USB"}, //a:b0,b:b1,x:b3,y:b4,leftshoulder:b6,rightshoulder:b7,lefttrigger:b8,righttrigger:b9,back:b10,start:b11,leftstick:b13,rightstick:b14,leftx:a0,lefty:a1,rightx:a3,righty:a4,dpup:h0.1,dpright:h0.2,dpdown:h0.4,dpleft:h0.8,
	//{0x333111ff, "Gembird JPD-DualForce"}, //a:b2,b:b3,x:b0,y:b1,start:b9,back:b8,leftshoulder:b4,rightshoulder:b5,dpup:h0.1,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,leftx:a0,lefty:a1,rightx:a2,righty:a4,lefttrigger:b6,righttrigger:b7,leftstick:b10,rightstick:b11,
	//{0x08011a34, "EXEQ RF USB Gamepad 8206"}, //a:b0,b:b1,x:b2,y:b3,leftshoulder:b4,rightshoulder:b5,leftstick:b8,rightstick:b7,back:b8,start:b9,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftx:a0,lefty:a1,rightx:a2,righty:a3,
	//{0x521311c0, "Battalife Joystick"}, //x:b4,a:b6,b:b7,y:b5,back:b2,start:b3,leftshoulder:b0,rightshoulder:b1,leftx:a0,lefty:a1,
	//{0xe5010810, "NEXT Classic USB Game Controller"}, //a:b0,b:b1,back:b8,start:b9,rightx:a2,righty:a3,leftx:a0,lefty:a1,
	//{0x00060079, "NGS Phantom"}, //a:b2,b:b3,y:b1,x:b0,start:b9,back:b8,leftstick:b10,rightstick:b11,leftshoulder:b4,rightshoulder:b5,dpup:h0.1,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,leftx:a0,lefty:a1,rightx:a2,righty:a4,lefttrigger:b6,righttrigger:b7,
	{0x0,""}
};

static void gctrl_onDetect(dx_gctrl *ctrl, const char *name) {
	if( !dx_gctrl_onDetect ) return;
	if( dx_gctrl_onDetect->hasValue )
		((void(*)(void *, dx_gctrl*, vbyte*))dx_gctrl_onDetect->fun)(dx_gctrl_onDetect->value, ctrl, (vbyte*)name);
	else
		((void(*)(dx_gctrl*, vbyte*))dx_gctrl_onDetect->fun)(ctrl, (vbyte*)name);
}

// XInput
static void gctrl_xinput_add(int uid, dx_gctrl **current) {
	dx_gctrl *pad = *current;
	dx_gctrl *prev = NULL;
	while (pad) {
		if (pad->xUID == uid) {
			if (pad == *current) *current = pad->next;
			else if (prev) prev->next = pad->next;

			pad->next = dx_gctrl_all;
			dx_gctrl_all = pad;
			return;
		}

		prev = pad;
		pad = pad->next;
	}

	pad = (dx_gctrl*)malloc(sizeof(dx_gctrl));
	pad->xUID = uid;
	pad->next = dx_gctrl_all;
	dx_gctrl_all = pad;
	gctrl_onDetect(pad, "XInput controller");
}

static void gctrl_xinput_remove(dx_gctrl *pad) {
	free(pad);
}

static void gctrl_xinput_update(dx_gctrl_data *data) {
	XINPUT_STATE state;
	HRESULT r = XInputGetState(data->ptr->xUID, &state);
	if (r == ERROR_DEVICE_NOT_CONNECTED)
		return;

	data->buttons = (state.Gamepad.wButtons & 0x0FFF) | (state.Gamepad.wButtons & 0xF000) >> 2;
	data->lx = (double)(state.Gamepad.sThumbLX < -32767 ? -1. : (state.Gamepad.sThumbLX / 32767.));
	data->ly = (double)(state.Gamepad.sThumbLY < -32767 ? -1. : (state.Gamepad.sThumbLY / 32767.));
	data->rx = (double)(state.Gamepad.sThumbRX < -32767 ? -1. : (state.Gamepad.sThumbRX / 32767.));
	data->ry = (double)(state.Gamepad.sThumbRY < -32767 ? -1. : (state.Gamepad.sThumbRY / 32767.));
	data->lt = (double)(state.Gamepad.bLeftTrigger / 255.);
	data->rt = (double)(state.Gamepad.bRightTrigger / 255.);
}

// DirectInput

static void gctrl_dinput_init() {
	HINSTANCE instance = GetModuleHandle(NULL);
	if( !instance )
		return;
	DirectInput8Create(instance, DIRECTINPUT_VERSION, &IID_IDirectInput8, &dinput, NULL);
}

static BOOL CALLBACK gctrl_dinput_deviceCb(const DIDEVICEINSTANCE *instance, void *context) {
	const DWORD devType = (instance->dwDevType & 0xFF);	
	if( devType != DI8DEVTYPE_GAMEPAD ) return DIENUM_CONTINUE;
	
	dx_gctrl **current = (dx_gctrl**)context;
	dx_gctrl *pad = *current;
	dx_gctrl *prev = NULL;
	while( pad ) {
		if( pad->xUID < 0 && !memcmp(&pad->guid, &instance->guidInstance, sizeof(pad->guid)) ) {
			if( pad == *current ) *current = pad->next;
			else if( prev ) prev->next = pad->next;

			pad->next = dx_gctrl_all;
			dx_gctrl_all = pad;
			return DIENUM_CONTINUE;
		}

		prev = pad;
		pad = pad->next;
	}

	// Find mapping
	int i = 0;
	dinput_mapping *mapping = &gctrl_dinput_mappings[i];
	while( mapping->guid ) {
		if( instance->guidProduct.Data1 == mapping->guid ) 
			break;
		mapping = &gctrl_dinput_mappings[++i];
	}
	if( !mapping->guid ) return DIENUM_CONTINUE;

	LPDIRECTINPUTDEVICE8 device;
	HRESULT result = IDirectInput8_CreateDevice(dinput, &instance->guidInstance, &device, NULL);
	if( FAILED(result) ) return DIENUM_CONTINUE;
	
	pad = (dx_gctrl*)malloc(sizeof(dx_gctrl));

	result = IDirectInputDevice8_QueryInterface(device, &IID_IDirectInputDevice8, (LPVOID *)&pad->device);
	IDirectInputDevice8_Release(device);
	if( FAILED(result) ) {
		free(pad);
		return DIENUM_CONTINUE;
	}

	result = IDirectInputDevice8_SetDataFormat(pad->device, &c_dfDIJoystick2);
	if( FAILED(result) ) {
		free(pad);
		return DIENUM_CONTINUE;
	}

	pad->mapping = mapping;
	pad->xUID = -1;
	memcpy(&pad->guid, &instance->guidInstance, sizeof(pad->guid));
	pad->next = dx_gctrl_all;
	dx_gctrl_all = pad;
	gctrl_onDetect(pad, mapping->name);

	return DIENUM_CONTINUE;
}

static void gctrl_dinput_remove(dx_gctrl *pad) {
	free(pad);
}

// Based on SDL ( https://hg.libsdl.org/SDL/file/007dfe83abf8/src/joystick/windows/SDL_dinputjoystick.c )
static int gctrl_dinput_translatePOV(DWORD value) {
	const int HAT_VALS[] = {
		HAT_UP,
		HAT_UP | HAT_RIGHT,
		HAT_RIGHT,
		HAT_DOWN | HAT_RIGHT,
		HAT_DOWN,
		HAT_DOWN | HAT_LEFT,
		HAT_LEFT,
		HAT_UP | HAT_LEFT
	};

	if( LOWORD(value) == 0xFFFF ) return HAT_CENTERED;

	value += 4500 / 2;
	value %= 36000;
	value /= 4500;

	if( value < 0 || value >= 8 ) return HAT_CENTERED;

	return HAT_VALS[value];
}

static void gctrl_dinput_update(dx_gctrl_data *data) {
	DIJOYSTATE2 state;
	HRESULT result = IDirectInputDevice8_GetDeviceState(data->ptr->device, sizeof(DIJOYSTATE2), &state);
	if( result == DIERR_INPUTLOST || result == DIERR_NOTACQUIRED ) {
		IDirectInputDevice8_Acquire(data->ptr->device);
		result = IDirectInputDevice8_GetDeviceState(data->ptr->device, sizeof(DIJOYSTATE2), &state);
	}

	if( result != DI_OK )
		return;

	long *p = (long*)&state;
	data->lx = (double)((*(p + data->ptr->mapping->axis[0])) / 65535.) * 2 - 1.;
	data->ly = (double)((*(p + data->ptr->mapping->axis[1])) / 65535.) * -2 + 1.;
	data->rx = (double)((*(p + data->ptr->mapping->axis[2])) / 65535.) * 2 - 1.;
	data->ry = (double)((*(p + data->ptr->mapping->axis[3])) / 65535.) * -2 + 1.;
	data->lt = (double)((*(p + data->ptr->mapping->axis[4])) / 65535.);
	data->rt = (double)((*(p + data->ptr->mapping->axis[5])) / 65535.);

	data->buttons = 0;
	dinput_btn *bmap;
	for( int i = 0; i < 14; i++ ) {
		bmap = &data->ptr->mapping->button[i];
		if( bmap->mask == 0 ) {
			data->buttons |= (state.rgbButtons[bmap->num] > 0 ? 1 : 0) << i;
		} else {
			data->buttons |= ((gctrl_dinput_translatePOV(state.rgdwPOV[bmap->num])&bmap->mask) > 0 ? 1 : 0) << i;
		}
	}
	printf("\n");
}

// 

HL_PRIM void HL_NAME(gctrl_init)() {
	gctrl_dinput_init();
}

HL_PRIM void HL_NAME(gctrl_detect)( vclosure *onDetect ) {
	dx_gctrl *current = dx_gctrl_all;
	dx_gctrl_all = NULL;
	dx_gctrl_onDetect = onDetect;

	// XInput
	for( int uid = XUSER_MAX_COUNT - 1; uid >= 0; uid-- ){
		XINPUT_CAPABILITIES capabilities;
		if (XInputGetCapabilities(uid, XINPUT_FLAG_GAMEPAD, &capabilities) == ERROR_SUCCESS) 
			gctrl_xinput_add(uid, &current);
	}

	// DInput
	IDirectInput8_EnumDevices(dinput, DI8DEVCLASS_GAMECTRL, gctrl_dinput_deviceCb, &current, DIEDFL_ATTACHEDONLY);

	while( current ){
		dx_gctrl *next = current->next;

		gctrl_onDetect(current, NULL);

		if( current->xUID >= 0 )
			gctrl_xinput_remove(current);
		else
			gctrl_dinput_remove(current);
		
		current = next;
	}
	dx_gctrl_onDetect = NULL;
}

HL_PRIM void HL_NAME(gctrl_update)(dx_gctrl_data *data) {
	if( data->ptr->xUID >= 0 )
		gctrl_xinput_update(data);
	else
		gctrl_dinput_update(data);
}

HL_PRIM void HL_NAME(gctrl_set_vibration)(dx_gctrl *ctrl, double strength) {
	if( ctrl->xUID >= 0 ){
		XINPUT_VIBRATION vibration;
		vibration.wLeftMotorSpeed = vibration.wRightMotorSpeed = (WORD)(strength * 65535);
		XInputSetState(ctrl->xUID, &vibration);
	}
}

#define TGAMECTRL _ABSTRACT(dx_gctrl)
DEFINE_PRIM(_VOID, gctrl_init, _NO_ARG);
DEFINE_PRIM(_VOID, gctrl_detect, _FUN(_VOID, TGAMECTRL _BYTES));
DEFINE_PRIM(_VOID, gctrl_update, _OBJ(TGAMECTRL _STRING _I32 _F64 _F64 _F64 _F64 _F64 _F64));
DEFINE_PRIM(_VOID, gctrl_set_vibration, TGAMECTRL _F64);