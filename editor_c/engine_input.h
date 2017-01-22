#pragma once

#include "engine_types.h"
#include "engine_math.h"

struct ButtonState {
	S32 halfTransitionCount;
	B32 endedDown;
};

struct KeyboardState {
	union {
		ButtonState keys[13];
		struct {
			ButtonState moveUp;
			ButtonState moveDown;
			ButtonState moveLeft;
			ButtonState moveRight;
			ButtonState functionkeys[8];
			ButtonState space;
		};
	};
};

enum MouseButton {
	MouseButton_Left,
	MouseButton_Middle,
	MouseButton_Right,
	MouseButton_Extended0,
	MouseButton_Extended1,

	MouseButton_Count,
};

struct MouseState {
	struct {
		union {
			ButtonState left;
			ButtonState middle;
			ButtonState right;
			ButtonState ext0;
			ButtonState ext1;
		};
		ButtonState buttons[MouseButton_Count];
	};
	Vec2f mousePos;
	F32 wheelDelta;
};

struct InputState {
	F32 deltaTime;
	KeyboardState keyboard;
	MouseState mouse;
};

inline B32 InputButtonWasDown(const ButtonState &state)
{
	B32 result = ((state.halfTransitionCount > 1) || ((state.halfTransitionCount == 1) && (state.endedDown)));
	return(result);
}
inline B32 InputButtonIsDown(const ButtonState &state)
{
	B32 result = state.endedDown;
	return(result);
}