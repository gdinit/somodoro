/* en_breakshort_state.cc */
// ===================================80 chars=================================|

#include "en_breakshort_state.h"

// TODO delete these?
extern std::unique_ptr <Settings>	SETTINGS;
extern std::unique_ptr <Globals>	GLOBALS;

// Used to increase/decrease window size with hotkeys
#define SIZE_STEP_PX 20
#define MIN_SIZE_PX 40

BreakshortState::BreakshortState( StateMachine &machine
	, sf::RenderWindow &window
	, EngineSharedContext &context
	, bool replace )
	: State{ machine, window, context, replace }
	, m_myObjNameStr( "BreakshortState" )
{
	loadSounds();
	playSoundWindingUp();
	// Reset to prevent instant-game-over next time
	GLOBALS->returnToMainMenuRequested = 0;
	m_timerLive = true;

	#if defined DBG
	std::cout << "[DEBUG]\tCreated state:\t\t" << m_myObjNameStr << "\n";
	#endif

	restartStateClock();
	// TODO base these values on config variables
	m_desiredAspectRatio = 640.f / 480.f;
	#if defined DBG
	std::cout << "[DEBUG]\tm_desiredAspectRatio is: \t" <<
	m_desiredAspectRatio << " //" << m_myObjNameStr << "\n";
	#endif

	// need this? m_worldView = m_window.getDefaultView();
	m_urgentUpdateNeeded = 10;

	// debug overlay font
	m_font.loadFromFile( "assets/fonts/sansation.ttf" );
	m_statisticsText.setFont( m_font );
	m_statisticsText.setPosition( 5.f, 5.f );
	m_statisticsText.setCharacterSize( 12u );
	m_statisticsText.setFillColor( sf::Color::White );
	updateDebugOverlayTextIfEnabled( true );
	// === ImGui Stuff =====================================================
	m_deltaClock.restart();
	ImGui::SFML::Init( m_window );
	// =====================================================================

	// START A NEW GAME
	m_windowSize = m_window.getSize();
	m_res.x = static_cast <float> ( m_windowSize.x );
	m_res.y = static_cast <float> ( m_windowSize.y );

	// TODO move JSON work to a function
	// TODO move JSON work to a single unified location in app for all
	// states
	std::ifstream	i( "data/settings.json" );
	nlohmann::json	j;
	i >> j;
	for ( nlohmann::json::iterator it = j.begin(); it != j.end(); ++it ) {
		if ( it.key() == "m_breakshortBgColorR" ) {
			m_breakshortBgColorR = it.value();
		} else if ( it.key() == "m_breakshortBgColorG" ) {
			m_breakshortBgColorG = it.value();
		} else if ( it.key() == "m_breakshortBgColorB" ) {
			m_breakshortBgColorB = it.value();
		} else if ( it.key() == "m_secsBreakshort" ) {
			m_secsBreakshort = it.value();
		} else if ( it.key() == "m_fontSizePxBreakshort" ) {
			m_fontSizePxBreakshort = it.value();
		} else if ( it.key() == "winAutoResize" ) {
			m_enSharedContext.winAutoResize = it.value();
			std::cout << "m_enSharedContext.winAutoResize is: " <<
			m_enSharedContext.winAutoResize << '\n';
		}
	}
	i.close();

	PDASSERT( ( m_fontSizePxBreakshort > 0 )
		,
		"ERROR: m_fontSizePxBreakshort must be > 0!\tIt is: " << m_fontSizePxBreakshort <<
		"\n" );

	m_breakshortBgColor.r = m_breakshortBgColorR;
	m_breakshortBgColor.g = m_breakshortBgColorG;
	m_breakshortBgColor.b = m_breakshortBgColorB;
	std::cout << "Short Break started - counting down: " <<
	m_secsBreakshort << " seconds.\n";

	m_breakshortFont.loadFromFile( "assets/fonts/monofont.ttf" );
	m_breakshortText.setFont( m_breakshortFont );
	m_breakshortText.setCharacterSize( m_fontSizePxBreakshort );
	m_breakshortText.setFillColor( sf::Color::White );

	// TODO change this to steady clock
	m_TPstart = std::chrono::system_clock::now();

	// must happen after everything else
	winAutoResizeIfRequested();
}

BreakshortState::~BreakshortState()
{
	#if defined DBG
	std::cout << "[DEBUG]\tDestructed state:\t" << m_myObjNameStr << "\n";
	#endif

	// TODO remove me
	#if defined DBG
	std::cout << "[DEBUG]\t" << m_myObjNameStr << " run time was: " <<
	getStateAgeAsSeconds() << " seconds\n";
	#endif
}

