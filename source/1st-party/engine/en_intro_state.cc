/* en_intro_state.cc */
// ===================================80 chars=================================|

#include "en_intro_state.h"

IntroState::IntroState( StateMachine &machine
	, sf::RenderWindow &window
	, EngineSharedContext &context
	, bool replace )
	: State{ machine, window, context, replace }, m_myObjNameStr(
		"IntroState" )
{
	m_enSharedContext.tIntroFirstLaunchTime =
		std::chrono::steady_clock::now();

	// go straight in to mainmenuState - keep this for future use
	m_next = StateMachine::build <MainMenuState> ( m_machine, m_window
			, m_enSharedContext, true );
}

IntroState::~IntroState()
{
	#if defined DBG
	std::cout << "[DEBUG]\tDestructed state:\t" << m_myObjNameStr << "\n";
	#endif
}

void IntroState::pause()
{
}

void IntroState::resume()
{
	restartStateClock();
}

void IntroState::update()
{
	sf::Time m_elapsedTime = m_clock.restart();
	m_timeSinceLastUpdate += m_elapsedTime;

	while ( m_timeSinceLastUpdate > State::TimePerFrame ) {
		m_timeSinceLastUpdate -= State::TimePerFrame;
		processEvents();
		// update statistics for the debug overlay
		m_statisticsUpdateTime += m_elapsedTime;
		m_statisticsNumFrames += 1;
		// update statsText only once a second
		if ( m_statisticsUpdateTime >= sf::seconds( 1.0f ) ) {
			if ( m_statisticsNumFrames <= 1 ) {
				// if we're playing catchup,
				// don't bother with debugOverlayText
				break;
			}

			recordObservedFPS();
			dynamicallyAdjustFPSLimit();
			updateDebugOverlayTextIfEnabled();
			printConsoleDebugIfEnabled();

			m_statisticsUpdateTime -= sf::seconds( 1.0f );
			m_statisticsNumFrames = 0;
		}
	}
}

void IntroState::draw()
{
	m_enSharedContext.frameID++;
	m_window.clear();
	m_window.setView( m_enSharedContext.view );
	m_window.draw( m_statisticsText );
	m_window.display();
}

void IntroState::processEvents()
{
	sf::Event evt;
	while ( m_window.pollEvent( evt ) ) {
		switch ( evt.type ) {
		case sf::Event::Closed:
			m_machine.quit();
			break;
		case sf::Event::Resized:
			m_enSharedContext.view = getLetterboxView(
					m_enSharedContext.view
					, evt.size.width
					, evt.size.height );
			break;
		case sf::Event::KeyPressed:
			// NOTE: Intro should not have pause state (no
			// user input = already paused state!)
			switch ( evt.key.code ) {
			case sf::Keyboard::Escape:
			case sf::Keyboard::Space:
			case sf::Keyboard::Return:
				m_next =
					StateMachine::build
					<MainMenuState>
						( m_machine
						, m_window
						,
						m_enSharedContext
						, true );
				break;
			case sf::Keyboard::Q:
				m_machine.quit();
				break;
			case sf::Keyboard::F2:
				this->tglDbgShowOverlay();
				break;
			case sf::Keyboard::F3:
				this->toggleDebugConsoleOutput();
				break;
			case sf::Keyboard::F4:
				this->
				tglDbgDFPSConsOutput();
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
	}
}

// ===================================80 chars=================================|
/* EOF */

