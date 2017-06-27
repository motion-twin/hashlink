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
	
	public static function detect( onDetect : GameControllerPtr -> Bool -> Void ){
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
	
	public function new( ptr : GameControllerPtr ){
		this.ptr = ptr;
		this.name = @:privateAccess String.fromUTF8(gctrlName(ptr));
	}
	
	public function update(){
		gctrlUpdate(this);
	}
	
	static function gctrlInit(){}
	static function gctrlDetect( onDetect : GameControllerPtr -> Bool -> Void ){}
	static function gctrlUpdate( pad : GameController ){}
	static function gctrlName( ptr : GameControllerPtr ) : hl.Bytes { return null; }
	
}