void BreakshortState::update()
{
	sf::Time m_elapsedTime = m_clock.restart();
	m_timeSinceLastUpdate += m_elapsedTime;

	if ( GLOBALS->returnToMainMenuRequested == 1 ) {
		// Game over! Terminate
		#if defined DBG
		std::cout << "Terminating due to returnToMainMenuRequested=1\n";
		#endif
		m_next = StateMachine::build <MainMenuState>
				( m_machine, m_window, m_enSharedContext
				, true );
	}

	while ( m_timeSinceLastUpdate > State::TimePerFrame ) {
		m_timeSinceLastUpdate -= State::TimePerFrame;

		processEvents();
		// obtain current screen res - need to pass it to game objects
		m_res = static_cast <sf::Vector2f>
			( m_window.getSize() );
		if ( m_enSharedContext.mustMainMenu == true ) {
			m_next = StateMachine::build <MainMenuState> (
					m_machine, m_window
					, m_enSharedContext
					, true );
		}
		// update statistics for the debug overlay
		m_statisticsUpdateTime += m_elapsedTime;
		m_statisticsNumFrames += 1;
		// update statsText only once a second
		if ( m_urgentUpdateNeeded > 0 ) {
			// update now!
			--m_urgentUpdateNeeded;
			updateDebugOverlayTextIfEnabled();
			printConsoleDebugIfEnabled();
			m_urgentUpdateNeeded = false;
		}
		if ( m_statisticsUpdateTime >= sf::seconds( 1.0f ) ) {
			if ( m_statisticsNumFrames <= 1 ) {
				break;	// if we're playing catchup, don't
				// bother with debugOverlayText
			}
			recordObservedFPS();
			dynamicallyAdjustFPSLimit();
			updateDebugOverlayTextIfEnabled();
			printConsoleDebugIfEnabled();
			m_statisticsUpdateTime -= sf::seconds( 1.0f );
			m_statisticsNumFrames = 0;
		}
	}

	if ( m_timerLive ) {
		calculateUpdateTimer();
	}
}

void BreakshortState::draw()
{
	m_enSharedContext.frameID++;
	m_window.clear( m_breakshortBgColor );
	m_grabbedWindow = m_windowActive
		&& m_enSharedContext.winMoveable
		&& sf::Mouse::isButtonPressed( sf::Mouse::Left );
	if ( m_grabbedWindow ) {
		m_window.setPosition( sf::Mouse::getPosition() +
			m_grabbedOffset );
	}
	m_window.setView( m_enSharedContext.view );

	// === ImGui Stuff =====================================================
	ImGui::SFML::Update( m_window, m_deltaClock.restart() );
	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoTitleBar;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoScrollbar;
	window_flags |= ImGuiWindowFlags_NoCollapse;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoMove;
	ImGui::SetNextWindowPos( ImVec2( 0, 0 ), ImGuiCond_Always );
	ImGui::SetNextWindowSize( ImVec2( -1, -1 ), ImGuiCond_Always );
	bool	boolPOpen = false;
	ImVec2	sizeOnFirstUse = ImVec2( -1, -1 );
	float	bgAlpha = 0.f;
	ImGui::Begin( " ", &boolPOpen, sizeOnFirstUse, bgAlpha, window_flags );
	if ( ImGui::Button( "Main Menu" ) ) {
		playSoundClicked();
		// m_enSharedContext.reqPlaySound = 1;
		m_next = StateMachine::build <MainMenuState> ( m_machine
				, m_window, m_enSharedContext, true );
	}
	ImGui::End();
	ImGui::SFML::Render( m_window );
	// =====================================================================

	if ( SETTINGS->inGameOverlay ) {
		m_window.draw( m_statisticsText );
	}
	m_window.draw( m_breakshortText );
	m_window.display();
}

void BreakshortState::pause()
{
	#if defined DBG
	std::cout << "[DEBUG]\tPaused State:\t\t" << m_myObjNameStr << "\n";
	#endif
}

void BreakshortState::resume()
{
	restartStateClock();
	m_urgentUpdateNeeded = 10;
	// destroy the queue
	// give me stats in the first frame, but first make up some plausible
	// values
	updateDebugOverlayTextIfEnabled( true );

	#if defined DBG
	std::cout << "[DEBUG]\tResumed State:\t\t" << m_myObjNameStr << "\n";
	#endif

	// if there is a pending play sound request, play it
	if ( m_enSharedContext.reqPlaySound ) {
		m_enSharedContext.reqPlaySound = 0;
		m_sClicked.play();
	}
}

