#include "SDL_RWops.h"

#include "Player.h"
#include "MathUtil.h"
#include "Path.h"
#include "Log.h"
#include "Error.h"
#include "Level.h"
#include "Game.h"
#include "GameConstants.h"

//animation data
const uint8_t animationWalk[] =			{0xFF,0x08,0x09,0x0A,0x0B,0x06,0x07,0xFF};
const uint8_t animationRun[] =			{0xFF,0x1E,0x1F,0x20,0x21,0xFF,0xFF,0xFF};
const uint8_t animationRoll[] =			{0xFE,0x2E,0x2F,0x30,0x31,0x32,0xFF,0xFF};
const uint8_t animationRoll2[] =		{0xFE,0x2E,0x2F,0x32,0x30,0x31,0x32,0xFF};
const uint8_t animationPush[] =			{0xFD,0x45,0x46,0x47,0x48,0xFF,0xFF,0xFF};
const uint8_t animationIdle[] =			{0x17,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x03,0x02,0x02,0x02,0x03,0x04,0xFE,0x02};
const uint8_t animationBalance[] =		{0x1F,0x3A,0x3B,0xFF};
const uint8_t animationLookUp[] =		{0x3F,0x05,0xFF};
const uint8_t animationDuck[] =			{0x3F,0x39,0xFF};
const uint8_t animationSpindash[] =		{0x00,0x58,0x59,0x58,0x5A,0x58,0x5B,0x58,0x5C,0x58,0x5D,0xFF};
const uint8_t animationWarp1[] =		{0x3F,0x33,0xFF};
const uint8_t animationWarp2[] =		{0x3F,0x34,0xFF};
const uint8_t animationWarp3[] =		{0x3F,0x35,0xFF};
const uint8_t animationWarp4[] =		{0x3F,0x36,0xFF};
const uint8_t animationSkid[] =			{0x07,0x37,0x38,0xFF};
const uint8_t animationFloat1[] =		{0x07,0x3C,0x3F,0xFF};
const uint8_t animationFloat2[] =		{0x07,0x3C,0x3D,0x53,0x3E,0x54,0xFF};
const uint8_t animationSpring[] =		{0x2F,0x40,0xFD,0x00};
const uint8_t animationHang[] =			{0x04,0x41,0x42,0xFF};
const uint8_t animationLeap1[] =		{0x0F,0x43,0x43,0x43,0xFE,0x01};
const uint8_t animationLeap2[] =		{0x0F,0x43,0x44,0xFE,0x01};
const uint8_t animationSurf[] =			{0x3F,0x49,0xFF};
const uint8_t animationGetAir[] =		{0x0B,0x56,0x56,0x0A,0x0B,0xFD,0x00};
const uint8_t animationBurnt[] =		{0x20,0x4B,0xFF};
const uint8_t animationDrown[] =		{0x20,0x4C,0xFF};
const uint8_t animationDeath[] =		{0x03,0x4D,0xFF};
const uint8_t animationShrink[] =		{0x03,0x4E,0x4F,0x50,0x51,0x52,0x00,0xFE,0x01};
const uint8_t animationHurt[] =			{0x03,0x55,0xFF};
const uint8_t animationWaterSlide[] =	{0x07,0x55,0x57,0xFF};
const uint8_t animationNull[] =			{0x77,0x00,0xFD,0x00};
const uint8_t animationFloat3[] =		{0x03,0x3C,0x3D,0x53,0x3E,0x54,0xFF};
const uint8_t animationFloat4[] =		{0x03,0x3C,0xFD,0x00};

const uint8_t* animationList[] = {
	animationWalk,
	animationRun,
	animationRoll,
	animationRoll2,
	animationPush,
	animationIdle,
	animationBalance,
	animationLookUp,
	animationDuck,
	animationSpindash,
	animationWarp1,
	animationWarp2,
	animationWarp3,
	animationWarp4,
	animationSkid,
	animationFloat1,
	animationFloat2,
	animationSpring,
	animationHang,
	animationLeap1,
	animationLeap2,
	animationSurf,
	animationGetAir,
	animationBurnt,
	animationDrown,
	animationDeath,
	animationShrink,
	animationHurt,
	animationWaterSlide,
	animationNull,
	animationFloat3,
	animationFloat4,
};

