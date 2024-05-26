# Минимальная версия CMake
cmake_minimum_required(VERSION 3.10)

# Название проекта
project(MicMuterProject)

# Установить стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Добавить исполняемый файл и указать подсистему Windows
add_executable(MicMuter WIN32 
    MicMuter/MicMuter.cpp
    MicMuter/resource.h
    MicMuter/resource.rc
    # Укажите все остальные файлы ресурсов, если они есть
)

# Включить каталоги заголовочных файлов, если они есть
# target_include_directories(MicMuter PRIVATE MicMuter/include)

# Добавить библиотеки, необходимые для работы с Windows API
target_link_libraries(MicMuter PRIVATE
    ole32
    uuid
    comctl32
    winmm
)

# Убедитесь, что ресурсный файл (.rc) включен в проект
if(WIN32)
    set(RESOURCE_FILES MicMuter/resource.rc)
    target_sources(MicMuter PRIVATE ${RESOURCE_FILES})
endif()
