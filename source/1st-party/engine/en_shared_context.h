/* en_shared_context.h */
// ===================================80 chars=================================|

// This struct stores some data and makes it accessible to all game states.
// It is instantiated in Application() & passed to each GameState as &ref.

#ifndef EN_SHARED_CONTEXT_H
#define EN_SHARED_CONTEXT_H

struct EngineSharedContext
{
	// Window
	// TODO: Change this
	// For now we only save a pointer & access it via a pointer.
	// Improve this code so we have no pointer here at all.
	sf::RenderWindow* pWindow = nullptr;
	sf::View view;

	// Game
	bool mustMainMenu = false;
	unsigned long long int frameID = 0;

	// Time
	std::chrono::steady_clock::time_point tIntroFirstLaunchTime;

	// Sound
	bool reqSndPlyFromPlay = 0;
	bool reqSndPlyFromPause = 0;
};

// EN_SHARED_CONTEXT_H
#endif

// ===================================80 chars=================================|
/* EOF */