PLAYER::PLAYER(const char *specPath, PLAYER *myFollow, int myController)
{
	LOG(("Creating a player with spec %s and controlled by controller %d...\n", specPath, myController));
	memset(this, 0, sizeof(PLAYER));
	
	//Equivalent of routine 0
	routine = PLAYERROUTINE_CONTROL;
	
	//Load art and mappings
	GET_APPEND_PATH(texPath, specPath, ".bmp");
	GET_APPEND_PATH(mapPath, specPath, ".map");
	
	texture = new TEXTURE(texPath);
	if (texture->fail != NULL)
	{
		Error(fail = texture->fail);
		return;
	}
	
	mappings = new MAPPINGS(mapPath);
	if (mappings->fail != NULL)
	{
		delete texture; //Free texture before erroring
		Error(fail = mappings->fail);
		return;
	}
	
	//Read properties from the specifications
	GET_APPEND_PATH(plrSpecPath, specPath, ".psp");
	SDL_RWops *playerSpec = SDL_RWFromFile(plrSpecPath, "rb");
	
	if (playerSpec == NULL)
	{
		delete texture; //Free texture and mappings before erroring
		delete mappings;
		Error(fail = SDL_GetError());
		return;
	}
	
	xRadius = SDL_ReadU8(playerSpec);
	yRadius = SDL_ReadU8(playerSpec);
	
	defaultXRadius = xRadius;
	defaultYRadius = yRadius;
	rollXRadius = SDL_ReadU8(playerSpec);
	rollYRadius = SDL_ReadU8(playerSpec);
	
	characterType = (CHARACTERTYPE)SDL_ReadU8(playerSpec);
	
	//Close file
	SDL_RWclose(playerSpec);
	
	//Set render properties
	priority = 2;
	widthPixels = 0x18;
	
	//Render flags
	renderFlags.xFlip = false;
	renderFlags.yFlip = false;
	renderFlags.alignPlane = true;
	renderFlags.alignBackground = false;
	renderFlags.assumePixelHeight = false;
	renderFlags.bit5 = false;
	renderFlags.bit6 = false;
	renderFlags.onScreen = false;
	
	//Initialize our speeds
	top = 0x0600;
	acceleration = 0x000C;
	deceleration = 0x0080;
	
	//Collision
	topSolidLayer = COLLISIONLAYER_NORMAL_TOP;
	lrbSolidLayer = COLLISIONLAYER_NORMAL_LRB;
	
	//Flipping stuff
	flipsRemaining = 0;
	flipSpeed = 4;
	
	super = false;
	
	//Set our following person
	follow = (void*)myFollow;
	
	//Set the controller to use
	controller = myController;
	
	//Initialize our record arrays
	for (int i = 0; i < PLAYER_RECORD_LENGTH; i++)
	{
		posRecord[i].x = x.pos - 0x20;	//Why the hell is the position offset?
		posRecord[i].y = x.pos - 0x04;
		memset(&statRecord[i], 0, sizeof(statRecord[0])); //Stat record is initialized as 0
	}
	
	recordPos = 0;
	
	LOG(("Success!\n"));
}

PLAYER::~PLAYER()
{
	if (texture)
		delete texture;
	if (mappings)
		delete mappings;
}

//Record our position in the records
void PLAYER::RecordPos()
{
	//Set the records at the current position
	posRecord[recordPos] = {x.pos, y.pos};
	statRecord[recordPos] = {controlHeld, controlPress, status};
	
	//Increment record position
	recordPos++;
	recordPos %= PLAYER_RECORD_LENGTH;
}

//Spindash code
const uint16_t spindashSpeed[9] =		{0x800, 0x880, 0x900, 0x980, 0xA00, 0xA80, 0xB00, 0xB80, 0xC00};
const uint16_t spindashSpeedSuper[9] =	{0xB00, 0xB80, 0xC00, 0xC80, 0xD00, 0xD80, 0xE00, 0xE80, 0xF00};
		
