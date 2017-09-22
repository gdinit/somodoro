/* en_break_state.h */
// ===================================80 chars=================================|

#ifndef EN_BREAK_STATE_H
#define EN_BREAK_STATE_H

#include "en_essentials.h"
#include "en_state.h"
#include "en_state_machine.h"
#include "en_main_menu_state.h"
// Required for centerOrigin
#include "en_util_utility.h"
#include "en_util_make_unique.h"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Audio.hpp>
// ===ImGui Stuff ==============================================================
#include <imgui.h>
#include <imgui-SFML.h>
// =============================================================================

// to in-line document
// Required for std::unique_ptr
#include <memory>
// Required for debugging -- TODO REMOVE THIS
#include <iostream>
// Required for capturing last windowResizeEvent
#include <chrono>
// Required for ceil
#include <cmath>

#include <ctime>
#include <ratio>
#include <chrono>

// Required for JSON
#include "3rd-party/json-nlohmann/v.2.1.1/json.hpp"
// Required for JSON
#include <fstream>
// To sleep & reduce CPU usage
#include <thread>

class StateMachine;

class BreakState : public State
{
public:
	BreakState( StateMachine &machine
	, sf::RenderWindow &window
	, EngineSharedContext &context
	, bool replace = true );
	virtual ~BreakState();
	void	processEvents();
	void	update();
	void	draw();
	void	pause();
	void	resume();
	void	makeWindowAlwaysOnTop();
	void	calculateUpdateTimer();
	void	updateText();
	void	centerText();
	void	loadSounds();
	void	playSoundIfRequested();
	void	playSoundClicked();
	void	playSoundWindingUp();
	void	playSoundChime();
	void	winSizeIncrease( int times );
	void	winSizeDecrease( int times );
	void	winManualToggleMoveable();
	void	winAutoToggleMoveableIfNecessary();
	void	winAutoResizeIfRequested();

private:
	t_objectName	m_myObjNameStr;
	sf::Vector2u	m_windowSize;
	// TODO move this to app
	float
			m_desiredAspectRatio;
	// ===ImGui Stuff ======================================================
	sf::Clock	m_deltaClock;
	// =====================================================================
	sf::Vector2i	m_grabbedOffset;
	bool		m_grabbedWindow = false;
	bool		m_windowActive = true;
	// This should be overwritten by JSON parser
	sf::Color	m_breakshortBgColor = sf::Color::Yellow;
	int32		m_breakshortBgColorR = 0;
	int32		m_breakshortBgColorG = 0;
	int32		m_breakshortBgColorB = 0;
	sf::Color	m_breaklongBgColor = sf::Color::Yellow;
	int32		m_breaklongBgColorR = 0;
	int32		m_breaklongBgColorG = 0;
	int32		m_breaklongBgColorB = 0;
	int32		m_secsBreakshort = 0;
	int32		m_secsBreaklong = 0;
	int32		m_countdownSecondsRemaining = 0;
	sf::Font	m_breakFont;
	sf::Text	m_breakText;
	sf::Font	m_breaklongFont;
	sf::Text	m_breaklongText;
	stdTimePoint	m_TPstart;
	stdTimePoint	m_TPlatest;
	bool		m_centerOriginNeeded = true;
	bool		m_timerLive = false;
	// SOUNDS
	sf::SoundBuffer	m_sbClicked;
	sf::Sound	m_sClicked;
	sf::SoundBuffer	m_sbWindingUp;
	sf::Sound	m_sWindingUp;
	sf::SoundBuffer	m_sbChime;
	sf::Sound	m_sChime;
	int32		m_winPosX = 30;
	int32		m_winPosY = 74;
	int32		m_fontSizePxBreakshort = -1;
	int32		m_fontSizePxBreaklong = -1;
	int32		m_breakType;
};

// EN_BREAK_STATE_H
#endif

// ===================================80 chars=================================|
/* EOF */

