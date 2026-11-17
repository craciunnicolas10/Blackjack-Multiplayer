# Visual Studio 2026 (v18) livreaza SegmentHeap.cmake; Visual Studio 2022 nu il are.
# Il includem doar daca exista, ca presetul MSVC sa mearga pe ambele versiuni.
if(DEFINED ENV{VSINSTALLDIR})
    file(TO_CMAKE_PATH "$ENV{VSINSTALLDIR}" _vs_install_dir)
    set(_vs_segment_heap "${_vs_install_dir}/Common7/IDE/CommonExtensions/Microsoft/CMake/cmake/Microsoft/SegmentHeap.cmake")
    if(EXISTS "${_vs_segment_heap}")
        include("${_vs_segment_heap}")
    endif()
endif()