bool PLAYER::Spindash()
{
	if (spindashing == false)
	{
		//We must be ducking in order to spindash
		if (anim != PLAYERANIMATION_DUCK)
			return true;
		
		//Initiate spindash
		if (controlPress.a || controlPress.b || controlPress.c)
		{
			//Play animation and sound
			anim = PLAYERANIMATION_SPINDASH;
			//PlaySound(SOUNDID_SPINDASH_REV);
			
			//Set spindash variables
			spindashing = true;
			spindashCounter = 0;
			
			//Make our spindash dust visible
			//spindashDust.anim = 1; //The original uses the dust object for splashing too, we're not doing that
		}
		else
		{
			return true;
		}
	}
	//Release spindash
	else if (!controlHeld.down)
	{
		//Begin rolling
		xRadius = rollXRadius;
		yRadius = rollYRadius;
		anim = PLAYERANIMATION_ROLL;
		
		//Offset our position to line up with the ground
		if (status.reverseGravity)
			y.pos -= 5;
		else
			y.pos += 5;
		
		//Release spindash
		spindashing = false;
		
		//Set our speed
		if (super)
			inertia = spindashSpeedSuper[spindashCounter / 0x100];
		else
			inertia = spindashSpeed[spindashCounter / 0x100];
		
		//Lock the camera behind us
		scrollDelay = (0x2000 - (inertia - 0x800) * 2) % (PLAYER_RECORD_LENGTH << 8);
		
		//Revert if facing left
		if (status.xFlip)
			inertia = -inertia;
		
		//Actually go into the roll routine
		status.inBall = true;
		//spindashDust.anim = 0;
		//PlaySound(SOUNDID_SPINDASH_RELEASE);
	}
	//Charging spindash
	else
	{
		//Reduce our spindash counter
		if (spindashCounter != 0)
		{
			uint16_t nextCounter = (spindashCounter - spindashCounter / 0x20);
			
			//The original makes sure the spindash counter is 0 if it underflows (which seems to be impossible, to my knowledge)
			if (nextCounter <= spindashCounter)
				spindashCounter -= spindashCounter / 0x20;
			else
				spindashCounter = 0;
		}
		
		//Rev our spindash
		if (controlPress.a || controlPress.b || controlPress.c)
		{
			//Restart the spindash animation and play the rev sound
			anim = PLAYERANIMATION_SPINDASH;
			nextAnim = PLAYERANIMATION_WALK;
			//PlaySound(SOUNDID_SPINDASH_REV);
			
			//Increase our spindash counter
			spindashCounter += 0x200;
			if (spindashCounter >= 0x800)
				spindashCounter = 0x800;
		}
	}
	
	//Collide with the level (S3K has code to crush you against the foreground layer from the background here)
	LevelBound();
	//AnglePos();
	return false;
}

//Jump ability functions
void PLAYER::JumpAbilities()
{
	if (jumpAbility == 0 && (controlPress.a || controlPress.b || controlPress.c))
	{
		//Perform our ability
		//status.rollJumping = false; //Also commented out because this would be awkward without any abilities
		
		//No ability code yet
		
		jumpAbility = 1;
	}
}

//Jumping functions
void PLAYER::JumpHeight()
{
	if (status.jumping)
	{
		//Slow us down if ABC is released when jumping
		int16_t minVelocity = status.underwater ? -0x200 : -0x400;
		
		if (minVelocity <= yVel)
			JumpAbilities();
		else if (!controlHeld.a && !controlHeld.b && !controlHeld.c)
			yVel = minVelocity;
	}
	else
	{
		//Cap our upwards velocity
		if (!status.pinballMode && yVel < -0xFC0)
			yVel = -0xFC0;
	}
}

void PLAYER::ChgJumpDir()
{
	//Move left and right
	if (!status.rollJumping)
	{
		int16_t newVelocity = xVel;
		
		//Move left if left is held
		if (controlHeld.left)
		{
			//Accelerate left
			status.xFlip = true;
			newVelocity -= acceleration * 2;
			
			//Don't accelerate past the top speed
			if (newVelocity <= -top)
			{
				newVelocity += acceleration * 2;
				if (newVelocity >= -top)
					newVelocity = -top;
			}
		}
		
		//Move right if right is held
		if (controlHeld.right)
		{
			//Accelerate right
			status.xFlip = false;
			newVelocity += acceleration * 2;
			
			//Don't accelerate past the top speed
			if (newVelocity >= top)
			{
				newVelocity -= acceleration * 2;
				if (newVelocity <= top)
					newVelocity = top;
			}
		}
		
		//Copy our new velocity to our velocity
		xVel = newVelocity;
	}
	
	//Air drag
	if (yVel >= -0x400 && yVel < 0)
	{
		int16_t drag = xVel / 0x20;
		
		if (drag > 0)
		{
			xVel -= drag;
			if (xVel < 0)
				xVel = 0;
		}
		else
		{
			xVel -= drag;
			if (xVel >= 0)
				xVel = 0;
		}
	}
}

