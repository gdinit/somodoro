/* en_break_state.cc */
// ===================================80 chars=================================|

#include "en_break_state.h"

// TODO delete this?
extern std::unique_ptr <Settings> SETTINGS;

// Used to increase/decrease window size with hotkeys
#define SIZE_STEP_PX 20
#define MIN_SIZE_PX 40

BreakState::BreakState( StateMachine &machine
	, sf::RenderWindow &window
	, EngineSharedContext &context
	, bool replace )
	: State{ machine, window, context, replace }
	, m_myObjNameStr( "ToBeUpdated" )
	, m_breakType( -1 )
{
	restartStateClock();
	setBreakType();
	readSettingsFromJson();
	validateSettings();
	assignSettingsBasedOnBreakType();
	loadSounds();
	playSoundWindingUp();
	m_windowSize = m_window.getSize();
	m_timerLive = true;
	m_urgentUpdateNeeded = 10;
	initDebugFont();
	// TODO change this to steady clock
	m_TPstart = std::chrono::system_clock::now();
	// === ImGui Stuff =====================================================
	m_deltaClock.restart();
	ImGui::SFML::Init( m_window );
	// =====================================================================
	// must happen after everything else
	winAutoResizeIfRequested();
	#if defined DBG
	std::cout << "[DEBUG]\tCreated state:\t\t" << m_myObjNameStr <<	"\n";
	#endif
}

BreakState::~BreakState()
{
	#if defined DBG
	std::cout << "[DEBUG]\tDestructed state:\t" << m_myObjNameStr << "\n";
	#endif
	#if defined DBG
	std::cout << "[DEBUG]\t" << m_myObjNameStr << " run time was: " <<
	getStateAgeAsSeconds() << " seconds\n";
	#endif
}

