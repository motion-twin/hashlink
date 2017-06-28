#define HL_NAME(n) directx_##n
#include <hl.h>
#include <xinput.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#define HAT_CENTERED    0x00
#define HAT_UP          0x01
#define HAT_RIGHT       0x02
#define HAT_DOWN        0x04
#define HAT_LEFT        0x08

typedef struct {
	hl_type *t;
	int num;
	int mask; // for hat only
} dinput_mapping_btn;

typedef struct {
	hl_type *t;
	unsigned int guid;
	vbyte *name;
	varray *button; // dinput_mapping_btn
	varray *axis; // int
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
static varray *gctrl_dinput_mappings = NULL;

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

static void gctrl_dinput_init( varray *mappings ) {
	if( !dinput ) {
		HINSTANCE instance = GetModuleHandle(NULL);
		if( instance )
			DirectInput8Create(instance, DIRECTINPUT_VERSION, &IID_IDirectInput8, &dinput, NULL);
	}

	if( mappings ) {
		if( gctrl_dinput_mappings )
			hl_remove_root(&gctrl_dinput_mappings);
		gctrl_dinput_mappings = mappings;
		hl_add_root(&gctrl_dinput_mappings);
	}
}

static BOOL CALLBACK gctrl_dinput_deviceCb(const DIDEVICEINSTANCE *instance, void *context) {
	dx_gctrl **current = (dx_gctrl**)context;
	dx_gctrl *pad = *current;
	dx_gctrl *prev = NULL;
	LPDIRECTINPUTDEVICE8 device;
	HRESULT result;
	dinput_mapping *mapping = NULL;

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
	for( int i=0; i<gctrl_dinput_mappings->size; i++ ){
		dinput_mapping *m = hl_aptr(gctrl_dinput_mappings, dinput_mapping*)[i];
		if( instance->guidProduct.Data1 == m->guid ) {
			mapping = m;
			break;
		}
	}
	if (!mapping ) return DIENUM_CONTINUE;

	result = IDirectInput8_CreateDevice(dinput, &instance->guidInstance, &device, NULL);
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
	IDirectInputDevice8_Unacquire(pad->device);
	IDirectInputDevice8_Release(pad->device);
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
	long *p = (long*)&state;
	dinput_mapping *mapping = data->ptr->mapping;
	int *axis = hl_aptr(mapping->axis, int);

	HRESULT result = IDirectInputDevice8_GetDeviceState(data->ptr->device, sizeof(DIJOYSTATE2), &state);
	if( result == DIERR_INPUTLOST || result == DIERR_NOTACQUIRED ) {
		IDirectInputDevice8_Acquire(data->ptr->device);
		result = IDirectInputDevice8_GetDeviceState(data->ptr->device, sizeof(DIJOYSTATE2), &state);
	}
	if( result != DI_OK )
		return;
	
	if( mapping->axis->size > 0 ) data->lx = (double)((*(p + axis[0])) / 65535.) * 2 - 1.;
	if( mapping->axis->size > 1 ) data->ly = (double)((*(p + axis[1])) / 65535.) * -2 + 1.;
	if( mapping->axis->size > 2 ) data->rx = (double)((*(p + axis[2])) / 65535.) * 2 - 1.;
	if( mapping->axis->size > 3 ) data->ry = (double)((*(p + axis[3])) / 65535.) * -2 + 1.;
	if( mapping->axis->size > 4 ) data->lt = (double)((*(p + axis[4])) / 65535.);
	if( mapping->axis->size > 5 ) data->rt = (double)((*(p + axis[5])) / 65535.);

	data->buttons = 0;
	for( int i = 0; i < mapping->button->size; i++ ) {
		dinput_mapping_btn *bmap = hl_aptr(mapping->button, dinput_mapping_btn*)[i];
		if( bmap->mask == 0 ) {
			data->buttons |= (state.rgbButtons[bmap->num] > 0 ? 1 : 0) << i;
		} else {
			data->buttons |= ((gctrl_dinput_translatePOV(state.rgdwPOV[bmap->num])&bmap->mask) > 0 ? 1 : 0) << i;
		}
	}
}

// 

HL_PRIM void HL_NAME(gctrl_init)( varray *mappings ) {
	gctrl_dinput_init( mappings );
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
	if( dinput )
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
DEFINE_PRIM(_VOID, gctrl_init, _ARR);
DEFINE_PRIM(_VOID, gctrl_detect, _FUN(_VOID, TGAMECTRL _BYTES));
DEFINE_PRIM(_VOID, gctrl_update, _OBJ(TGAMECTRL _STRING _I32 _F64 _F64 _F64 _F64 _F64 _F64));
DEFINE_PRIM(_VOID, gctrl_set_vibration, TGAMECTRL _F64);