void PLAYER::JumpAngle()
{
	//Bring our angle down back upwards
	if (angle != 0)
	{
		if (angle >= 0x80)
		{
			angle += 2;
			if (angle < 0x80)
				angle = 0;
		}
		else
		{
			angle -= 2;
			if (angle >= 0x80)
				angle = 0;
		}
	}
	
	//Handle our flipping
	uint8_t nextFlipAngle = flipAngle;
	
	if (nextFlipAngle != 0)
	{
		if (inertia >= 0 || flipTurned)
		{
			if (nextFlipAngle >= 0x100 - flipSpeed && flipsRemaining-- == 0)
			{
				flipsRemaining = 0;
				nextFlipAngle = 0;
			}
			else
			{
				nextFlipAngle += flipSpeed;
			}
		}
		else
		{
			if (nextFlipAngle < flipSpeed && flipsRemaining-- == 0)
			{
				flipsRemaining = 0;
				nextFlipAngle = 0;
			}
			else
			{
				nextFlipAngle -= flipSpeed;
			}
		}
		
		flipAngle = nextFlipAngle;
	}
}

bool PLAYER::Jump()
{
	if (controlPress.a || controlPress.b || controlPress.c)
	{
		//Get the angle of our head
		uint8_t headAngle = angle;
		if (status.reverseGravity)
			headAngle = (~(headAngle + 0x40)) - 0x40;
		headAngle -= 0x80;
		
		//Don't jump if under a low ceiling
		//if (CalcRoomOverHead(headAngle) >= 6)
		{
			//Get the velocity of our jump
			int16_t jumpVelocity;
			
			if (characterType != CHARACTERTYPE_KNUCKLES)
			{
				//Normal jumping
				jumpVelocity = super ? 0x800 : 0x680;
				
				//Lower our jump in water
				if (status.underwater)
					jumpVelocity = 0x380;
			}
			else
			{
				//Knuckles' low jumping
				jumpVelocity = status.underwater ? 0x300 : 0x600;
			}
			
			//Apply the velocity
			int16_t sin, cos;
			GetSine(angle - 0x40, &sin, &cos);
			
			xVel += (cos * jumpVelocity) / 0x100;
			yVel += (sin * jumpVelocity) / 0x100;
			
			//Put us in the jump state
			status.inAir = true;
			status.pushing = false;
			status.jumping = true;
			status.stickToConvex = false;
			
			//PlaySound(SOUNDID_JUMP);
			
			//Handle our collision and roll state
			xRadius = defaultXRadius;
			yRadius = defaultYRadius;
			
			if (!status.inBall)
			{
				//Go into ball form
				xRadius = rollXRadius;
				yRadius = rollYRadius;
				anim = PLAYERANIMATION_ROLL;
				status.inBall = true;
				
				//Shift us down to the ground
				if (status.reverseGravity)
					y.pos += (yRadius - defaultYRadius);
				else
					y.pos -= (yRadius - defaultYRadius);
			}
			else
			{
				//Set our roll jump flag (also we use the regular non-roll collision size for some reason)
				status.rollJumping = true;
			}
			return false;
		}
	}
	
	return true;
}

//Slope gravity related functions
void PLAYER::SlopeResist()
{
	if (((angle + 0x60) & 0xFF) < 0xC0)
	{
		//Get our slope gravity
		int16_t sin;
		GetSine(angle, &sin, NULL);
		sin = (sin * 0x20) / 0x100;
		
		//Apply our slope gravity (if our inertia is non-zero, always apply, if it is 0, apply if the force is at least 0xD units per frame)
		if (inertia != 0)
		{
			if (inertia < 0)
				inertia += sin;
			else if (sin != 0)
				inertia += sin;
		}
		else
		{
			if (abs(sin) >= 0xD)
				inertia += sin;
		}
	}
}

void PLAYER::RollRepel()
{
	if (((angle + 0x60) & 0xFF) < 0xC0)
	{
		//Get our slope gravity
		int16_t sin;
		GetSine(angle, &sin, NULL);
		sin = (sin * 0x50) / 0x100;
		
		//Apply our slope gravity (divide by 4 if opposite to our inertia sign)
		if (inertia >= 0)
		{
			if (sin < 0)
				sin /= 4;
			inertia += sin;
		}
		else
		{
			if (sin >= 0)
				sin /= 4;
			inertia += sin;
		}
	}
}

