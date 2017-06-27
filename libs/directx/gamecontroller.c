#define HL_NAME(n) directx_##n
#include <hl.h>
#include <xinput.h>

typedef struct _dx_gctrl dx_gctrl;
struct _dx_gctrl {
	int xUID;
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

static dx_gctrl *dx_gctrl_all = NULL;

HL_PRIM void HL_NAME(gctrl_init)() {
}

void gctrl_onDetect(vclosure *onDetect, dx_gctrl *ctrl, bool added) {
	if( onDetect->hasValue )
		((void(*)(void *, dx_gctrl*, bool))onDetect->fun)(onDetect->value, ctrl, added);
	else
		((void(*)(dx_gctrl*, bool))onDetect->fun)(ctrl, added);
}

void gctrl_addXInput(int uid, dx_gctrl **current, vclosure *onDetect) {
	dx_gctrl *pad = *current;
	dx_gctrl *prev = NULL;
	while (pad) {
		if (pad->xUID == uid) {
			if (pad == *current)
				*current = pad->next;
			else if (prev)
				prev->next = pad->next;

			pad->next = dx_gctrl_all;
			dx_gctrl_all = pad;
			return;
		}

		pad = pad->next;
	}

	pad = (dx_gctrl*)malloc(sizeof(dx_gctrl));
	pad->xUID = uid;
	pad->next = dx_gctrl_all;
	dx_gctrl_all = pad;
	gctrl_onDetect(onDetect, pad, true);
}

void gctrl_removeXInput(dx_gctrl *pad, vclosure *onDetect) {
	gctrl_onDetect(onDetect, pad, false);
	free(pad);
}

HL_PRIM void HL_NAME(gctrl_detect)( vclosure *onDetect) {
	dx_gctrl *current = dx_gctrl_all;
	dx_gctrl_all = NULL;

	// XInput
	for (int uid = XUSER_MAX_COUNT - 1; uid >= 0; uid--) {
		XINPUT_CAPABILITIES capabilities;
		if (XInputGetCapabilities(uid, XINPUT_FLAG_GAMEPAD, &capabilities) == ERROR_SUCCESS) {
			gctrl_addXInput(uid, &current, onDetect);
		}
	}

	// TODO DInput

	while (current) {
		dx_gctrl *next = current->next;

		if( current->xUID >= 0 )
			gctrl_removeXInput(current, onDetect);
		// TODO DInput
		
		current = next;
	}
}

void gctrl_updateXInput(dx_gctrl_data *data) {
	XINPUT_STATE state;
	HRESULT r = XInputGetState(data->ptr->xUID, &state);
	if (r == ERROR_DEVICE_NOT_CONNECTED) 
		return;

	data->buttons = (state.Gamepad.wButtons&0x0FFF) | (state.Gamepad.wButtons&0xF000) >> 2;
	data->lx = (double)(state.Gamepad.sThumbLX < -32767 ? -1. : (state.Gamepad.sThumbLX / 32767.));
	data->ly = (double)(state.Gamepad.sThumbLY < -32767 ? -1. : (state.Gamepad.sThumbLY / 32767.));
	data->rx = (double)(state.Gamepad.sThumbRX < -32767 ? -1. : (state.Gamepad.sThumbRX / 32767.));
	data->ry = (double)(state.Gamepad.sThumbRY < -32767 ? -1. : (state.Gamepad.sThumbRY / 32767.));
	data->lt = (double)(state.Gamepad.bLeftTrigger / 255.);
	data->rt = (double)(state.Gamepad.bRightTrigger / 255.);
}

HL_PRIM void HL_NAME(gctrl_update)(dx_gctrl_data *data) {
	if (data->ptr->xUID >= 0)
		gctrl_updateXInput(data);
}

HL_PRIM void HL_NAME(gctrl_set_vibration)(dx_gctrl *ctrl, double strength) {
	if (ctrl->xUID >= 0) {
		XINPUT_VIBRATION vibration;
		vibration.wLeftMotorSpeed = vibration.wRightMotorSpeed = (WORD)(strength * 65535);
		XInputSetState(ctrl->xUID, &vibration);
	}
}

HL_PRIM vbyte *HL_NAME(gctrl_name)(dx_gctrl *gctrl) {
	return "XInput";
}

#define TGAMECTRL _ABSTRACT(dx_gctrl)
DEFINE_PRIM(_VOID, gctrl_init, _NO_ARG);
DEFINE_PRIM(_VOID, gctrl_detect, _FUN(_VOID, TGAMECTRL _BOOL));
DEFINE_PRIM(_BYTES, gctrl_name, TGAMECTRL);
DEFINE_PRIM(_VOID, gctrl_update, _OBJ(TGAMECTRL _STRING _I32 _F64 _F64 _F64 _F64 _F64 _F64));
DEFINE_PRIM(_VOID, gctrl_set_vibration, TGAMECTRL _F64);