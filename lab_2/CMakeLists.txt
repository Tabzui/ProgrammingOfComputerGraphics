cmake_minimum_required(VERSION 3.16)
project(Lab2_Graphics LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Подключение модулей Qt, включая Test
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets Test)

# Основное приложение
add_executable(Lab2_Graphics
    main.cpp
    ScannerEngine.h
    ScannerEngine.cpp
    Parsers.h
    Parsers.cpp
    ImageInfo.h
    MainWindow.h
    MainWindow.cpp
)

target_link_libraries(Lab2_Graphics PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
)

# Модульные тесты
enable_testing()

add_executable(UnitTests
    tst_parsers.cpp
    Parsers.h
    Parsers.cpp
    ImageInfo.h
)

target_link_libraries(UnitTests PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Test
)

add_test(NAME UnitTests COMMAND UnitTests)