void PLAYER::SlopeRepel()
{
	if (!status.stickToConvex)
	{
		if (moveLock == 0)
		{
			//Are we on a steep enough slope and going too slow?
			if (((angle + 0x18) & 0xFF) >= 0x30 && abs(inertia) < 0x280)
			{
				//Lock our controls for 30 frames (half a second)
				moveLock = 30;
				
				//Slide down the slope, or fall off if very steep
				if (((angle + 0x30) & 0xFF) >= 0x60)
					status.inAir = true;
				else if (((angle + 0x30) & 0xFF) >= 0x30)
					inertia += 0x80;
				else
					inertia -= 0x80;
			}
		}
		else
		{
			//Decrement moveLock every frame it's non-zero
			moveLock--;
		}
	}
}

//Movement functions
void PLAYER::MoveLeft()
{
	int16_t newInertia = inertia;
	
	if (newInertia <= 0)
	{
		//Flip if not already turned around
		if (!status.xFlip)
		{
			status.xFlip = true;
			status.pushing = false;
			nextAnim = PLAYERANIMATION_RUN;
		}
		
		//Accelerate
		newInertia -= acceleration;
		
		//Don't accelerate past the top speed
		if (newInertia <= -top)
		{
			newInertia += acceleration;
			if (newInertia >= -top)
				newInertia = -top;
		}
		
		//Set inertia and do walk animation
		inertia = newInertia;
		anim = PLAYERANIMATION_WALK;
	}
	else
	{
		//Decelerate
		newInertia -= deceleration;
		if (newInertia < 0)
			newInertia = -0x80;
		inertia = newInertia;
		
		//Do skid animation if on a floor and above 0x400 units per frame
		if (((angle + 0x20) & 0xC0) == 0 && inertia >= 0x400)
		{
			//PlaySound(SOUNDID_SKID);
			anim = PLAYERANIMATION_SKID;
			status.xFlip = false;
			
			//TODO: Create dust here
		}
	}
}

void PLAYER::MoveRight()
{
	int16_t newInertia = inertia;
	
	if (newInertia >= 0)
	{
		//Flip if not already turned around
		if (status.xFlip)
		{
			status.xFlip = false;
			status.pushing = false;
			nextAnim = PLAYERANIMATION_RUN;
		}
		
		//Accelerate
		newInertia += acceleration;
		
		//Don't accelerate past the top speed
		if (newInertia >= top)
		{
			newInertia -= acceleration;
			if (newInertia <= top)
				newInertia = top;
		}
		
		//Set inertia and do walk animation
		inertia = newInertia;
		anim = PLAYERANIMATION_WALK;
	}
	else
	{
		//Decelerate
		newInertia += deceleration;
		if (newInertia >= 0)
			newInertia = 0x80;
		inertia = newInertia;
		
		//Do skid animation if on a floor and above 0x400 units per frame
		if (((angle + 0x20) & 0xC0) == 0 && inertia <= -0x400)
		{
			//PlaySound(SOUNDID_SKID);
			anim = PLAYERANIMATION_SKID;
			status.xFlip = true;
			
			//TODO: Create dust here
		}
	}
}

void PLAYER::Move()
{
	if (!status.isSliding)
	{
		if (!moveLock)
		{
			//Move left and right
			if (controlHeld.left)
				MoveLeft();
			if (controlHeld.right)
				MoveRight();
			
			if (((angle + 0x20) & 0xC0) == 0 && inertia == 0)
			{
				//Do idle animation
				status.pushing = false;
				anim = PLAYERANIMATION_IDLE;
				
				//Look up and down
				if (controlHeld.up)
				{
					anim = PLAYERANIMATION_LOOKUP;
				}
				else if (controlHeld.down)
				{
					anim = PLAYERANIMATION_DUCK; //This is done in Roll too
				}
			}
		}
		
		//TODO: camera scrolling down to sonic
		
		//Friction
		uint16_t friction = acceleration;
		if (super)
			friction = 0x000C;
		
		if (!controlHeld.left && !controlHeld.right && inertia != 0)
		{
			if (inertia > 0)
			{
				inertia -= friction;
				if (inertia < 0)
					inertia = 0;
			}
			else
			{
				inertia += friction;
				if (inertia >= 0)
					inertia = 0;
			}
		}
	}
	
	//Convert our inertia into global speeds
	int16_t sin, cos;
	GetSine(angle, &sin, &cos);
	xVel = (cos * inertia) / 0x100;
	yVel = (sin * inertia) / 0x100;
}

