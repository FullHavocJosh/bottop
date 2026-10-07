# Complete Stock btop Removal Plan

## Objective

Remove ALL stock btop system monitoring code, leaving only:

- Core infrastructure (main loop, config, theming, input handling)
- AzerothCore monitoring functionality
- Menu system (for quit/options)

## Files Analysis

### Files to REMOVE Completely

1. **src/linux/btop_collect.cpp** (118KB) - All Linux system monitoring
    - CPU stats collection
    - Memory stats collection
    - Network stats collection
    - Process list collection
    - Disk I/O collection

### Files to HEAVILY MODIFY

#### 1. btop_draw.cpp (116KB) - Remove ~80% of code

**Remove these entire namespaces:**

- `namespace Cpu { ... }` (lines ~521-975) - CPU box drawing
- `namespace Gpu { ... }` (lines ~976-1166) - GPU box drawing
- `namespace Mem { ... }` (lines ~1167-1432) - Memory box drawing
- `namespace Net { ... }` (lines ~1433-1532) - Network box drawing
- `namespace Proc { ... }` (lines ~1533-2389) - Process box drawing

**Keep:**

- `namespace Draw { ... }` - Core drawing functions
- `namespace Draw::AzerothCore { ... }` - Our monitoring
- `calcSizes()` function - Simplified for AzerothCore only
- Drawing utilities and helpers

**Estimated reduction:** 116KB → ~30KB

#### 2. btop_input.cpp (20KB) - Remove ~70% of code

**Remove:**

- All Proc box input handlers (lines ~336-506)
- All Cpu box input handlers (lines ~508-538)
- All Mem box input handlers (lines ~540-565)
- All Net box input handlers (lines ~567-601)
- All Gpu box input handlers (if any)
- Mouse input handling for stock boxes

**Keep:**

- Core input polling/reading
- Menu keybinds (q, esc)
- AzerothCore zone navigation (z, arrows, enter)

**Estimated reduction:** 20KB → ~5KB

#### 3. btop.cpp (40KB) - Remove ~50% of code

**Remove:**

- CPU data collection calls
- Memory data collection calls
- Network data collection calls
- Process data collection calls
- GPU data collection calls
- All stock box initialization
- Stock box update loops

**Keep:**

- Main event loop
- Signal handling
- Terminal setup/cleanup
- Config loading
- AzerothCore initialization and updates
- Menu handling

**Estimated reduction:** 40KB → ~20KB

#### 4. btop_config.cpp (25KB) - Minimal changes

**Remove/Comment out:**

- CPU-specific config options
- Memory-specific config options
- Network-specific config options
- Process-specific config options
- GPU-specific config options
- Box toggle configs

**Keep:**

- Core config system
- Theme config
- Window/display config
- Update interval config
- AzerothCore connection configs

**Estimated reduction:** 25KB → ~20KB

### Files to KEEP As-Is (Core Infrastructure)

1. **main.cpp** - Entry point
2. **btop.hpp** - Main header
3. **btop_cli.cpp/hpp** - Command line parsing
4. **btop_draw.hpp** - Drawing definitions (after cleanup)
5. **btop_input.hpp** - Input definitions
6. **btop_menu.cpp/hpp** - Menu system
7. **btop_shared.cpp/hpp** - Shared utilities, Terminal handling
8. **btop_theme.cpp/hpp** - Theme system
9. **btop_tools.cpp/hpp** - Utility functions
10. **btop_azerothcore.cpp/hpp** - Our monitoring (100% keep)
11. **btop_collect_stub.cpp** - Empty stub (keep as placeholder)
12. **config.h.in** - CMake config template

## Implementation Strategy

### Phase 1: Remove System Collection (Safest First)

1. ✅ Already done - btop_collect_stub.cpp exists
2. Remove `src/linux/btop_collect.cpp` file
3. Update CMakeLists.txt if needed

### Phase 2: Clean btop_draw.cpp

1. Remove `namespace Cpu` block entirely
2. Remove `namespace Gpu` block entirely
3. Remove `namespace Mem` block entirely
4. Remove `namespace Net` block entirely
5. Remove `namespace Proc` block entirely
6. Update `calcSizes()` to only handle AzerothCore box
7. Update draw calls in main loop section

### Phase 3: Clean btop_input.cpp

1. Remove all Proc input handlers (after line 337)
2. Remove all Cpu input handlers
3. Remove all Mem input handlers
4. Remove all Net input handlers
5. Remove all Gpu input handlers
6. Keep only: q (quit), esc/m (menu), z (zones)

### Phase 4: Clean btop.cpp

1. Remove CPU data collection
2. Remove Memory data collection
3. Remove Network data collection
4. Remove Process data collection
5. Remove GPU data collection
6. Simplify main loop to only handle AzerothCore
7. Remove stock box initialization code

### Phase 5: Clean btop_config.cpp

1. Comment out unused config options
2. Keep only relevant configs
3. Update config file generation

### Phase 6: Update CMakeLists.txt

1. Remove linux/btop_collect.cpp from sources
2. Verify all dependencies
3. Clean up any GPU-specific build flags

## Testing Checklist After Each Phase

- [ ] Code compiles without errors
- [ ] bottop starts without crashing
- [ ] AzerothCore box displays correctly
- [ ] Performance graph shows
- [ ] Zone list shows
- [ ] 'z' key activates zone navigation
- [ ] Arrow keys navigate zones
- [ ] Enter expands zones
- [ ] Left arrow exits navigation
- [ ] Esc opens menu
- [ ] 'q' quits cleanly

## Expected Results

### Before (Current)

- Total source code: ~400KB
- Stock btop boxes: Disabled but code present
- Build time: ~15 seconds
- Binary size: ~2-3MB
- Lines of code: ~15,000

### After (Goal)

- Total source code: ~150KB (62% reduction)
- Stock btop boxes: Completely removed
- Build time: ~5-8 seconds
- Binary size: ~800KB-1.5MB
- Lines of code: ~5,000

### Benefits

1. **Cleaner codebase** - Only relevant code
2. **Faster builds** - Less to compile
3. **Smaller binary** - No unused code
4. **Easier maintenance** - Less code to understand
5. **No confusion** - Can't accidentally enable stock boxes
6. **Better performance** - No dead code paths

## Risks & Mitigation

### Risk 1: Breaking shared utilities

**Mitigation:** Keep all btop_shared.cpp/hpp, btop_tools.cpp/hpp intact

### Risk 2: Breaking theme system

**Mitigation:** Keep all theme code, just remove references to stock box themes

### Risk 3: Breaking menu system

**Mitigation:** Keep menu code, remove only stock box menu options

### Risk 4: Build errors from missing dependencies

**Mitigation:** Use stub functions where needed, keep function signatures

## Rollback Plan

If anything breaks:

1. Restore from backup: `cp -r /home/havoc/bottop_backup_* /home/havoc/bottop`
2. Rebuild: `cd /home/havoc/bottop && cmake --build build`
3. Investigate specific issue
4. Fix and retry

## Files to Create

1. `REMOVE_SYSTEM_MONITORING.md` - This document
2. `remove_system_monitoring.sh` - Automated removal script
3. `CLEANUP_SUMMARY.md` - Post-cleanup documentation
4. `BEFORE_AFTER_COMPARISON.md` - Stats and comparison

## Next Steps

Would you like me to:

1. **Option A:** Remove everything in one go (fastest but riskier)
2. **Option B:** Remove phase by phase with testing after each (slower but safer)
3. **Option C:** Create a separate branch and do major surgery there

**Recommendation:** Option B - Phase by phase with testing
