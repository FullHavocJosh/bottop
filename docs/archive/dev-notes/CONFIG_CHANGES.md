# Configuration Changes Summary

**Date:** December 13, 2025

## Overview

Restructured Bottop configuration to separate sensitive credentials from general settings for improved security.

---

## Changes Made

### 1. Configuration File Path ✅

**Old:** `~/.config/btop/bottop.conf`  
**New:** `~/.config/bottop/bottop.conf`

**Reason:** Bottop-specific directory to avoid conflicts with original btop

### 2. Environment Variables for Credentials ✅

**Added support for:**

- `BOTTOP_AC_SSH_HOST` - SSH connection string
- `BOTTOP_AC_DB_HOST` - Database hostname
- `BOTTOP_AC_DB_USER` - Database username
- `BOTTOP_AC_DB_PASS` - Database password (SENSITIVE)
- `BOTTOP_AC_DB_NAME` - Database name
- `BOTTOP_AC_CONTAINER` - Docker container name

**Location:** `~/.zshrc_envvars`

**Benefits:**

- ✅ Passwords never stored in config files
- ✅ Easy credential rotation
- ✅ Prevent accidental git commits of secrets
- ✅ Per-session override capability

### 3. Configuration Priority

Bottop now uses this order (highest to lowest):

1. **Environment variables** - From ~/.zshrc_envvars
2. **Config file** - ~/.config/bottop/bottop.conf
3. **Built-in defaults** - Empty strings (requires manual setup)

### 4. Code Changes

**Modified Files:**

- `src/btop_config.cpp` - Added environment variable loading
- `src/btop.cpp` - Config path already correct
- `apply_database_optimizations.sh` - Updated to prefer env vars

**Key Functions Added:**

```cpp
void load_env_overrides() {
    // Loads BOTTOP_AC_* environment variables
    // Called after config file is read
    // Overrides config file values
}
```

### 5. Documentation Updates

**Created:**

- `CONFIGURATION.md` - Comprehensive configuration guide (500+ lines)

**Updated:**

- `APPLY_DATABASE_OPTIMIZATIONS.md` - Environment variable instructions
- Config file comments - Reference environment variables

---

## Migration Guide

### For New Users

1. **Add environment variables:**

    ```bash
    cat >> ~/.zshrc_envvars << 'EOF'
    # Bottop Configuration
    export BOTTOP_AC_SSH_HOST='root@your-server'
    export BOTTOP_AC_DB_HOST='database-host'
    export BOTTOP_AC_DB_USER='root'
    export BOTTOP_AC_DB_PASS='your-password'
    export BOTTOP_AC_CONTAINER='container-name'
    EOF
    ```

2. **Reload environment:**

    ```bash
    source ~/.zshrc_envvars
    ```

3. **Run Bottop:**
    ```bash
    /home/havoc/bottop/build/bottop
    ```

### For Existing Users

**If you have old btop config with passwords:**

1. **Extract current values:**

    ```bash
    grep azerothcore ~/.config/btop/btop.conf
    ```

2. **Add to environment file:**

    ```bash
    # Copy values to ~/.zshrc_envvars
    export BOTTOP_AC_SSH_HOST='<from config>'
    export BOTTOP_AC_DB_PASS='<from config>'
    # etc.
    ```

3. **Reload and test:**

    ```bash
    source ~/.zshrc_envvars
    /home/havoc/bottop/build/bottop
    ```

4. **Optional - Clean up old config:**
    ```bash
    rm ~/.config/btop/bottop.conf
    # Will be recreated without passwords
    ```

---

## Security Improvements

### Before

```
~/.config/btop/bottop.conf:
  azerothcore_db_pass = "password123"  ❌ Plain text password in config
```

### After

```
~/.zshrc_envvars:
  export BOTTOP_AC_DB_PASS='password123'  ✅ Separate secure file

~/.config/bottop/bottop.conf:
  azerothcore_db_pass = "<set via BOTTOP_AC_DB_PASS environment variable>"
```

**Additional Security:**

```bash
# Restrict permissions on credentials file
chmod 600 ~/.zshrc_envvars

# Config file can be world-readable (no secrets)
chmod 644 ~/.config/bottop/bottop.conf
```

---

## Testing