//Rolling functions
void PLAYER::Roll()
{
	if (!controlHeld.left && !controlHeld.right)
	{
		if (controlHeld.down)
		{
			if (abs(inertia) >= 0x100)
			{
				//Roll
				if (!status.inBall)
				{
					//Enter ball state
					status.inBall = true;
					xRadius = rollXRadius;
					yRadius = rollYRadius;
					anim = PLAYERANIMATION_ROLL;
					
					//This is supposed to keep us on the ground, but when we're on a ceiling, it does... not that
					if (status.reverseGravity)
						y.pos -= 5;
					else
						y.pos += 5;
					
					//Play the sound
					//PlaySound(SOUNDID_ROLL);
					
					//Code that doesn't trigger (leftover from Sonic 1's S-tubes)
					if (inertia == 0)
						inertia = 0x200;
				}
			}
			else
			{
				//Duck
				anim = PLAYERANIMATION_DUCK;
			}
		}
		else if (anim == PLAYERANIMATION_DUCK)
		{
			//Revert to walk animation if was ducking
			anim = PLAYERANIMATION_WALK;
		}
	}
}

void PLAYER::RollLeft()
{
	if (inertia <= 0)
	{
		status.xFlip = true;
		anim = PLAYERANIMATION_ROLL;
	}
	else
	{
		inertia -= 0x20;
		if (inertia < 0)
			inertia = -0x80;
	}
}
	
void PLAYER::RollRight()
{
	if (inertia >= 0)
	{
		status.xFlip = false;
		anim = PLAYERANIMATION_ROLL;
	}
	else
	{
		inertia += 0x20;
		if (inertia >= 0)
			inertia = 0x80;
	}
}

void PLAYER::RollSpeed()
{
	//Get our friction (super has separate friction) and deceleration when pulling back
	uint16_t friction = acceleration / 2;
	if (super)
		friction = 6;
	
	if (!status.isSliding)
	{
		//Decelerate if pulling back
		if (!status.pinballMode && !moveLock)
		{
			if (controlHeld.left)
				RollLeft();
			if (controlHeld.right)
				RollRight();
		}
		
		//Friction
		if (inertia > 0)
		{
			inertia -= friction;
			if (inertia < 0)
				inertia = 0;
		}
		else if (inertia < 0)
		{
			inertia += friction;
			if (inertia >= 0)
				inertia = 0;
		}
		
		//Stop if slowed down
		if (abs(inertia) < 0x80)
		{
			if (!status.pinballMode)
			{
				//Exit ball form
				status.inBall = false;
				xRadius = defaultXRadius;
				yRadius = defaultYRadius;
				anim = PLAYERANIMATION_IDLE;
				y.pos -= 5;
			}
			else
			{
				//Speed us back up if we slow down
				if (status.xFlip)
					inertia = -0x400;
				else
					inertia = 0x400;
			}
		}
	}
	
	//Convert our inertia into global speeds
	int16_t sin, cos;
	GetSine(angle, &sin, &cos);
	xVel = (cos * inertia) / 0x100;
	yVel = (sin * inertia) / 0x100;
	
	//Cap our global horizontal speed for some reason
	if (xVel <= -0x1000)
		xVel = -0x1000;
	if (xVel >= 0x1000)
		xVel = 0x1000;
}

//Level boundary function
void PLAYER::LevelBoundSide(uint16_t bound)
{
	//Set our position to the boundary
	x.pos = bound;
	x.sub = 0;
	
	//Set our speed to 0
	xVel = 0;
	inertia = 0;
}

void PLAYER::LevelBound()
{
	//Get our next position and boundaries
	uint16_t nextPos = (xPosLong + (xVel * 0x100)) / 0x10000;
	uint16_t leftBound = gLevelLeftBoundary + 0x10;
	uint16_t rightBound = gLevelRightBoundary - 0x18; //For some reason the right boundary is 8 pixels greater than the left boundary
	
	//Clip us into the boundaries
	if (nextPos < leftBound)
		LevelBoundSide(leftBound);
	else if (nextPos > rightBound)
		LevelBoundSide(rightBound);
}

//Animation update
void PLAYER::AdvanceFrame(const uint8_t* animation)
{
	//Advance our frame
	uint8_t frame = animation[1 + animFrame];
	
	//Handle specific commands
	if (frame >= 0x80)
	{
		switch (frame)
		{
			case 0xFF:
				animFrame = 0;
				frame = animation[1];
				break;
			case 0xFE:
				animFrame -= animation[2 + animFrame];
				frame = animation[1 + animFrame];
				break;
			case 0xFD:
				anim = (PLAYERANIMATION)animation[2 + animFrame];
				return;
			default:
				return;
		}
	}

	mappingFrame = frame;
	++animFrame;
}

