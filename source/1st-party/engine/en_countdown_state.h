/* en_countdown_state.h */
// ===================================80 chars=================================|

#ifndef EN_COUNTDOWN_STATE_H
#define EN_COUNTDOWN_STATE_H

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

class CountdownState : public State
{
public:
	CountdownState( StateMachine &machine
	, sf::RenderWindow &window
	, EngineSharedContext &context
	, bool replace = true );
	virtual ~CountdownState();
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
	void	readSettings();

private:
	const t_objectName	m_myObjNameStr;
	sf::Vector2u		m_windowSize;
	// ===ImGui Stuff ======================================================
	sf::Clock		m_deltaClock;
	// =====================================================================
	sf::Vector2i		m_grabbedOffset;
	bool			m_grabbedWindow = false;
	bool			m_windowActive = true;
	// This should be overwritten by JSON parser
	sf::Color		m_pomodoroBgColor = sf::Color::Yellow;
	int32			m_pomodoroBgColorR = 0;
	int32			m_pomodoroBgColorG = 0;
	int32			m_pomodoroBgColorB = 0;
	int32			m_pomodoroSecs = 0;
	int32			m_countdownSecondsRemaining = 0;
	sf::Font		m_countdownFont;
	sf::Text		m_countdownText;
	stdTimePoint		m_TPstart;
	stdTimePoint		m_TPlatest;
	bool			m_centerOriginNeeded = true;
	bool			m_timerLive = false;
	// SOUNDS
	sf::SoundBuffer		m_sbClicked;
	sf::Sound		m_sClicked;
	sf::SoundBuffer		m_sbWindingUp;
	sf::Sound		m_sWindingUp;
	sf::SoundBuffer		m_sbChime;
	sf::Sound		m_sChime;
	int32			m_winPosX = 30;
	int32			m_winPosY = 74;
	int32			m_pomodoroFontSize = -1;
};

// EN_COUNTDOWN_STATE_H
#endif

// ===================================80 chars=================================|
/* EOF */

