cmake_minimum_required(VERSION 3.21)

project(BusJamPuzzle
    VERSION     1.0.0
    DESCRIPTION "Bus Jam Puzzle — Qt6/QML"
    LANGUAGES   CXX
)

# ─── Padrão C++ ───────────────────────────────────────────────────────────────

set(CMAKE_CXX_STANDARD          17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

# ─── Qt6 ──────────────────────────────────────────────────────────────────────

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Quick
    Qml
    QuickControls2
    QuickLayouts
)

qt_standard_project_setup()

# ─── Fontes de C++ ────────────────────────────────────────────────────────────

set(BUSJAM_CPP_SOURCES

    # Ponto de entrada
    main.cpp

    # Model — entidades de domínio puro
    src/model/Bus.cpp
    src/model/Passenger.cpp
    src/model/Platform.cpp
    src/model/Board.cpp

    # Engine — lógica de jogo
    src/engine/BoardEngine.cpp
    src/engine/MovementSolver.cpp
    src/engine/GameController.cpp
    src/engine/BusListModel.cpp
    src/engine/PassengerListModel.cpp
    src/engine/PlatformListModel.cpp

    # Persistence — I/O e progressão
    src/persistence/LevelLoader.cpp
    src/persistence/LevelManager.cpp
    src/persistence/PersistenceManager.cpp
)

set(BUSJAM_CPP_HEADERS

    # Common
    src/common/Enums.h

    # Model
    src/model/Bus.h
    src/model/Passenger.h
    src/model/Platform.h
    src/model/Board.h

    # Engine
    src/engine/BoardEngine.h
    src/engine/MovementSolver.h
    src/engine/GameController.h

    # Persistence
    src/persistence/LevelLoader.h
    src/persistence/LevelManager.h
    src/persistence/PersistenceManager.h
)

# ─── Executável principal ─────────────────────────────────────────────────────

qt_add_executable(BusJamPuzzle
    ${BUSJAM_CPP_SOURCES}
    ${BUSJAM_CPP_HEADERS}
)

# ─── Módulo QML ───────────────────────────────────────────────────────────────
# qt_add_qml_module regista os ficheiros QML como recursos compilados (Qt 6.2+).
# O URI "BusJamPuzzle" corresponde ao import path em main.qml.

qt_add_qml_module(BusJamPuzzle
    URI BusJamPuzzle
    VERSION 1.0
    QML_FILES
        # Raiz
        qml/main.qml

        # Ecrãs
        qml/screens/MainMenuScreen.qml
        qml/screens/LevelSelectScreen.qml
        qml/screens/GameScreen.qml

        # Componentes
        qml/components/BoardView.qml
        qml/components/BusItem.qml
        qml/components/PassengerItem.qml
        qml/components/PlatformView.qml
        qml/components/HUD.qml

        # Style (singletons)
        qml/style/Theme.qml
        qml/style/Sizes.qml
    RESOURCES
        # qmldir dos singletons
        qml/style/qmldir
        qml/components/qmldir
)

# ─── Recursos estáticos ───────────────────────────────────────────────────────
# Fontes, ícones e ficheiros de nível JSON.

set(BUSJAM_STATIC_FILES "")
foreach(_font
        assets/fonts/FredokaOne-Regular.ttf
        assets/fonts/Nunito-Regular.ttf
        assets/fonts/Nunito-Bold.ttf)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${_font}")
        list(APPEND BUSJAM_STATIC_FILES ${_font})
    else()
        message(STATUS "Fonte opcional não encontrada (ignorada): ${_font}")
    endif()
endforeach()
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets/icons/app_icon.png")
    list(APPEND BUSJAM_STATIC_FILES assets/icons/app_icon.png)
else()
    message(STATUS "Ícone opcional não encontrado (ignorado): assets/icons/app_icon.png")
endif()

if(BUSJAM_STATIC_FILES)
    qt_add_resources(BusJamPuzzle "static_assets"
        PREFIX "/"
        FILES ${BUSJAM_STATIC_FILES}
    )
endif()

qt_add_resources(BusJamPuzzle "level_data"
    PREFIX "/levels"
    FILES
        levels/level_001.json
        levels/level_002.json
        levels/level_003.json
        levels/level_004.json
        levels/level_005.json
        levels/level_006.json
        levels/level_007.json
        levels/level_008.json
        levels/level_009.json
        levels/level_010.json
)