void BreakState::update()
{
	winAutoToggleMoveableIfNecessary();
	sf::Time m_elapsedTime = m_clock.restart();
	m_timeSinceLastUpdate += m_elapsedTime;
	while ( m_timeSinceLastUpdate > State::TimePerFrame ) {
		m_timeSinceLastUpdate -= State::TimePerFrame;
		processEvents();
		( m_window.getSize() );
		if ( m_enSharedContext.mustMainMenu == true ) {
			m_next = StateMachine::build <MainMenuState> (
					m_machine, m_window, m_enSharedContext
					, true );
		}
		// update statistics for the debug overlay
		m_statisticsUpdateTime += m_elapsedTime;
		m_statisticsNumFrames += 1;
		// update statsText only once a second
		if ( m_urgentUpdateNeeded > 0 ) {
			--m_urgentUpdateNeeded;
			updateDebugOverlayTextIfEnabled();
			printConsoleDebugIfEnabled();
			m_urgentUpdateNeeded = false;
		}
		if ( m_statisticsUpdateTime >= sf::seconds( 1.0f ) ) {
			if ( m_statisticsNumFrames <= 1 ) {
				// if playing catchup, don't bother
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
	if ( m_timerLive ) {
		calculateUpdateTimer();
	}
}

void BreakState::draw()
{
	m_enSharedContext.frameID++;
	m_window.clear( m_breakBgColor );
	m_grabbedWindow = m_windowActive
		&& m_enSharedContext.winMoveable
		&& sf::Mouse::isButtonPressed( sf::Mouse::Left );
	if ( m_grabbedWindow ) {
		m_window.setPosition(
			sf::Mouse::getPosition() + m_grabbedOffset );
	}
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
	ImGui::Begin( " ", &boolPOpen, sizeOnFirstUse, bgAlpha
		, window_flags );
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
	m_window.draw( m_breakText );
	m_window.display();
}

void BreakState::pause()
{
}

void BreakState::resume()
{
}

void BreakState::processEvents()
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
			m_enSharedContext.view = getLetterboxView(
					m_enSharedContext.view
					, evt.size.width
					, evt.size.height );
			break;
		case sf::Event::MouseButtonPressed:
			if ( evt.mouseButton.button ==
			     sf::Mouse::Left ) {
				m_grabbedOffset =
					m_window.getPosition() -
					sf::Mouse::getPosition();
			}
			break;
		case sf::Event::KeyPressed:
			switch ( evt.key.code ) {
			case sf::Keyboard::Q:
				std::cout << "Quitting on Q key"
				" press. Goodbye!\n";
				m_machine.quit();
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
			case sf::Keyboard::M:
				if ( ( sf::Keyboard::isKeyPressed( sf::
					       Keyboard
					       ::RControl ) ) ||
				     ( sf::Keyboard::isKeyPressed( sf::
					       Keyboard
					       ::
					       LControl ) ) ) {
					winManualToggleMoveable();
				} else {
					m_next = StateMachine::build
						<MainMenuState> (
							m_machine, m_window
							, m_enSharedContext
							,
							true );
					break;
				}
				break;
			default:
				break;
			}
			break;
		case sf::Event::KeyReleased:
			switch ( evt.key.code ) {
			//// Disabling windows size change as it is really
			//// badly
			//// implemented at the moment
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

void BreakState::makeWindowAlwaysOnTop()
// TODO: move this to state.cc
{
	#ifdef _WIN32
	// define something for Windows (32-bit and 64-bit, this part is
	// common)
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

void BreakState::calculateUpdateTimer()
{
	m_TPlatest = std::chrono::system_clock::now();
	std::chrono::duration <double> elapsed_secs = m_TPlatest - m_TPstart;
	m_countdownSecondsRemaining = m_breakSecs - round(
			elapsed_secs.count() );

	if ( elapsed_secs.count() >= m_breakSecs ) {
		m_timerLive = false;
		playSoundChime();
	}

	if ( !m_window.hasFocus() ) {
		std::this_thread::sleep_for( std::chrono::milliseconds(
				500 ) );
	}
	updateText();
	centerText();
}

void BreakState::updateText()
{
	m_breakText.setString( "" );
	int	minutes = ( m_countdownSecondsRemaining / 60 ) % 60;
	int	seconds = m_countdownSecondsRemaining % 60;
	if ( seconds == 0 ) {
		m_breakText.setString(
			// MINUTES
			std::to_string( minutes )
			// SEPARATOR & SECONDS
			+ ":00"
			// Close
			);
	} else if ( seconds > 0 && seconds <= 9 ) {
		m_breakText.setString(
			// MINUTES
			std::to_string( minutes )
			// SEPARATOR
			+ ":0"
			// SECONDS
			+ std::to_string( seconds )
			// Close
			);
	} else {
		m_breakText.setString(
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

void BreakState::centerText()
{
	if ( m_centerOriginNeeded == true ) {
		m_centerOriginNeeded = false;
		centerOrigin( m_breakText );
	}
	centerOrigin( m_breakText );
	m_breakText.setPosition( m_windowSize.x / 2
		, m_windowSize.y / 2 );
}

void BreakState::loadSounds()
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

void BreakState::playSoundIfRequested()
{
	if ( m_enSharedContext.reqPlaySound ) {
		m_enSharedContext.reqPlaySound = 0;
		playSoundClicked();
	}
}

void BreakState::playSoundClicked()
{
	#if defined DBG
	std::cout << "[DEBUG] Playing a sound.\t" << m_myObjNameStr << "\n";
	#endif
	m_sClicked.play();
}

void BreakState::playSoundWindingUp()
{
	#if defined DBG
	std::cout << "[DEBUG] Playing a sound.\t" << m_myObjNameStr << "\n";
	#endif
	m_sWindingUp.play();
}

void BreakState::playSoundChime()
{
	#if defined DBG
	std::cout << "[DEBUG] Playing a sound.\t" << m_myObjNameStr << "\n";
	#endif
	m_sChime.play();
}

void BreakState::winSizeIncrease( int times )
{
	for ( int n = 0; n < times; n++ ) {
		sf::Vector2u	cSize = m_window.getSize();
		sf::Vector2u	nSize
			= { cSize.x + SIZE_STEP_PX
			    , cSize.y + SIZE_STEP_PX };

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

void BreakState::winSizeDecrease( int times )
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

void BreakState::winManualToggleMoveable()
{
	// toggle
	m_enSharedContext.winMoveable = !m_enSharedContext.winMoveable;

	// report
	#if defined DBG
	std::string newValueText;
	if ( m_enSharedContext.winMoveable ) {
		newValueText = "ON\t";
	} else {
		newValueText = "OFF";
	}
	std::cout << "[DEBUG] Manually toggled moveable. New value: " <<
	newValueText <<	"\t//" << m_myObjNameStr << "\n";
	#endif

	// Toggle time should be updated so that autoToggleBack can
	// function
	m_enSharedContext.TPmoveToggleTime =
		std::chrono::steady_clock::now();
}

void BreakState::winAutoToggleMoveableIfNecessary()
{
	// exit, if win is NOT moveable
	if ( !m_enSharedContext.winMoveable ) {
		return;
	}
	// if window currently IS moveable AND THRESHOLD SECONDS have
	// passed,
	// disable it.
	typedef std::chrono::seconds Seconds;
	stdyTimePoint	t0 = m_enSharedContext.TPmoveToggleTime;
	stdyTimePoint	t1 = std::chrono::steady_clock::now();
	auto		elapsed_time =
		std::chrono::duration_cast <Seconds> (
			t1 - t0 );
	auto		duration = Seconds(
			m_enSharedContext.winMoveableDurationSecs );
	if ( elapsed_time >= duration ) {
		m_enSharedContext.winMoveable =
			!m_enSharedContext.winMoveable;
		#if defined DBG
		std::cout << "[DEBUG] Automatically turned off moveable!\n";
		#endif
	}
}

void BreakState::winAutoResizeIfRequested()
{
	if ( m_enSharedContext.winAutoResize ) {
		winSizeDecrease( 3 );
		m_enSharedContext.winMMMustResize
			= !m_enSharedContext.winMMMustResize;
	}
}

void BreakState::initDebugFont()
{
	m_font.loadFromFile( "assets/fonts/sansation.ttf" );
	m_statisticsText.setFont( m_font );
	m_statisticsText.setPosition( 5.f, 5.f );
	m_statisticsText.setCharacterSize( 12u );
	m_statisticsText.setFillColor( sf::Color::White );
	updateDebugOverlayTextIfEnabled( true );
}

void BreakState::readSettingsFromJson()
{
	std::ifstream	i( "data/settings.json" );
	nlohmann::json	j;
	i >> j;
	for ( nlohmann::json::iterator it = j.begin(); it != j.end();
	      ++it ) {
		if ( it.key() == "m_breakshortBgColorR" ) {
			m_breakshortBgColorR = it.value();
		} else if ( it.key() == "m_breakshortBgColorG" ) {
			m_breakshortBgColorG = it.value();
		} else if ( it.key() == "m_breakshortBgColorB" ) {
			m_breakshortBgColorB = it.value();
		} else if ( it.key() == "m_breaklongBgColorR" ) {
			m_breaklongBgColorR = it.value();
		} else if ( it.key() == "m_breaklongBgColorG" ) {
			m_breaklongBgColorG = it.value();
		} else if ( it.key() == "m_breaklongBgColorB" ) {
			m_breaklongBgColorB = it.value();
		} else if ( it.key() == "m_breakshortSecs" ) {
			m_breakshortSecs = it.value();
		} else if ( it.key() == "m_breaklongSecs" ) {
			m_breaklongSecs = it.value();
		} else if ( it.key() == "m_fontSizePxBreakshort" ) {
			m_fontSizePxBreakshort = it.value();
		} else if ( it.key() == "m_fontSizePxBreaklong" ) {
			m_fontSizePxBreaklong = it.value();
		} else if ( it.key() == "winAutoResize" ) {
			m_enSharedContext.winAutoResize = it.value();
			std::cout << "m_enSharedContext.winAutoResize is: " <<
			m_enSharedContext.winAutoResize << '\n';
		}
	}
	i.close();
}

void BreakState::assignSettingsBasedOnBreakType()
{
	if ( m_breakType == 0 ) {
		m_breakSecs = m_breakshortSecs;
		m_breakBgColor.r = m_breakshortBgColorR;
		m_breakBgColor.g = m_breakshortBgColorG;
		m_breakBgColor.b = m_breakshortBgColorB;
		m_breakFont.loadFromFile( "assets/fonts/monofont.ttf" );
		m_breakText.setFont( m_breakFont );
		m_breakText.setCharacterSize( m_fontSizePxBreakshort );
		m_breakText.setFillColor( sf::Color::White );
	} else if ( m_breakType == 1 ) {
		m_breakSecs = m_breaklongSecs;
		m_breakBgColor.r = m_breaklongBgColorR;
		m_breakBgColor.g = m_breaklongBgColorG;
		m_breakBgColor.b = m_breaklongBgColorB;
		m_breakFont.loadFromFile( "assets/fonts/monofont.ttf" );
		m_breakText.setFont( m_breaklongFont );
		m_breakText.setCharacterSize( m_fontSizePxBreaklong );
		m_breakText.setFillColor( sf::Color::White );
	}

	// Common
	#if defined DBG
	std::cout << m_myObjNameStr << " Break started - counting down: " <<
	m_breakSecs << " seconds.\n";
	#endif
}

void BreakState::validateSettings()
{
	// TODO add more PDASSERT()s to validate every setting!
	PDASSERT( ( m_fontSizePxBreakshort > 0 )
		,
		"ERROR: m_fontSizePxBreakshort must be > 0!\tIt is: " << m_fontSizePxBreakshort <<
		"\n" );
}

void BreakState::setBreakType()
{
	if ( m_enSharedContext.nextBreakIsShort ) {
		m_myObjNameStr = "BreakShort";
		m_breakType = 0;
	} else {
		m_myObjNameStr = "BreakLong";
		m_breakType = 1;
	}

	// TODO remove this debug cout
	#if defined DBG
	if ( m_breakType == -1 ) {
		std::cout << "m_breakType is NEGATIVE!!! (" << m_breakType <<
		")\n";
	} else if ( m_breakType == 0 ) {
		std::cout << "m_breakType is SHORT (" << m_breakType << ")\n";
	} else if ( m_breakType == 1 ) {
		std::cout << "m_breakType is LONG (" << m_breakType << ")\n";
	} else {
		std::cout << "m_breakType is UNKNOWN!!! (" << m_breakType <<
		")\n";
	}
	#endif
}

// ===================================80 chars=================================|
/* EOF */