### Verify Environment Variables

```bash
# Check variables are set
env | grep BOTTOP_

# Should output:
# BOTTOP_AC_SSH_HOST=root@testing-azerothcore.rollet.family
# BOTTOP_AC_DB_HOST=testing-ac-database
# BOTTOP_AC_DB_USER=root
# BOTTOP_AC_DB_PASS=password
# BOTTOP_AC_DB_NAME=acore_characters
# BOTTOP_AC_CONTAINER=testing-ac-worldserver
```

### Test Script

```bash
cd /home/havoc/bottop
./apply_database_optimizations.sh

# Should output:
# [1/6] Loading configuration...
# ✓ Using environment variables from ~/.zshrc_envvars
```

### Test Bottop

```bash
/home/havoc/bottop/build/bottop

# Should:
# 1. Connect successfully
# 2. Show AzerothCore data
# 3. Create config file at ~/.config/bottop/bottop.conf
# 4. Config file shows: "<set via BOTTOP_AC_DB_PASS environment variable>"
```

---

## Files Modified

### Source Code (2 files)

1. `src/btop_config.cpp` - Environment variable support
2. `src/btop.cpp` - Config path (already correct)

### Scripts (1 file)

1. `apply_database_optimizations.sh` - Prefer env vars over config

### Configuration (1 file)

1. `~/.zshrc_envvars` - Added Bottop credentials

### Documentation (3 files)

1. `CONFIGURATION.md` - New comprehensive guide
2. `APPLY_DATABASE_OPTIMIZATIONS.md` - Updated for env vars
3. `CONFIG_CHANGES.md` - This file

---

## Backward Compatibility

✅ **Fully backward compatible**

- Old config files still work
- Environment variables are **optional**
- Falls back to config file if env vars not set
- No breaking changes for existing users

**Migration is recommended but not required**

---

## Next Steps

1. **Current Session:**
    - ✅ Code changes complete
    - ✅ Environment variables set
    - ✅ Documentation updated
    - ⏳ **Testing needed** - Run Bottop to verify

2. **Future Enhancements:**
    - Support for multiple profiles (dev/staging/prod)
    - Config validation tool
    - Encrypted credential storage
    - Secret manager integration (Vault, AWS Secrets Manager)

---

## Support

### Documentation References

- **Configuration Guide:** See `CONFIGURATION.md`
- **Script Usage:** See `APPLY_DATABASE_OPTIMIZATIONS.md`
- **Troubleshooting:** Both docs have troubleshooting sections

### Common Issues

1. **"No configuration found"**
    - Set environment variables in ~/.zshrc_envvars
    - Or run Bottop once to create config file

2. **"Connection refused"**
    - Check SSH_HOST is correct
    - Verify SSH keys are configured
    - Test: `ssh $BOTTOP_AC_SSH_HOST "echo OK"`

3. **"Database access denied"**
    - Verify DB_USER and DB_PASS are correct
    - Check environment variables: `echo $BOTTOP_AC_DB_PASS`
    - Test connection manually

4. **"Variables not found"**
    - Reload: `source ~/.zshrc_envvars`
    - Check ~/.zshrc sources the file
    - Verify syntax (no spaces around `=`)

---

## Rollback (If Needed)

If you need to revert changes:

1. **Remove environment variables:**

    ```bash
    # Comment out in ~/.zshrc_envvars
    sed -i 's/^export BOTTOP_/#export BOTTOP_/' ~/.zshrc_envvars
    ```

2. **Use config file only:**

    ```bash
    nano ~/.config/bottop/bottop.conf
    # Set passwords directly in config
    ```

3. **Rebuild without env support:**
    ```bash
    # Not necessary - code still supports config file
    # Environment variables are purely additive
    ```

---

## Summary

**What Changed:**

- ✅ Config location: `~/.config/bottop/` (was already correct)
- ✅ Credentials: Environment variables preferred
- ✅ Security: Passwords separate from config files
- ✅ Compatibility: Fully backward compatible

**What Didn't Change:**

- ❌ Config file format (still key = "value")
- ❌ Config options available
- ❌ Bottop functionality
- ❌ Script behavior (just reads differently)

**Status:** ✅ **Ready to test**

Run Bottop to verify everything works with the new configuration system!
