package dx;

private typedef GameControllerPtr = hl.Abstract<"dx_gctrl">;

enum GameControllerButton {
	Btn_DPadUp;
	Btn_DPadDown;
	Btn_DPadLeft;
	Btn_DPadRight;
	Btn_Start;
	Btn_Back;
	Btn_LeftStick;
	Btn_RightStick;
	Btn_LB;
	Btn_RB;
	Btn_A;
	Btn_B;
	Btn_X;
	Btn_Y;
}

@:keep @:hlNative("directx")
class GameController {
	
	public static function init(){
		gctrlInit();
	}
	
	public static function detect( onDetect : GameControllerPtr -> hl.Bytes -> Void ){
		gctrlDetect(onDetect);
	}

	
	public var ptr(default,null) : GameControllerPtr;
	public var name(default,null) : String;
	public var buttons : haxe.EnumFlags<GameControllerButton>;
	public var lx : Float;
	public var ly : Float;
	public var rx : Float;
	public var ry : Float;
	public var lt : Float;
	public var rt : Float;
	
	public function new( ptr : GameControllerPtr, name : hl.Bytes ){
		this.ptr = ptr;
		this.name = @:privateAccess String.fromUCS2(name);
	}
	
	public inline function update(){
		gctrlUpdate(this);
	}
	
	public inline function setVibration( strength : Float ){
		gctrlSetVibration(ptr,strength);
	}
		
	static function gctrlInit(){}
	static function gctrlDetect( onDetect : GameControllerPtr -> hl.Bytes -> Void ){}
	static function gctrlUpdate( pad : GameController ){}
	static function gctrlSetVibration( ptr : GameControllerPtr, strength : Float ){}
	
}
