/* en_application.h */
// ===================================80 chars=================================|

#ifndef EN_APPLICATION_H
#define EN_APPLICATION_H

#include "en_essentials.h"
#include "en_shared_context.h"
#include "en_defines.h"
#include "en_intro_state.h"
#include "en_globals.h"
#include "game/mc_game_defines.h"

// Required for SFML setIcon
#include <SFML/Graphics.hpp>

// Required for JSON
#include "3rd-party/json-nlohmann/v.2.1.1/json.hpp"
// Required for JSON
#include <fstream>

class Application
{
public:
	Application();
	virtual ~Application();
	int	createWindow();
	void	loadSettings();
	void	run();

private:
	const t_objectName	m_myObjNameStr;
	StateMachine		m_machine;
	sf::RenderWindow	m_window;
	EngineSharedContext	m_enSharedContext;
};

// EN_APPLICATION_H
#endif

// ===================================80 chars=================================|
/* EOF */

