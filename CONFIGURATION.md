# Bottop Configuration Guide

## Overview

Bottop stores configuration in two locations:

- **Non-sensitive settings:** `~/.config/bottop/bottop.conf`
- **Sensitive credentials:** `~/.zshrc_envvars` (environment variables)

This separation keeps passwords and SSH credentials out of config files.

---

## Quick Start

### 1. Set Environment Variables

Add these to `~/.zshrc_envvars`:

```bash
# Bottop (AzerothCore monitoring) Configuration
export BOTTOP_AC_SSH_HOST='root@your-server.example.com'
export BOTTOP_AC_DB_HOST='database-hostname'
export BOTTOP_AC_DB_USER='root'
export BOTTOP_AC_DB_PASS='your-secure-password'
export BOTTOP_AC_DB_NAME='acore_characters'
export BOTTOP_AC_CONTAINER='worldserver-container-name'
```

**Make sure your `~/.zshrc` sources this file:**

```bash
# Add to ~/.zshrc if not already present:
[ -f ~/.zshrc_envvars ] && source ~/.zshrc_envvars
```

### 2. Reload Environment

```bash
source ~/.zshrc_envvars
```

### 3. Run Bottop

```bash
/home/havoc/bottop/build/bottop
```

On first run, Bottop will create `~/.config/bottop/bottop.conf` with default settings.

---

## Environment Variables (Sensitive Data)

### Required Variables

| Variable              | Description                           | Example                   |
| --------------------- | ------------------------------------- | ------------------------- |
| `BOTTOP_AC_SSH_HOST`  | SSH connection string                 | `root@server.example.com` |
| `BOTTOP_AC_DB_HOST`   | Database host (from inside container) | `testing-ac-database`     |
| `BOTTOP_AC_DB_USER`   | Database username                     | `root`                    |
| `BOTTOP_AC_DB_PASS`   | **Database password** (SENSITIVE)     | `SecurePassword123!`      |
| `BOTTOP_AC_CONTAINER` | Docker container name                 | `testing-ac-worldserver`  |

### Optional Variables

| Variable            | Description   | Default            |
| ------------------- | ------------- | ------------------ |
| `BOTTOP_AC_DB_NAME` | Database name | `acore_characters` |

### Why Environment Variables?

✅ **Security Benefits:**

- Passwords never written to disk in plain text config files
- Environment variables can have restricted permissions (600 on ~/.zshrc_envvars)
- No risk of accidentally committing passwords to git
- Easy to rotate credentials without editing config files

✅ **Flexibility:**

- Override config file values per-session
- Different credentials for different shells/terminals
- Share config files without exposing secrets

---

## Config File (Non-Sensitive Settings)

Location: `~/.config/bottop/bottop.conf`

This file is created automatically on first run. It contains:

- Theme settings
- Display preferences
- Update intervals
- Box visibility

### Example Config File

```ini
#? Config file for btop v. 1.4.5

#* SECURITY NOTE: Sensitive values can be set via environment variables:
#* - BOTTOP_AC_DB_PASS    (database password)
#* - BOTTOP_AC_DB_USER    (database username)
#* - BOTTOP_AC_SSH_HOST   (SSH connection string)
#* - BOTTOP_AC_DB_HOST    (database host)
#* - BOTTOP_AC_DB_NAME    (database name)
#* - BOTTOP_AC_CONTAINER  (docker container name)
#* Add these to ~/.zshrc_envvars to keep passwords out of this config file.

#* Color theme
color_theme = "Default"

#* Theme background
theme_background = True

#* Update interval in milliseconds
update_ms = 2000

#* SSH host for AzerothCore monitoring (format: user@hostname[:port]).
#* Can be set via environment variable: BOTTOP_AC_SSH_HOST
azerothcore_ssh_host = ""

#* Database host for AzerothCore monitoring.
#* Can be set via environment variable: BOTTOP_AC_DB_HOST
azerothcore_db_host = ""

#* Database username for AzerothCore monitoring.
#* Can be set via environment variable: BOTTOP_AC_DB_USER
azerothcore_db_user = ""

#* Database password for AzerothCore monitoring.
#* SECURITY: Recommended to set via environment variable: BOTTOP_AC_DB_PASS
azerothcore_db_pass = "<set via BOTTOP_AC_DB_PASS environment variable>"

#* Database name for AzerothCore monitoring.
#* Can be set via environment variable: BOTTOP_AC_DB_NAME
azerothcore_db_name = "acore_characters"

#* Docker container name for AzerothCore server.
#* Can be set via environment variable: BOTTOP_AC_CONTAINER
azerothcore_container = ""
```

