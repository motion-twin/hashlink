#define HL_NAME(n) directx_##n
#include <hl.h>
#include <xinput.h>
#include <dinput.h>


typedef struct _dx_gctrl dx_gctrl;
struct _dx_gctrl {
	int xUID;
	GUID guid;
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
static const char *s_ControllerMappings[] =
{
	"8f0e1200000000000000504944564944", //,Acme,platform:Windows,x:b2,a:b0,b:b1,y:b3,back:b8,start:b9,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b5,rightshoulder:b6,righttrigger:b7,leftstick:b10,rightstick:b11,leftx:a0,lefty:a1,rightx:a3,righty:a2,
	"341a3608000000000000504944564944", //,Afterglow PS3 Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,platform:Windows,
	"ffff0000000000000000504944564944", //,GameStop Gamepad,a:b0,b:b1,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,guide:,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b2,y:b3,platform:Windows,
	"6d0416c2000000000000504944564944", //,Generic DirectInput Controller,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,platform:Windows,
	"0d0f6e00000000000000504944564944", //,HORIPAD 4,a:b1,b:b2,y:b3,x:b0,start:b9,guide:b12,back:b8,leftstick:b10,rightstick:b11,leftshoulder:b4,rightshoulder:b5,dpup:h0.1,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,leftx:a0,lefty:a1,rightx:a2,righty:a3,lefttrigger:b6,righttrigger:b7,platform:Windows,
	"6d0419c2000000000000504944564944", //,Logitech F710 Gamepad,a:b1,b:b2,back:b8,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftshoulder:b4,leftstick:b10,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b11,righttrigger:b7,rightx:a2,righty:a3,start:b9,x:b0,y:b3,platform:Windows,
	"88880803000000000000504944564944", //,PS3 Controller,a:b2,b:b1,back:b8,dpdown:h0.8,dpleft:h0.4,dpright:h0.2,dpup:h0.1,guide:b12,leftshoulder:b4,leftstick:b9,lefttrigger:b6,leftx:a0,lefty:a1,rightshoulder:b5,rightstick:b10,righttrigger:b7,rightx:a3,righty:a4,start:b11,x:b0,y:b3,platform:Windows,
	"4c056802000000000000504944564944", //,PS3 Controller,a:b14,b:b13,back:b0,dpdown:b6,dpleft:b7,dpright:b5,dpup:b4,guide:b16,leftshoulder:b10,leftstick:b1,lefttrigger:b8,leftx:a0,lefty:a1,rightshoulder:b11,rightstick:b2,righttrigger:b9,rightx:a2,righty:a3,start:b3,x:b15,y:b12,platform:Windows,
	"25090500000000000000504944564944", //,PS3 DualShock,a:b2,b:b1,back:b9,dpdown:h0.8,dpleft:h0.4,dpright:h0.2,dpup:h0.1,guide:,leftshoulder:b6,leftstick:b10,lefttrigger:b4,leftx:a0,lefty:a1,rightshoulder:b7,rightstick:b11,righttrigger:b5,rightx:a2,righty:a3,start:b8,x:b0,y:b3,platform:Windows,
	"4c05c405000000000000504944564944", //,Sony DualShock 4,a:b1,b:b2,y:b3,x:b0,start:b9,guide:b12,back:b13,leftstick:b10,rightstick:b11,leftshoulder:b4,rightshoulder:b5,dpup:h0.1,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,leftx:a0,lefty:a1,rightx:a2,righty:a5,lefttrigger:a3,righttrigger:a4,platform:Windows,
	"4c05cc09000000000000504944564944", //,Sony DualShock 4,a:b1,b:b2,y:b3,x:b0,start:b9,guide:b12,back:b13,leftstick:b10,rightstick:b11,leftshoulder:b4,rightshoulder:b5,dpup:h0.1,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,leftx:a0,lefty:a1,rightx:a2,righty:a5,lefttrigger:a3,righttrigger:a4,platform:Windows,
	"4c05a00b000000000000504944564944", //,Sony DualShock 4 Wireless Adaptor,a:b1,b:b2,y:b3,x:b0,start:b9,guide:b12,back:b13,leftstick:b10,rightstick:b11,leftshoulder:b4,rightshoulder:b5,dpup:h0.1,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,leftx:a0,lefty:a1,rightx:a2,righty:a5,lefttrigger:a3,righttrigger:a4,platform:Windows,
	"6d0418c2000000000000504944564944", //,Logitech RumblePad 2 USB,platform:Windows,x:b0,a:b1,b:b2,y:b3,back:b8,start:b9,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,leftstick:b10,rightstick:b11,leftx:a0,lefty:a1,rightx:a2,righty:a3,
	"36280100000000000000504944564944", //,OUYA Controller,platform:Windows,a:b0,b:b3,y:b2,x:b1,start:b14,guide:b15,leftstick:b6,rightstick:b7,leftshoulder:b4,rightshoulder:b5,dpup:b8,dpleft:b10,dpdown:b9,dpright:b11,leftx:a0,lefty:a1,rightx:a3,righty:a4,lefttrigger:b12,righttrigger:b13,
	"4f0400b3000000000000504944564944", //,Thrustmaster Firestorm Dual Power,a:b0,b:b2,y:b3,x:b1,start:b10,guide:b8,back:b9,leftstick:b11,rightstick:b12,leftshoulder:b4,rightshoulder:b6,dpup:h0.1,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,leftx:a0,lefty:a1,rightx:a2,righty:a3,lefttrigger:b5,righttrigger:b7,platform:Windows,
	"00f00300000000000000504944564944", //,RetroUSB.com RetroPad,a:b1,b:b5,x:b0,y:b4,back:b2,start:b3,leftshoulder:b6,rightshoulder:b7,leftx:a0,lefty:a1,platform:Windows,
	"00f0f100000000000000504944564944", //,RetroUSB.com Super RetroPort,a:b1,b:b5,x:b0,y:b4,back:b2,start:b3,leftshoulder:b6,rightshoulder:b7,leftx:a0,lefty:a1,platform:Windows,
	"28040140000000000000504944564944", //,GamePad Pro USB,platform:Windows,a:b1,b:b2,x:b0,y:b3,back:b8,start:b9,leftshoulder:b4,rightshoulder:b5,leftx:a0,lefty:a1,lefttrigger:b6,righttrigger:b7,
	"ff113133000000000000504944564944", //,SVEN X-PAD,platform:Windows,a:b2,b:b3,y:b1,x:b0,start:b5,back:b4,leftshoulder:b6,rightshoulder:b7,dpup:h0.1,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,leftx:a0,lefty:a1,rightx:a2,righty:a4,lefttrigger:b8,righttrigger:b9,
	"8f0e0300000000000000504944564944", //,Piranha xtreme,platform:Windows,x:b3,a:b2,b:b1,y:b0,back:b8,start:b9,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,dpup:h0.1,leftshoulder:b6,lefttrigger:b4,rightshoulder:b7,righttrigger:b5,leftstick:b10,rightstick:b11,leftx:a0,lefty:a1,rightx:a3,righty:a2,
	"8f0e0d31000000000000504944564944", //,Multilaser JS071 USB,platform:Windows,a:b1,b:b2,y:b3,x:b0,start:b9,back:b8,leftstick:b10,rightstick:b11,leftshoulder:b4,rightshoulder:b5,dpup:h0.1,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,leftx:a0,lefty:a1,rightx:a2,righty:a3,lefttrigger:b6,righttrigger:b7,
	"10080300000000000000504944564944", //,PS2 USB,platform:Windows,a:b2,b:b1,y:b0,x:b3,start:b9,back:b8,leftstick:b10,rightstick:b11,leftshoulder:b6,rightshoulder:b7,dpup:h0.1,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,leftx:a0,lefty:a1,rightx:a4,righty:a2,lefttrigger:b4,righttrigger:b5,
	"79000600000000000000504944564944", //,G-Shark GS-GP702,a:b2,b:b1,x:b3,y:b0,back:b8,start:b9,leftstick:b10,rightstick:b11,leftshoulder:b4,rightshoulder:b5,dpup:h0.1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,leftx:a0,lefty:a1,rightx:a2,righty:a4,lefttrigger:b6,righttrigger:b7,platform:Windows,
	"4b12014d000000000000504944564944", //,NYKO AIRFLO,a:b0,b:b1,x:b2,y:b3,back:b8,guide:b10,start:b9,leftstick:a0,rightstick:a2,leftshoulder:a3,rightshoulder:b5,dpup:h0.1,dpdown:h0.0,dpleft:h0.8,dpright:h0.2,leftx:h0.6,lefty:h0.12,rightx:h0.9,righty:h0.4,lefttrigger:b6,righttrigger:b7,platform:Windows,
	"d6206dca000000000000504944564944", //,PowerA Pro Ex,a:b1,b:b2,x:b0,y:b3,back:b8,guide:b12,start:b9,leftstick:b10,rightstick:b11,leftshoulder:b4,rightshoulder:b5,dpup:h0.1,dpdown:h0.0,dpleft:h0.8,dpright:h0.2,leftx:a0,lefty:a1,rightx:a2,righty:a3,lefttrigger:b6,righttrigger:b7,platform:Windows,
	"a3060cff000000000000504944564944", //,Saitek P2500,a:b2,b:b3,y:b1,x:b0,start:b4,guide:b10,back:b5,leftstick:b8,rightstick:b9,leftshoulder:b6,rightshoulder:b7,dpup:h0.1,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,leftx:a0,lefty:a1,rightx:a2,righty:a3,platform:Windows,
	"4f0415b3000000000000504944564944", //,Thrustmaster Dual Analog 3.2,platform:Windows,x:b1,a:b0,b:b2,y:b3,back:b8,start:b9,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b5,rightshoulder:b6,righttrigger:b7,leftstick:b10,rightstick:b11,leftx:a0,lefty:a1,rightx:a2,righty:a3,
	"6f0e1e01000000000000504944564944", //,Rock Candy Gamepad for PS3,platform:Windows,a:b1,b:b2,x:b0,y:b3,back:b8,start:b9,guide:b12,leftshoulder:b4,rightshoulder:b5,leftstick:b10,rightstick:b11,leftx:a0,lefty:a1,rightx:a2,righty:a3,lefttrigger:b6,righttrigger:b7,dpup:h0.1,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,
	"83056020000000000000504944564944", //,iBuffalo USB 2-axis 8-button Gamepad,a:b1,b:b0,y:b2,x:b3,start:b7,back:b6,leftshoulder:b4,rightshoulder:b5,leftx:a0,lefty:a1,platform:Windows,
	"10080100000000000000504944564944", //,PS1 USB,platform:Windows,a:b2,b:b1,x:b3,y:b0,back:b8,start:b9,leftshoulder:b6,rightshoulder:b7,leftstick:b10,rightstick:b11,leftx:a0,lefty:a1,rightx:a3,righty:a2,lefttrigger:b4,righttrigger:b5,dpup:h0.1,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,
	"49190204000000000000504944564944", //,Ipega PG-9023,a:b0,b:b1,x:b3,y:b4,back:b10,start:b11,leftstick:b13,rightstick:b14,leftshoulder:b6,rightshoulder:b7,dpup:h0.1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,leftx:a0,lefty:a1,rightx:a3,righty:a4,lefttrigger:b8,righttrigger:b9,platform:Windows,
	"4f0423b3000000000000504944564944", //,Dual Trigger 3-in-1,a:b1,b:b2,x:b0,y:b3,back:b8,start:b9,leftstick:b10,rightstick:b11,leftshoulder:b4,rightshoulder:b5,dpup:h0.1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,leftx:a0,lefty:a1,rightx:a2,righty:a5,lefttrigger:b6,righttrigger:b7,platform:Windows,
	"0d0f4900000000000000504944564944", //,Hatsune Miku Sho Controller,a:b1,b:b2,x:b0,y:b3,back:b8,guide:b12,start:b9,leftstick:b10,rightstick:b11,leftshoulder:b4,rightshoulder:b5,dpup:h0.1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,leftx:a0,lefty:a1,rightx:a2,righty:a3,lefttrigger:b6,righttrigger:b7,platform:Windows,
	"79004318000000000000504944564944", //,Mayflash GameCube Controller Adapter,platform:Windows,a:b1,b:b2,x:b0,y:b3,back:b0,start:b9,guide:b0,leftshoulder:b4,rightshoulder:b7,leftstick:b0,rightstick:b0,leftx:a0,lefty:a1,rightx:a5,righty:a2,lefttrigger:a3,righttrigger:a4,dpup:h0.1,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,
	"79000018000000000000504944564944", //,Mayflash WiiU Pro Game Controller Adapter (DInput),a:b1,b:b2,x:b0,y:b3,back:b8,start:b9,leftstick:b10,rightstick:b11,leftshoulder:b4,rightshoulder:b5,dpup:h0.1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,leftx:a0,lefty:a1,rightx:a2,righty:a3,lefttrigger:b6,righttrigger:b7,platform:Windows,
	"2509e803000000000000504944564944", //,Mayflash Wii Classic Controller,a:b1,b:b0,x:b3,y:b2,back:b8,guide:b10,start:b9,leftshoulder:b4,rightshoulder:b5,dpup:h0.1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:b11,dpdown:b13,dpleft:b12,dpright:b14,leftx:a0,lefty:a1,rightx:a2,righty:a3,lefttrigger:b6,righttrigger:b7,platform:Windows,
	"300f1001000000000000504944564944", //,Saitek P480 Rumble Pad,a:b2,b:b3,x:b0,y:b1,back:b8,start:b9,leftstick:b10,rightstick:b11,leftshoulder:b4,rightshoulder:b6,dpup:h0.1,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,leftx:a0,lefty:a1,rightx:a3,righty:a2,lefttrigger:b5,righttrigger:b7,platform:Windows,
	"10280900000000000000504944564944", //,8Bitdo SFC30 GamePad,a:b1,b:b0,y:b3,x:b4,start:b11,back:b10,leftshoulder:b6,leftx:a0,lefty:a1,rightshoulder:b7,platform:Windows,
	"63252305000000000000504944564944", //,USB Vibration Joystick (BM),platform:Windows,x:b3,a:b2,b:b1,y:b0,back:b8,start:b9,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,dpup:h0.1,leftshoulder:b4,lefttrigger:b6,rightshoulder:b5,righttrigger:b7,leftstick:b10,rightstick:b11,leftx:a0,lefty:a1,rightx:a2,righty:a3,
	"20380900000000000000504944564944", //,8Bitdo NES30 PRO Wireless,platform:Windows,a:b0,b:b1,x:b3,y:b4,leftshoulder:b6,rightshoulder:b7,lefttrigger:b8,righttrigger:b9,back:b10,start:b11,leftstick:b13,rightstick:b14,leftx:a0,lefty:a1,rightx:a3,righty:a4,dpup:h0.1,dpright:h0.2,dpdown:h0.4,dpleft:h0.8,
	"02200090000000000000504944564944", //,8Bitdo NES30 PRO USB,platform:Windows,a:b0,b:b1,x:b3,y:b4,leftshoulder:b6,rightshoulder:b7,lefttrigger:b8,righttrigger:b9,back:b10,start:b11,leftstick:b13,rightstick:b14,leftx:a0,lefty:a1,rightx:a3,righty:a4,dpup:h0.1,dpright:h0.2,dpdown:h0.4,dpleft:h0.8,
	"ff113133000000000000504944564944", //,Gembird JPD-DualForce,platform:Windows,a:b2,b:b3,x:b0,y:b1,start:b9,back:b8,leftshoulder:b4,rightshoulder:b5,dpup:h0.1,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,leftx:a0,lefty:a1,rightx:a2,righty:a4,lefttrigger:b6,righttrigger:b7,leftstick:b10,rightstick:b11,
	"341a0108000000000000504944564944", //,EXEQ RF USB Gamepad 8206,a:b0,b:b1,x:b2,y:b3,leftshoulder:b4,rightshoulder:b5,leftstick:b8,rightstick:b7,back:b8,start:b9,dpdown:h0.4,dpleft:h0.8,dpright:h0.2,dpup:h0.1,leftx:a0,lefty:a1,rightx:a2,righty:a3,platform:Windows,
	"c0111352000000000000504944564944", //,Battalife Joystick,platform:Windows,x:b4,a:b6,b:b7,y:b5,back:b2,start:b3,leftshoulder:b0,rightshoulder:b1,leftx:a0,lefty:a1,
	"100801e5000000000000504944564944", //,NEXT Classic USB Game Controller,a:b0,b:b1,back:b8,start:b9,rightx:a2,righty:a3,leftx:a0,lefty:a1,platform:Windows,
	"79000600000000000000504944564944", //,NGS Phantom,a:b2,b:b3,y:b1,x:b0,start:b9,back:b8,leftstick:b10,rightstick:b11,leftshoulder:b4,rightshoulder:b5,dpup:h0.1,dpleft:h0.8,dpdown:h0.4,dpright:h0.2,leftx:a0,lefty:a1,rightx:a2,righty:a4,lefttrigger:b6,righttrigger:b7,platform:Windows,
	NULL
};


void gctrl_onDetect(dx_gctrl *ctrl, const vbyte *name) {
	if( !dx_gctrl_onDetect ) return;
	if( dx_gctrl_onDetect->hasValue )
		((void(*)(void *, dx_gctrl*, vbyte*))dx_gctrl_onDetect->fun)(dx_gctrl_onDetect->value, ctrl, (vbyte*)name);
	else
		((void(*)(dx_gctrl*, vbyte*))dx_gctrl_onDetect->fun)(ctrl, (vbyte*)name);
}

// XInput
void gctrl_xinput_add(int uid, dx_gctrl **current) {
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
	gctrl_onDetect(pad, (const vbyte*)(L"XInput controller"));
}

void gctrl_xinput_remove(dx_gctrl *pad) {
	free(pad);
}

void gctrl_xinput_update(dx_gctrl_data *data) {
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
	// TODO if( isXInput(instance) ) return DIENUM_CONTINUE;
	
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

	pad = (dx_gctrl*)malloc(sizeof(dx_gctrl));
	pad->xUID = -1;
	memcpy(&pad->guid, &instance->guidInstance, sizeof(pad->guid));
	pad->next = dx_gctrl_all;
	dx_gctrl_all = pad;
	gctrl_onDetect(pad, (const vbyte*)instance->tszProductName); // TODO get from mapping

	return DIENUM_CONTINUE;
}

void gctrl_dinput_remove(dx_gctrl *pad) {
	free(pad);
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