void PLAYER::Animate()
{
	//If next animation is not equal to our current animation, switch
	if (anim != nextAnim)
	{
		nextAnim = anim;
		animFrame = 0;
		animFrameDuration = 0;
		status.pushing = false;
	}
	
	//Handle our animation
	const uint8_t** myAnimationList = animationList;
	const uint8_t* animation = myAnimationList[anim];
	
	//If our first byte is a frame duration
	if (animation[0] < 0x80)
	{
		//Copy our flip
		renderFlags.xFlip = status.xFlip;
		renderFlags.yFlip = status.reverseGravity;
		
		if (--animFrameDuration >= 0)
			return;
		
		animFrameDuration = animation[0];
		AdvanceFrame(animation);
	}
	else
	{
		//Wait until we should advance frame
		if (--animFrameDuration >= 0)
			return;
		
		if (animation[0] == 0xFF)
		{
			//Walk animation
			if (flipAngle == 0)
			{
				uint8_t offAngle = angle;
				if (offAngle < 0x80)
					--offAngle;
				
				if (!status.xFlip)
					offAngle = ~offAngle;
				
				offAngle += 0x10;
				
				//Get our horizontal flip, and flip according to angle
				renderFlags.xFlip = status.xFlip;
				
				if (offAngle >= 0x80)
				{
					renderFlags.xFlip = !renderFlags.xFlip;
					renderFlags.yFlip = !status.reverseGravity;
				}
				else
				{
					renderFlags.yFlip = status.reverseGravity;
				}
				
				//Walking, not pushing
				if (!status.pushing)
				{
					//Convert our angle down to 45 degree intervals
					offAngle /= 0x20;
					offAngle &= 3;
					
					//Get our animation speed
					uint16_t speed = abs(inertia);
					if (status.isSliding)
						speed *= 2;
					
					if (!super)
					{
						//Get which animation to use
						const uint8_t *walkAnimation = myAnimationList[PLAYERANIMATION_RUN];
						if (speed < 0x600)
						{
							walkAnimation = myAnimationList[PLAYERANIMATION_WALK];
							offAngle *= 6;
						}
						else
						{
							offAngle *= 4;
						}
						
						//Apply our animation speed
						speed = -speed + 0x800;

						if (speed >= 0x8000)
							speed = 0;
						
						animFrameDuration = speed / 0x100;
						
						//Advance frame and offset by offAngle
						AdvanceFrame(walkAnimation);
						mappingFrame += offAngle;
						return;
					}
					else
					{
						//Get which animation to use
						const uint8_t *walkAnimation = myAnimationList[PLAYERANIMATION_RUN];
						if (speed < 0x800)
						{
							walkAnimation = myAnimationList[PLAYERANIMATION_WALK];
							offAngle *= 8;
						}
						
						//Apply our animation speed
						speed = -speed + 0x800;

						if (speed >= 0x8000)
							speed = 0;
						
						animFrameDuration = speed / 0x100;
						
						//Advance frame and offset by offAngle
						AdvanceFrame(walkAnimation);
						mappingFrame += offAngle;
						return;
					}
				}
				else
				{
					if (--animFrameDuration >= 0)
						return;
					
					//Set our animation speed
					animFrameDuration = -abs(inertia) + 0x800 >= 0x8000 ? 0 : (-abs(inertia) + 0x800) / 0x40;
					animation = myAnimationList[PLAYERANIMATION_PUSH];
					renderFlags.xFlip = status.xFlip;
					renderFlags.yFlip = status.reverseGravity;
					
					AdvanceFrame(animation);
				}
			}
			else
			{
				return;
			}
		}
		else
		{
			if (--animFrameDuration >= 0)
				return;
			
			//Roll animation
			if (animation[0] == 0xFE)
			{
				//Do our rolling animation (ROLL2 is a faster roll animation)
				if (abs(inertia) >= 0x600)
					animation = myAnimationList[PLAYERANIMATION_ROLL2];
				else
					animation = myAnimationList[PLAYERANIMATION_ROLL];
				
				//Set our animation speed
				uint16_t speed = -abs(inertia) + 0x400;
				if (speed >= 0x8000)
					speed = 0;
				animFrameDuration = speed / 0x100;
				
				//Copy our flip
				renderFlags.xFlip = status.xFlip;
				renderFlags.yFlip = status.reverseGravity;
			}
			else //Push animation
			{
				if (--animFrameDuration >= 0)
					return;
				
				//Set our animation speed
				animFrameDuration = -abs(inertia) + 0x800 >= 0x8000 ? 0 : (-abs(inertia) + 0x800) / 0x40;
				animation = myAnimationList[PLAYERANIMATION_PUSH];
				renderFlags.xFlip = status.xFlip;
				renderFlags.yFlip = status.reverseGravity;
			}
			
			AdvanceFrame(animation);
		}
	}
}

