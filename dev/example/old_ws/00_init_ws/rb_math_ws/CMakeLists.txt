cmake_minimum_required(VERSION 3.10)

get_filename_component(ProjectName ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" ProjectName ${ProjectName})

#実行ファイル名を指定
project(${ProjectName})

IF(WIN32)
set(CMAKE_C_FLAGS "-std=c99")
ELSE(WIN32)
set(CMAKE_C_FLAGS "-std=gnu99")
ENDIF(WIN32)

set(CMAKE_CXX_FLAGS "-std=c++1y -O0 -g")

IF(WIN32)
	add_definitions(-DWINDOWS)
	add_compile_options(/wd4819)
	add_compile_options(-D _AFXDLL)
	add_compile_options(/wd4103)
	add_compile_options(/wd4530)
ENDIF(WIN32)

#=================================#
#ビルドオプション
option(USE_DEBUG_INFO "USE_DEBUG_INFO" OFF)
if(USE_DEBUG_INFO)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D DBG_INFO")
endif()

option(DEV_UT_INFO "DEV_UT" OFF)
if(DEV_UT_INFO)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D DEV_UT")
endif()

option(DEV_MATH_INFO "DEV_MATH" OFF)
if(DEV_MATH_INFO)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D DEV_MATH")
endif()

#Dev_MainCycleディレクトリ, ssbライブラリディレクトリの指定
#============================#


set(COMMON_DIR
	../../Common
)

set(LIB_DIR
	../../Lib
)

set(TOOLS_DIR
	../../Tools
)

set(SRCS_DIR
	srcs
)

set(INCLUDE_DIRS
	${COMMON_DIR}
	${LIB_DIR}	
	${TOOLS_DIR}
	${SRCS_DIR}
)

#includeディレクトリ指定
include_directories(
	${INCLUDE_DIRS}
)

# =================================#
# main_src指定
set(MAIN_SRCS
	${SRCS_DIR}/main.c
	${SRCS_DIR}/Mainctrl.c
	${SRCS_DIR}/DbgCmd.c
)

set(LIB_SRCS
	${LIB_DIR}/RB_Math.c
)

set(TOOLS_SRCS
	${TOOLS_DIR}/Kbhit.c
)

set(UT_SRCS
	${LIB_DIR}/RB_Math_UT.c
)

# ProjectNameという名前の実行ファイルを作成
add_executable(
	${ProjectName}
	${MAIN_SRCS}
	${LIB_SRCS}
	${TOOLS_SRCS}
	${UT_SRCS}
)

#当該ターゲットで用いられているライブラリ指定
IF (NOT WIN32)
	target_link_libraries ( ${ProjectName} m stdc++ supc++ pthread )
ENDIF()