---

## Configuration Priority

Bottop uses this order (highest to lowest):

1. **Environment variables** (from ~/.zshrc_envvars)
2. **Config file** (~/.config/bottop/bottop.conf)
3. **Built-in defaults** (empty strings, requiring env vars)

### Example

If you have:

- Config file: `azerothcore_db_pass = "password1"`
- Environment: `BOTTOP_AC_DB_PASS='password2'`

Bottop will use `password2` (environment variable wins).

---

## Supporting Scripts

### apply_database_optimizations.sh

The database optimization script also uses the same configuration:

1. **First checks** environment variables
2. **Falls back to** config file
3. **Errors if** neither is configured

```bash
# Works with environment variables
export BOTTOP_AC_SSH_HOST='root@server'
./apply_database_optimizations.sh

# Or with config file
# (reads from ~/.config/bottop/bottop.conf)
./apply_database_optimizations.sh
```

---

## Security Best Practices

### 1. Protect Your Environment File

```bash
# Set restrictive permissions on credentials file
chmod 600 ~/.zshrc_envvars
```

### 2. Never Commit Credentials

Add to `.gitignore`:

```
.zshrc_envvars
*.conf
```

### 3. Use Strong Passwords

Generate secure passwords:

```bash
openssl rand -base64 32
```

### 4. Rotate Credentials Regularly

When changing passwords:

1. Update `~/.zshrc_envvars`
2. Reload: `source ~/.zshrc_envvars`
3. No need to edit config file!

### 5. Use SSH Key Authentication

Instead of SSH passwords:

```bash
# Generate key (if needed)
ssh-keygen -t ed25519

# Copy to server
ssh-copy-id root@server.example.com

# Test connection
ssh root@server.example.com "echo 'SSH OK'"
```

---

## Troubleshooting

### Config File Not Created

**Problem:** `~/.config/bottop/bottop.conf` doesn't exist

**Solution:** Run Bottop once:

```bash
/home/havoc/bottop/build/bottop
# Press 'q' to quit after it starts
```

### Environment Variables Not Working

**Problem:** Changes to ~/.zshrc_envvars don't take effect

**Solution 1:** Reload in current shell

```bash
source ~/.zshrc_envvars
```

**Solution 2:** Start new shell

```bash
zsh  # or bash
```

**Solution 3:** Check if sourced in ~/.zshrc

```bash
grep "zshrc_envvars" ~/.zshrc
# Should output: [ -f ~/.zshrc_envvars ] && source ~/.zshrc_envvars
```

### Connection Fails with "Permission Denied"

**Problem:** SSH authentication fails

**Solutions:**

1. **Check SSH key:**

    ```bash
    ssh root@your-server "echo OK"
    ```

2. **Add key to agent:**

    ```bash
    ssh-add ~/.ssh/id_ed25519
    ```

3. **Verify permissions:**
    ```bash
    chmod 600 ~/.ssh/id_ed25519
    chmod 644 ~/.ssh/id_ed25519.pub
    chmod 700 ~/.ssh
    ```

### Database Connection Fails

**Problem:** "Cannot connect to MySQL"

**Checklist:**

1. **Environment variables set?**

    ```bash
    echo $BOTTOP_AC_DB_PASS
    # Should output your password
    ```

2. **Correct database host?**
    - Use hostname **from inside container**
    - Usually `testing-ac-database` (not `localhost`)

