cmake_minimum_required(VERSION 3.20)
project(engine_lab CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Visual Studio 솔루션에서 폴더로 묶을 수 있게 한다
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# 모든 실행 파일을 build/bin 한 곳으로
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# MSVC: 소스가 UTF-8임을 알려준다 (한글 주석/출력)
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/utf-8>)
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/W4>)
add_compile_options($<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-Wall>)

# ─────────────────────────────────────────────────────────────
# <카테고리>/<주제>/main.cpp 가 있으면 자동으로 실행 파일 타깃 생성.
# 새 주제를 시작할 때 CMakeLists를 건드릴 필요가 없다.
#   00-cpp/01-object-layout/main.cpp  ->  타깃 이름 object_layout
# ─────────────────────────────────────────────────────────────
file(GLOB_RECURSE LAB_MAINS CONFIGURE_DEPENDS "*/*/main.cpp")

foreach(SRC ${LAB_MAINS})
    get_filename_component(TOPIC_DIR ${SRC} DIRECTORY)
    get_filename_component(TOPIC_NAME ${TOPIC_DIR} NAME)

    # "01-object-layout" -> "object_layout"
    string(REGEX REPLACE "^[0-9]+-" "" TARGET_NAME ${TOPIC_NAME})
    string(REPLACE "-" "_" TARGET_NAME ${TARGET_NAME})

    add_executable(${TARGET_NAME} ${SRC})
    target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/tools)

    message(STATUS "lab target: ${TARGET_NAME}  <-  ${TOPIC_NAME}")
endforeach()

# ─────────────────────────────────────────────────────────
# <카테고리>/<주제>/extra/*.cpp — 곁가지 측정.
# 본 실험(main.cpp)과 달리 파일 하나가 곳 실행 파일 하나다.
#   00-cpp/03-move-semantics/extra/bind_table.cpp
#     ->  타깃 이름 move_semantics_bind_table
#     ->  솔루션 폴더 extra/move_semantics
# ─────────────────────────────────────────────────────────
file(GLOB_RECURSE LAB_EXTRAS CONFIGURE_DEPENDS "*/*/extra/*.cpp")

foreach(SRC ${LAB_EXTRAS})
    get_filename_component(EXTRA_DIR ${SRC} DIRECTORY)
    get_filename_component(TOPIC_DIR ${EXTRA_DIR} DIRECTORY)
    get_filename_component(TOPIC_NAME ${TOPIC_DIR} NAME)
    get_filename_component(FILE_NAME ${SRC} NAME_WE)

    string(REGEX REPLACE "^[0-9]+-" "" TOPIC_SHORT ${TOPIC_NAME})
    string(REPLACE "-" "_" TOPIC_SHORT ${TOPIC_SHORT})

    set(TARGET_NAME "${TOPIC_SHORT}_${FILE_NAME}")

    add_executable(${TARGET_NAME} ${SRC})
    target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/tools)
    set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "extra/${TOPIC_SHORT}")

    message(STATUS "extra target: ${TARGET_NAME}  <-  ${TOPIC_NAME}/extra")
endforeach()
