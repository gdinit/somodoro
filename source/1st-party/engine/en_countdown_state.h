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
#include <imgui.h>
#include <imgui-SFML.h>

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
	void	onResize();
	void	initializeState();
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
	void	winSizeIncrease( int times );
	void	winSizeDecrease( int times );
	void	winToggleMoveable();

private:
	const t_objectName	m_myObjNameStr;
	sf::Vector2f		m_res;

	sf::Vector2u		m_textureSize;
	sf::Vector2u		m_windowSize;
	// TODO move this to app
	float
				m_desiredAspectRatio;
	// imgui stuff
	sf::Clock		m_deltaClock;
	int32			m_clicked;
	// pomodoro stuff
	sf::Vector2i		m_grabbedOffset;
	bool			m_grabbedWindow = false;
	bool			m_windowActive = true;
	// This should be overwritten by JSON parser
	sf::Color		m_countdownBgColor = sf::Color::Yellow;
	int32			m_countdownBgColorR = 0;
	int32			m_countdownBgColorG = 0;
	int32			m_countdownBgColorB = 0;
	int32			m_secsPomodoro = 0;
	int32			m_secsBreakShort = 0;	
	int32			m_secsBreakLong = 0;	
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
};

// EN_COUNTDOWN_STATE_H
#endif

// ===================================80 chars=================================|
/* EOF */

