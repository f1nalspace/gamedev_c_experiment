#pragma once

#include "engine_types.h"
#include "engine_math.h"

struct ButtonState {
	S32 halfTransitionCount;
	B32 endedDown;
};

struct KeyboardState {
	union {
		ButtonState space;
		ButtonState functionkeys[8];
	};
	ButtonState keys[9];
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
	KeyboardState keyboard;
	MouseState mouse;
};

inline B32 WasDown(const ButtonState &state)
{
	B32 result = ((state.halfTransitionCount > 1) || ((state.halfTransitionCount == 1) && (state.endedDown)));
	return(result);
}
inline B32 IsDown(const ButtonState &state)
{
	B32 result = state.endedDown;
	return(result);
}