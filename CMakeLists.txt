cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

project(SevenSegmentDisplay)

find_package(Qt5 REQUIRED COMPONENTS Qml Quick)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Add module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# Include Melpers
include(Melpers)

#melp_print_list(CMAKE_CXX_COMPILE_FEATURES "CXX COMPILE FEATURES" SEPERATOR HALFINDENT)

# Distinguish between release and debug builds
# If nothing is specified assume Release
if(CMAKE_BUILD_TYPE MATCHES "Debug")
    # Enable verbose makefiles in debug builds
    set(CMAKE_VERBOSE_MAKEFILE ON)
        
    # QML-Debugging
    # Note: You need to pass e.g. "-qmljsdebugger=port:56346,block" to the application
    # to enable QML-Debugging.
    # You may also consider to export QML_DISABLE_OPTIMIZER=1 in your execution environment.
    add_definitions(-DQT_QML_DEBUG)
	
	# Enable creation of test target
	enable_testing()
	
	# Enable creation of tests
	set(MELP_TEST_CREATION ON)
else()
    # Disable qDebug output in release builds
    add_definitions(-DQT_NO_DEBUG_OUTPUT)
endif()

# C++ 11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)

if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g3")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -fmessage-length=0")
endif()

# QT_USE_FAST_CONCATENATION and QT_USE_FAST_OPERATOR_PLUS seems to be deprecated
# since 4.8 and replaced by QT_USE_QSTRINGBUILDER
add_definitions(-DQT_USE_QSTRINGBUILDER)

include_directories(src)

add_subdirectory(src)
add_subdirectory(src/gui)
melp_print_list(SRCS "Source files" SEPERATOR HALFINDENT)

qt5_add_resources(QRCS
    src/gui/qml.qrc
)
melp_print_list(QRCS "Resource files" SEPERATOR HALFINDENT)

add_executable(SevenSegmentDisplayDemo ${SRCS} ${QRCS})
target_link_libraries(SevenSegmentDisplayDemo Qt5::Qml Qt5::Quick)
