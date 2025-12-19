# SSH Detection Auto-Configuration Feature

## Overview
Bottop now automatically detects whether it is running in a local session or over SSH, and adjusts the polling rate (`update_ms`) accordingly for optimal performance.

## How It Works

### Detection Method
The system checks for SSH environment variables:
- `SSH_CONNECTION`
- `SSH_CLIENT`
- `SSH_TTY`

If any of these variables are present, the session is identified as SSH.

### Auto-Configuration Behavior

| Connection Type | Default update_ms | Rationale |
|----------------|-------------------|-----------|
| **Local** | 1000ms (1 second) | Fast updates for responsive local monitoring |
| **SSH** | 5000ms (5 seconds) | Conservative rate to reduce network overhead |

### Preservation of User Settings
- **Only triggers if** `update_ms >= 10000ms` (default is 30000ms)
- **Preserves custom values**: If you set a custom value < 10000ms, it will not be overridden
- **One-time configuration**: Runs once at startup

## Testing Results

SSH Connection detected in current session
Recommended update_ms: 5000ms

## Configuration

### Manual Override
To set a custom polling rate that will not be auto-adjusted, edit ~/.config/bottop/bottop.conf:

```
update_ms = 2000
```

Custom values less than 10000ms will not be auto-adjusted.

### Recommended Values
- **100-500ms**: Very responsive, higher CPU usage
- **1000ms**: Good balance for local monitoring
- **2000-5000ms**: Recommended for graphs, good for SSH
- **10000ms+**: Will trigger auto-configuration

## Benefits
1. **Optimal Performance**: Fast updates when local, conservative when remote
2. **Bandwidth Efficiency**: Reduces SSH traffic overhead
3. **User-Friendly**: Works automatically without manual configuration
4. **Flexible**: Easy to override with custom values

## Implementation Details
- **Location**: `src/btop_config.cpp`
- **Functions**:
  - `is_ssh_session()`: Detects SSH connection
  - `adjust_update_ms_for_connection()`: Applies auto-configuration
- **Called during**: Configuration load, after environment variable overrides
