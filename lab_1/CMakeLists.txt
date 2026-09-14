cmake_minimum_required(VERSION 3.16)
project(LAB_1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets Test REQUIRED)

add_executable(LAB_1
    main.cpp
    ColorModel.h
    ColorModel.cpp
    ColorController.h
    ColorController.cpp
    mainwindow.h
    mainwindow.cpp
)

target_link_libraries(LAB_1 PRIVATE Qt6::Widgets)

enable_testing()

add_executable(LAB_1_Tests
    test_colormodel.cpp
    ColorModel.h
    ColorModel.cpp
)

target_link_libraries(LAB_1_Tests PRIVATE Qt6::Core Qt6::Test)

add_test(NAME ColorModelTests COMMAND LAB_1_Tests)
