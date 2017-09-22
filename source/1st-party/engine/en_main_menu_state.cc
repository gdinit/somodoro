/* en_main_menu_state.cc */
// ===================================80 chars=================================|

#include "en_main_menu_state.h"

// Used to increase/decrease window size with hotkeys
#define SIZE_STEP_PX 20
#define MIN_SIZE_PX 40

MainMenuState::MainMenuState( StateMachine &machine
	, sf::RenderWindow &window
	, EngineSharedContext &context
	, bool replace )
	: State{ machine, window, context, replace }, m_myObjNameStr(
		"MainMenuState" )
{
	// Get size of window.
	m_windowSize = m_window.getSize();
	readSettings();

	#if defined DBG
	std::cout << "[DEBUG]\tCreated state:\t\t" << m_myObjNameStr << "\n";
	#endif
	restartStateClock();
	loadSounds();
	playSoundClicked();
	// resize stuff here
	// TODO base these values on config variables
	m_desiredAspectRatio = 640.f / 480.f;
	#if defined DBG
	std::cout << "[DEBUG]\tm_desiredAspectRatio is: \t" <<
	m_desiredAspectRatio << " //" << m_myObjNameStr << "\n";
	#endif
	m_enSharedContext.mustMainMenu = false;
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

	m_tex1Start.loadFromFile( "assets/textures/start.png" );
	m_tex2Short.loadFromFile( "assets/textures/short.png" );
	m_tex3Long.loadFromFile( "assets/textures/long.png" );
	////////////////////////////////////////

	// must happen after everything else
	// change this to winAutoResizeIfRequested();
	if ( m_enSharedContext.winMMMustResize ) {
		m_enSharedContext.winMMMustResize
			= !m_enSharedContext.winMMMustResize;
		winSizeIncrease( 3 );
	}
}

MainMenuState::~MainMenuState()
{
	#if defined DBG
	std::cout << "[DEBUG]\tDestructed state:\t" << m_myObjNameStr << "\n";
	#endif
}

void MainMenuState::update()
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
		// however, if just entered this state
		// (i.e.: this is the 2nd updateStats), then immediately update
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
}

void MainMenuState::draw()
{
	m_enSharedContext.frameID++;
	m_window.clear( m_mainmenuBgColor );
	m_grabbedWindow = m_windowActive
		&& m_enSharedContext.winMoveable
		&& sf::Mouse::isButtonPressed( sf::Mouse::Left );
	if ( m_grabbedWindow ) {
		m_window.setPosition( sf::Mouse::getPosition() +
			m_grabbedOffset );
	}
	m_window.setView( m_enSharedContext.view );
	ImGui::SFML::Update( m_window, m_deltaClock.restart() );
	// =====================================================================
	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoTitleBar;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoScrollbar;
	window_flags |= ImGuiWindowFlags_NoCollapse;
	window_flags |= ImGuiWindowFlags_NoResize;
	window_flags |= ImGuiWindowFlags_NoMove;
	// window_flags |= ImGuiWindowFlags_MenuBar;
	// window_flags |= ImGuiWindowFlags_ShowBorders;
	//
	// ImGui::SetNextWindowPos( ImVec2( m_windowSize.x / 2, m_windowSize.y
	/// 2 ), ImGuiCond_Always );
	//
	// FIXME hardcoded value. not safe. should be calculated
	ImGui::SetNextWindowSize( ImVec2( 160, 160 ), ImGuiCond_Always );
	ImGui::SetNextWindowPosCenter( ImGuiCond_Always );
	//
	// boolPOpen: Click upper right corner to close a window, available when
	// 'bool* p_open' is passed to ImGui::Begin()
	bool	boolPOpen = true;
	ImVec2	sizeOnFirstUse = ImVec2( 100, 100 );
	float	bgAlpha = 0.f;
	ImGui::Begin( " ", &boolPOpen, sizeOnFirstUse, bgAlpha, window_flags );
	// =====================================================================
	if ( ImGui::ImageButton( m_tex1Start, -1, sf::Color::Green
		     , sf::Color::White ) ) {
		// m_enSharedContext.reqPlaySound = 1;
		playSoundClicked();
		m_next = StateMachine::build <CountdownState> ( m_machine
				, m_window, m_enSharedContext, true );
	}
	// =====================================================================
	if ( ImGui::ImageButton( m_tex2Short, -1, sf::Color::Green
		     , sf::Color::White ) ) {
		// m_enSharedContext.reqPlaySound = 1;
		playSoundClicked();
		m_next = StateMachine::build <BreakshortState> ( m_machine
				, m_window, m_enSharedContext, true );
	}
	// =====================================================================
	if ( ImGui::ImageButton( m_tex3Long, -1, sf::Color::Green
		     , sf::Color::White ) ) {
		// m_enSharedContext.reqPlaySound = 1;
		playSoundClicked();
		m_next = StateMachine::build <BreaklongState> ( m_machine
				, m_window, m_enSharedContext, true );
	}
	// =====================================================================
	ImGui::End();
	ImGui::SFML::Render( m_window );
	// =====================================================================
	if ( SETTINGS->inGameOverlay ) {
		m_window.draw( m_statisticsText );
	}
	m_window.display();
}

