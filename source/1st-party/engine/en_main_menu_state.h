/* en_main_menu_state.h */
// ===================================80 chars=================================|

#ifndef EN_MAIN_MENU_STATE_H
#define EN_MAIN_MENU_STATE_H

#include "en_essentials.h"
#include "en_state.h"
#include "en_state_machine.h"
// For centerOrigin
#include "en_util_utility.h"
#include "en_countdown_state.h"
#include "en_break_state.h"
#include "en_breaklong_state.h"
// Required for JSON
#include "3rd-party/json-nlohmann/v.2.1.1/json.hpp"
// Required for JSON
#include <fstream>

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Window/Mouse.hpp>
#include <imgui.h>
#include <imgui-SFML.h>

// for FPS text
#include <string>
// For round()
#include <cmath>

class StateMachine;

class MainMenuState : public State
{
public:
	MainMenuState( StateMachine &machine
	, sf::RenderWindow &window
	, EngineSharedContext &context
	, bool replace = true );
	virtual ~MainMenuState();
	// resize stuff here
	void	processEvents();
	void	update();
	void	draw();
	void	pause();
	void	resume();
	void	makeWindowAlwaysOnTop();
	void	winSizeIncrease( int times );
	void	winSizeDecrease( int times );
	void	winManualToggleMoveable();
	void	winAutoToggleMoveableIfNecessary();
	void	readSettings();

private:
	const t_objectName	m_myObjNameStr;
	sf::Vector2u		m_windowSize;
	// TODO move this to app
	float			m_desiredAspectRatio;
	// ImGui Stuff
	// TODO clean-up this. imgui image button tests.
	sf::Clock		m_deltaClock;
	sf::Texture		m_tex1Start;
	sf::Texture		m_tex2Short;
	sf::Texture		m_tex3Long;
	sf::Vector2f		m_position = { 0, 0 };
	sf::Vector2f		m_dimension = { 0, 0 };
	//
	sf::Color		m_bgColor;
	sf::Color		m_tintColor;
	// pomodoro stuff
	sf::Vector2i		m_grabbedOffset;
	bool			m_grabbedWindow = false;
	bool			m_windowActive = true;
	// This should be overwritten by JSON parser
	sf::Color		m_mainmenuBgColor;
	int			m_mainmenuBgColorR;
	int			m_mainmenuBgColorG;
	int			m_mainmenuBgColorB;
	// SOUNDS
	int32			m_winPosX = 30;
	int32			m_winPosY = 74;
};

// EN_MAIN_MENU_STATE_H
#endif

// ===================================80 chars=================================|
/* EOF */