//Update
void PLAYER::Update()
{
	if (debug)
	{
		//Debug mode (UNIMPLEMENTED)
		debug = false;
	}
	else
	{
		//Run code
		switch (routine)
		{
			case PLAYERROUTINE_CONTROL:
				
				//Copy the given controller's inputs if not locked
				if (!controlLock)
				{
					controlHeld = gController[controller].held;
					controlPress = gController[controller].press;
				}
				
				if (objectControl.disableOurMovement)
				{
					//Enable our jump abilities
					jumpAbility = 0;
				}
				else
				{
					//The original uses the two bits for a jump table, but we can't do that because it'd be horrible
					
					//Standing / walking on ground
					if (status.inBall == false && status.inAir == false)
					{
						if (Spindash() && Jump())
						{
							//Handle slope gravity and our movement
							SlopeResist();
							Move();
							Roll();
							
							//Keep us in level bounds
							LevelBound();
							
							//Move according to our velocity
							xPosLong += xVel * 0x100;
							if (status.reverseGravity)
								yPosLong -= yVel * 0x100;
							else
								yPosLong += yVel * 0x100;
							
							//Handle collision and falling off of slopes
							//AnglePos();
							SlopeRepel();
						}
					}
					//In mid-air, falling
					else if (status.inBall == false && status.inAir == true)
					{
						//Handle our movement
						JumpHeight();
						ChgJumpDir();
						
						//Keep us in level bounds
						LevelBound();
						
						//Move according to our velocity
						xPosLong += xVel * 0x100;
						if (status.reverseGravity)
							yPosLong -= yVel * 0x100;
						else
							yPosLong += yVel * 0x100;
						
						//Gravity (0x38 above water, 0x10 below water)
						yVel += 0x38;
						if (status.underwater)
							yVel -= 0x28;
						
						//Handle our angle receding when we run / jump off of a ledge
						JumpAngle();
						
						//Handle collision
						//DoLevelCollision();
					}
					//Rolling on the ground
					else if (status.inBall == true && status.inAir == false)
					{
						if (status.pinballMode || Jump())
						{
							//Handle slope gravity and our movement
							RollRepel();
							RollSpeed();
							
							//Keep us in level bounds
							LevelBound();
							
							//Move according to our velocity
							xPosLong += xVel * 0x100;
							if (status.reverseGravity)
								yPosLong -= yVel * 0x100;
							else
								yPosLong += yVel * 0x100;
							
							//Handle collision and falling off of slopes
							//AnglePos();
							SlopeRepel();
						}
					}
					//Jumping or rolled off of a ledge
					else if (status.inBall == true && status.inAir == true)
					{
						//Handle our movement
						JumpHeight();
						ChgJumpDir();
						
						//Keep us in level bounds
						LevelBound();
						
						//Move according to our velocity
						xPosLong += xVel * 0x100;
						if (status.reverseGravity)
							yPosLong -= yVel * 0x100;
						else
							yPosLong += yVel * 0x100;
						
						//Gravity (0x38 above water, 0x10 below water)
						yVel += 0x38;
						if (status.underwater)
							yVel -= 0x28;
						
						//Handle our angle receding when we run / jump off of a ledge
						JumpAngle();
						
						//Handle collision
						//DoLevelCollision();
					}
				}
				
				//TODO: Level wrapping
				//bsr.s	Sonic_Display
				//bsr.w	SonicKnux_SuperHyper
				RecordPos();
				//bsr.w	Sonic_Water
				
				Animate();
				break;
		}
	}
}

//Draw our player
void PLAYER::Draw()
{
	//Draw the player sprite
	SDL_Rect *mapRect = &mappings->rect[mappingFrame];
	SDL_Point *mapOrig = &mappings->origin[mappingFrame];
	
	int origX = mapOrig->x;
	int origY = mapOrig->y;
	if (renderFlags.xFlip)
		origX = mapRect->w - origX;
	if (renderFlags.yFlip)
		origY = mapRect->h - origY;
	
	texture->Draw(texture->loadedPalette, mapRect, x.pos - origX - gLevel->camera->x, y.pos - origY - gLevel->camera->y, renderFlags.xFlip, renderFlags.yFlip);
}