# ─── Includes ─────────────────────────────────────────────────────────────────

target_include_directories(BusJamPuzzle PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common
    ${CMAKE_CURRENT_SOURCE_DIR}/src/model
    ${CMAKE_CURRENT_SOURCE_DIR}/src/engine
    ${CMAKE_CURRENT_SOURCE_DIR}/src/persistence
)

# ─── Ligação de bibliotecas ───────────────────────────────────────────────────

target_link_libraries(BusJamPuzzle PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::Qml
    Qt6::QuickControls2
    Qt6::QuickLayouts
)

# ─── Flags de compilação ──────────────────────────────────────────────────────

target_compile_options(BusJamPuzzle PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:
        /W4
        /WX
        /permissive-
        /wd4068          # Suprimir aviso de pragma desconhecido (Qt moc)
    >
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
        -Wnon-virtual-dtor
        -Woverloaded-virtual
    >
)

# Definições de compilação
target_compile_definitions(BusJamPuzzle PRIVATE
    APP_VERSION="${PROJECT_VERSION}"
    QT_DISABLE_DEPRECATED_BEFORE=0x060000  # Ignora API antes do Qt 6.0
)

# ─── Configuração para mobile ─────────────────────────────────────────────────

# Android
if(ANDROID)
    set_target_properties(BusJamPuzzle PROPERTIES
        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android"
        QT_ANDROID_VERSION_NAME       "${PROJECT_VERSION}"
        QT_ANDROID_VERSION_CODE       1
        QT_ANDROID_MIN_SDK_VERSION    21
        QT_ANDROID_TARGET_SDK_VERSION 33
    )
endif()

# iOS
if(IOS)
    set_target_properties(BusJamPuzzle PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER  "com.example.busjampuzzle"
        MACOSX_BUNDLE_BUNDLE_VERSION  "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
        XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"   # iPhone + iPad
    )
endif()

# ─── Testes unitários ─────────────────────────────────────────────────────────

option(BUSJAM_BUILD_TESTS "Compilar testes unitários" ON)

if(BUSJAM_BUILD_TESTS)
    find_package(Qt6 COMPONENTS Test QUIET)

    if(Qt6Test_FOUND)
        enable_testing()

        # Biblioteca partilhada de domínio para os testes
        add_library(BusJamDomain STATIC
            ${BUSJAM_CPP_SOURCES}
            ${BUSJAM_CPP_HEADERS}
        )
        target_include_directories(BusJamDomain PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/src
            ${CMAKE_CURRENT_SOURCE_DIR}/src/common
            ${CMAKE_CURRENT_SOURCE_DIR}/src/model
            ${CMAKE_CURRENT_SOURCE_DIR}/src/engine
            ${CMAKE_CURRENT_SOURCE_DIR}/src/persistence
        )
        target_link_libraries(BusJamDomain PUBLIC
            Qt6::Core Qt6::Gui Qt6::Quick Qt6::Qml
        )

        # Macro para criar testes facilmente
        macro(busjam_add_test TEST_NAME TEST_SRC)
            add_executable(${TEST_NAME} ${TEST_SRC})
            target_link_libraries(${TEST_NAME} PRIVATE BusJamDomain Qt6::Test)
            add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
        endmacro()

        busjam_add_test(tst_BoardEngine   tests/tst_BoardEngine.cpp)
        busjam_add_test(tst_MovementSolver tests/tst_MovementSolver.cpp)
        busjam_add_test(tst_LevelLoader    tests/tst_LevelLoader.cpp)
    else()
        message(STATUS "Qt6::Test não encontrado — testes desactivados.")
    endif()
endif()

# ─── Instalação ───────────────────────────────────────────────────────────────

install(TARGETS BusJamPuzzle
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Necessário para Qt6 deployar o QML em instalação
qt_generate_deploy_app_script(
    TARGET BusJamPuzzle
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})

# ─── Mensagem de resumo ───────────────────────────────────────────────────────

message(STATUS "")
message(STATUS "╔══ BusJamPuzzle ${PROJECT_VERSION} ═══════════════╗")
message(STATUS "║  Qt             : ${Qt6_VERSION}")
message(STATUS "║  Compilador     : ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "║  Build type     : ${CMAKE_BUILD_TYPE}")
message(STATUS "║  Testes         : ${BUSJAM_BUILD_TESTS}")
message(STATUS "╚══════════════════════════════════════╝")
message(STATUS "")