void BreakshortState::processEvents()
{
	sf::Event evt;
	while ( m_window.pollEvent( evt ) ) {
		// === ImGui Stuff =============================================
		ImGui::SFML::ProcessEvent( evt );
		// =============================================================
		switch ( evt.type ) {
		case sf::Event::Closed:
			m_machine.quit();
			break;
		case sf::Event::GainedFocus:
			m_windowActive = true;
			break;
		case sf::Event::LostFocus:
			m_windowActive = false;
			makeWindowAlwaysOnTop();
			break;
		case sf::Event::Resized:
			// onResize();
			m_enSharedContext.view = getLetterboxView(
					m_enSharedContext.view
					, evt.size.width
					, evt.size.height );
			break;
		case sf::Event::MouseButtonPressed:
			if ( evt.mouseButton.button == sf::Mouse::Left ) {
				m_grabbedOffset = m_window.getPosition() -
					sf::Mouse::getPosition();
			}
			break;
		case sf::Event::KeyPressed:
			switch ( evt.key.code ) {
			case sf::Keyboard::Escape:
			case sf::Keyboard::Q:
				std::cout << "Quitting on Q key"
				" press. Goodbye!\n";
				m_machine.quit();
				break;
			case sf::Keyboard::M:
				m_next = StateMachine::build
					<MainMenuState> ( m_machine, m_window
						, m_enSharedContext, true );
				break;
			case sf::Keyboard::F2:
				this->tglDbgShowOverlay();
				break;
			case sf::Keyboard::F3:
				this->
				toggleDebugConsoleOutput();
				break;
			case sf::Keyboard::F4:
				this->
				tglDbgDFPSConsOutput();
				break;
			case sf::Keyboard::L:
				if ( ( sf::Keyboard::isKeyPressed( sf::Keyboard
					       ::RControl ) ) ||
				     ( sf::Keyboard::isKeyPressed( sf::Keyboard
					       ::
					       LControl ) ) ) {
					winToggleMoveable();
				}
				break;
			default:
				break;
			}
			break;
		case sf::Event::KeyReleased:
			switch ( evt.key.code ) {
			// Disabling windows size change as it is really badly
			// implemented at the moment
			// case sf::Keyboard::Add:
			// case sf::Keyboard::Num9:
			// winSizeIncrease( 1 );
			// break;
			// case sf::Keyboard::Subtract:
			// case sf::Keyboard::Num0:
			// winSizeDecrease( 1 );
			// break;
			default:
				break;
			}
			break;
		default:
			break;
		}
	}
}

void BreakshortState::onResize()
{
}

void BreakshortState::makeWindowAlwaysOnTop()
// TODO: move this to state.cc
{
	#ifdef _WIN32
	// define something for Windows (32-bit and 64-bit, this part is common)
	HWND hwnd = m_window.getSystemHandle();
	SetWindowPos( hwnd, HWND_TOPMOST, 0, 0, 0, 0
		, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );

	#ifdef _WIN64
	// define something for Windows (64-bit only)
	#else
	// define something for Windows (32-bit only)
	#endif
	#elif __APPLE__
	#include "TargetConditionals.h"
	#if TARGET_IPHONE_SIMULATOR
	// iOS Simulator
	#elif TARGET_OS_IPHONE
	// iOS device
	#elif TARGET_OS_MAC
	// Other kinds of Mac OS
	#else
	#   error "Unknown Apple platform"
	#endif
	#elif __linux__
	// linux
	#elif __unix__	// all unices not caught above
	// Unix
	#elif defined ( _POSIX_VERSION )
	// POSIX
	#else
	#   error "Unknown compiler"
	#endif
}

void BreakshortState::calculateUpdateTimer()
{
	m_TPlatest = std::chrono::system_clock::now();
	std::chrono::duration <double> elapsed_secs = m_TPlatest - m_TPstart;
	m_countdownSecondsRemaining = m_secsBreakshort - round(
			elapsed_secs.count() );

	if ( elapsed_secs.count() >= m_secsBreakshort ) {
		m_timerLive = false;
		playSoundChime();
	}

	if ( !m_window.hasFocus() ) {
		std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
	}
	updateText();
	centerText();
}