void MainMenuState::onResize()
{
}

void MainMenuState::pause()
{
}

void MainMenuState::resume()
{
	restartStateClock();
}

void MainMenuState::processEvents()
{
	// fetch events
	sf::Event evt;

	// process events
	while ( m_window.pollEvent( evt ) ) {
		ImGui::SFML::ProcessEvent( evt );
		switch ( evt.type ) {
		case sf::Event::Closed:
			std::cout << "Quitting on close event."
			" Goodbye!\n";
			m_machine.quit();
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
			if ( evt.mouseButton.button ==
			     sf::Mouse::Left ) {
				m_grabbedOffset =
					m_window.getPosition() -
					sf::Mouse::getPosition();
			}
			break;
		case sf::Event::KeyPressed:
			switch ( evt.key.code ) {
			case sf::Keyboard::Space:
				m_enSharedContext.reqPlaySound = 1;
				m_next = StateMachine::build
					<CountdownState> ( m_machine
						, m_window
						,
						m_enSharedContext
						, true );
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
			case sf::Keyboard::Escape:
			case sf::Keyboard::Q:
				std::cout << "Quitting on Q key press."
				" Goodbye!\n";
				m_machine.quit();
				break;
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
			case sf::Keyboard::M:
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
		default:
			break;
		}
	}
}

void MainMenuState::makeWindowAlwaysOnTop()
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

void MainMenuState::winSizeIncrease( int times )
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
	}
}

void MainMenuState::winSizeDecrease( int times )
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
		}
	}
}

void MainMenuState::winToggleMoveable()
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

void MainMenuState::loadSounds()
{
	if ( !m_sbClicked.loadFromFile(
		     "assets/sounds/clicked.wav" ) ) {
	}
	m_sClicked.setBuffer( m_sbClicked );
}

void MainMenuState::playSoundClicked()
{
	#if defined DBG
	std::cout << "[DEBUG] Playing a sound.\t" << m_myObjNameStr << "\n";
	#endif
	m_sClicked.play();
}

void MainMenuState::readSettings()
{
	std::cout << "Reading settings from file...\n";

	std::ifstream	i( "data/settings.json" );
	nlohmann::json	j;
	i >> j;
	for ( nlohmann::json::iterator it = j.begin(); it != j.end(); ++it ) {
		if ( it.key() == "m_mainmenuBgColorR" ) {
			m_mainmenuBgColorR = it.value();
		} else if ( it.key() == "m_mainmenuBgColorG" ) {
			m_mainmenuBgColorG = it.value();
		} else if ( it.key() == "m_mainmenuBgColorB" ) {
			m_mainmenuBgColorB = it.value();
		} else if ( it.key() == "winMoveable" ) {
			m_enSharedContext.winMoveable = it.value();
		} else if ( it.key() == "winAutoResize" ) {
			m_enSharedContext.winAutoResize = it.value();
			std::cout << "m_enSharedContext.winAutoResize is: " <<
			m_enSharedContext.winAutoResize << '\n';
		}
	}
	i.close();
	m_mainmenuBgColor.r = m_mainmenuBgColorR;
	m_mainmenuBgColor.g = m_mainmenuBgColorG;
	m_mainmenuBgColor.b = m_mainmenuBgColorB;
}

// ===================================80 chars=================================|
/* EOF */

