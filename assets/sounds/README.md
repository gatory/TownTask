# TaskTown Audio Assets

This directory contains all the sound effects and music for TaskTown.

## Sound Effects

### UI Sounds
- `ui_click.wav` - Button click sound
- `ui_hover.wav` - Button hover sound
- `notification.wav` - General notification sound

### Task System Sounds
- `task_complete.wav` - Task completion sound
- `xp_gain.wav` - Experience point gain sound

### Timer Sounds
- `timer_start.wav` - Timer start sound
- `timer_pause.wav` - Timer pause sound
- `timer_resume.wav` - Timer resume sound
- `timer_stop.wav` - Timer stop sound
- `timer_alert.wav` - Timer completion alert

### Habit Tracking Sounds
- `habit_checkin.wav` - Habit check-in sound
- `habit_streak.wav` - Habit streak achievement sound

### Building Sounds
- `building_enter.wav` - Entering a building
- `building_exit.wav` - Exiting a building
- `building_upgrade.wav` - Building upgrade sound
- `decoration_place.wav` - Placing a decoration

### Gamification Sounds
- `achievement.wav` - Achievement unlock sound
- `coffee_reward.wav` - Coffee token reward sound

## Music

### Ambient Music
- `town_ambient.ogg` - Town/overworld ambient music
- `coffee_shop_ambient.ogg` - Coffee shop ambient music
- `library_ambient.ogg` - Library ambient music
- `gym_ambient.ogg` - Gym ambient music
- `home_ambient.ogg` - Home ambient music
- `bulletin_board_ambient.ogg` - Bulletin board ambient music

### Special Music
- `focus_session.ogg` - Focus/pomodoro session music
- `celebration.ogg` - Achievement/celebration music

## Audio Guidelines

### Sound Effects
- Format: WAV, 44.1kHz, 16-bit
- Duration: 0.1-2.0 seconds
- Volume: Normalized to -6dB peak

### Music
- Format: OGG Vorbis, 44.1kHz, stereo
- Quality: 128-192 kbps
- Loop-friendly (seamless loops)
- Volume: Normalized to -12dB peak

## Attribution

All audio assets should be properly licensed for commercial use or created specifically for TaskTown.

## Implementation Notes

The AudioManager system supports:
- Dynamic volume control
- Crossfading between music tracks
- Pitch variation for sound effects
- Concurrent sound limiting
- Audio settings persistence