void BreakshortState::updateText()
{
	m_breakshortText.setString( "" );
	int	minutes = ( m_countdownSecondsRemaining / 60 ) % 60;
	int	seconds = m_countdownSecondsRemaining %	60;
	if ( seconds == 0 ) {
		m_breakshortText.setString(
			// MINUTES
			std::to_string( minutes )
			// SEPARATOR & SECONDS
			+ ":00"
			// Close
			);
	} else if ( seconds > 0 && seconds <= 9 ) {
		m_breakshortText.setString(
			// MINUTES
			std::to_string( minutes )
			// SEPARATOR
			+ ":0"
			// SECONDS
			+ std::to_string( seconds )
			// Close
			);
	} else {
		m_breakshortText.setString(
			// MINUTES
			std::to_string( minutes )
			// SEPARATOR
			+ ":"
			// SECONDS
			+ std::to_string( seconds )
			// Close
			);
	}
}

void BreakshortState::centerText()
{
	if ( m_centerOriginNeeded == true ) {
		m_centerOriginNeeded = false;
		centerOrigin( m_breakshortText );
	}
	centerOrigin( m_breakshortText );
	m_breakshortText.setPosition( m_windowSize.x / 2, m_windowSize.y / 2 );
}

void BreakshortState::loadSounds()
{
	if ( !m_sbClicked.loadFromFile(
		     "assets/sounds/clicked.wav" ) ) {
	}
	m_sClicked.setBuffer( m_sbClicked );

	if ( !m_sbWindingUp.loadFromFile(
		     "assets/sounds/winding_up.ogg" ) ) {
	}
	m_sWindingUp.setBuffer( m_sbWindingUp );

	if ( !m_sbChime.loadFromFile(
		     "assets/sounds/chime.ogg" ) ) {
	}
	m_sChime.setBuffer( m_sbChime );
}

void BreakshortState::playSoundIfRequested()
{
	if ( m_enSharedContext.reqPlaySound ) {
		m_enSharedContext.reqPlaySound = 0;
		playSoundClicked();
	}
}

void BreakshortState::playSoundClicked()
{
	#if defined DBG
	std::cout << "[DEBUG] Playing a sound.\t" << m_myObjNameStr << "\n";
	#endif
	m_sClicked.play();
}

void BreakshortState::playSoundWindingUp()
{
	#if defined DBG
	std::cout << "[DEBUG] Playing a sound.\t" << m_myObjNameStr << "\n";
	#endif
	m_sWindingUp.play();
}

void BreakshortState::playSoundChime()
{
	#if defined DBG
	std::cout << "[DEBUG] Playing a sound.\t" << m_myObjNameStr << "\n";
	#endif
	m_sChime.play();
}

void BreakshortState::winSizeIncrease( int times )
{
	for ( int n = 0; n < times; n++ ) {
		sf::Vector2u	cSize = m_window.getSize();
		sf::Vector2u	nSize
			= { cSize.x + SIZE_STEP_PX, cSize.y + SIZE_STEP_PX };

		#if defined DBG
		std::cout << "[DEBUG] curSize: " << cSize.x << "," << cSize.y <<
		"\tnewSize: " << nSize.x << "," << nSize.y << "\t//" <<
		m_myObjNameStr << "\n";
		#endif
		m_window.setSize( nSize );
		m_winPosX += SIZE_STEP_PX;
		m_winPosY += SIZE_STEP_PX;
	}
}

void BreakshortState::winSizeDecrease( int times )
{
	for ( int n = 0; n < times; n++ ) {
		sf::Vector2u	cSize = m_window.getSize();
		sf::Vector2u	nSize = { cSize.x - SIZE_STEP_PX
					  , cSize.y - SIZE_STEP_PX };
		#if defined DBG
		std::cout << "[DEBUG] curSize: " << cSize.x << "," << cSize.y <<
		"\tnewSize: " << nSize.x << "," << nSize.y << "\t//" <<
		m_myObjNameStr << "\n";
		#endif

		if ( nSize.x > MIN_SIZE_PX && nSize.y > MIN_SIZE_PX ) {
			m_window.setSize( nSize );
			m_winPosX -= SIZE_STEP_PX;
			m_winPosY -= SIZE_STEP_PX;
		}
	}
}

void BreakshortState::winToggleMoveable()
{
	m_enSharedContext.winMoveable = !m_enSharedContext.winMoveable;
	#if defined DBG
	std::string newValueText;
	if ( m_enSharedContext.winMoveable ) {
		newValueText = "ON\t";
	} else {
		newValueText = "OFF";
	}
	std::cout << "[DEBUG] Toggled moveable. New value: " << newValueText <<
	"\t//" << m_myObjNameStr << "\n";
	#endif
}

void BreakshortState::winAutoResizeIfRequested()
{
	if ( m_enSharedContext.winAutoResize ) {
		winSizeDecrease( 3 );
		m_enSharedContext.winMMMustResize
			= !m_enSharedContext.winMMMustResize;
	}
}

// ===================================80 chars=================================|
/* EOF */