3. **Test connection manually:**
    ```bash
    ssh $BOTTOP_AC_SSH_HOST \
      "docker exec $BOTTOP_AC_CONTAINER \
      mysql -h$BOTTOP_AC_DB_HOST \
      -u$BOTTOP_AC_DB_USER \
      -p$BOTTOP_AC_DB_PASS \
      -e 'SELECT 1'"
    ```

### Values Don't Override

**Problem:** Environment variables don't override config file

**Debug:**

1. **Check variable is set:**

    ```bash
    env | grep BOTTOP_
    ```

2. **Verify Bottop sees it:**

    ```bash
    # Run Bottop and check which values it uses
    # (Look for connection errors with specific host/user)
    ```

3. **Restart Bottop:**
    - Environment is read on startup
    - Changes require restart

---

## Example Setups

### Development (Local)

```bash
# ~/.zshrc_envvars
export BOTTOP_AC_SSH_HOST='root@localhost'
export BOTTOP_AC_DB_HOST='localhost'
export BOTTOP_AC_DB_USER='root'
export BOTTOP_AC_DB_PASS='dev_password'
export BOTTOP_AC_CONTAINER='azerothcore-worldserver'
```

### Production (Remote)

```bash
# ~/.zshrc_envvars
export BOTTOP_AC_SSH_HOST='root@prod.example.com'
export BOTTOP_AC_DB_HOST='prod-db-hostname'
export BOTTOP_AC_DB_USER='acore_readonly'
export BOTTOP_AC_DB_PASS='$(cat ~/.secrets/ac_db_pass)'  # From secret manager
export BOTTOP_AC_CONTAINER='prod-worldserver'
```

### Testing (Staging)

```bash
# ~/.zshrc_envvars
export BOTTOP_AC_SSH_HOST='root@staging.example.com'
export BOTTOP_AC_DB_HOST='staging-database'
export BOTTOP_AC_DB_USER='test_user'
export BOTTOP_AC_DB_PASS='test_password_123'
export BOTTOP_AC_CONTAINER='staging-worldserver'
```

---

## Migration from Old Config

If you have an existing config with passwords in the file:

### Step 1: Extract Values

```bash
# View current config
cat ~/.config/btop/bottop.conf | grep azerothcore
```

### Step 2: Add to Environment File

```bash
# Add to ~/.zshrc_envvars
export BOTTOP_AC_SSH_HOST='<value from config>'
export BOTTOP_AC_DB_HOST='<value from config>'
export BOTTOP_AC_DB_USER='<value from config>'
export BOTTOP_AC_DB_PASS='<value from config>'
export BOTTOP_AC_CONTAINER='<value from config>'
```

### Step 3: Reload and Test

```bash
source ~/.zshrc_envvars
/home/havoc/bottop/build/bottop
```

### Step 4: Clean Up Config (Optional)

```bash
# Remove old config to regenerate clean version
rm ~/.config/btop/bottop.conf
# Run Bottop to create new config (will use env vars)
/home/havoc/bottop/build/bottop
```

---

## Files Summary

| File                           | Purpose                            | Contains Passwords? |
| ------------------------------ | ---------------------------------- | ------------------- |
| `~/.config/bottop/bottop.conf` | Display settings, theme, intervals | ❌ No               |
| `~/.zshrc_envvars`             | Credentials and connection info    | ✅ Yes              |
| `~/.zshrc`                     | Shells startup, sources envvars    | ❌ No               |

**Remember:** Only `~/.zshrc_envvars` contains sensitive data - protect it accordingly!

---

## Getting Help

If configuration isn't working:

1. Check environment variables: `env | grep BOTTOP_`
2. Verify config file exists: `ls -la ~/.config/bottop/`
3. Test connectivity: `ssh $BOTTOP_AC_SSH_HOST "echo OK"`
4. Check Bottop logs/errors when starting
5. Review this guide's Troubleshooting section

For more help, create a GitHub issue with:

- Bottop version: `/home/havoc/bottop/build/bottop --version`
- Error messages (remove passwords!)
- Config file content (remove passwords!)
