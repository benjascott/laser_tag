add_executable(lasertag.elf
main.c
filterTest.c
histogram.c
sound.c
filter.c
timer_ps.c
lockoutTimer.c
hitLedTimer.c
transmitter.c
runningModes.c
trigger.c
isr.c
detector.c
# runningModes.c
)

add_subdirectory(sounds)
target_link_libraries(lasertag.elf ${330_LIBS} sounds lasertag_libs queue_lib)
set_target_properties(lasertag.elf PROPERTIES LINKER_LANGUAGE CXX)