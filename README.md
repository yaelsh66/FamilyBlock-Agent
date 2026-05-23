# FamilyBlock Agent

Windows service that enforces screen-time rules on a child device by talking to the Family Block backend.

## Requirements

- Windows 10 or later
- Visual Studio 2022 with Desktop development with C++
- Administrator rights for installing the Windows service and editing the hosts file

## Configuration

Copy the example files:

```powershell
Copy-Item .env.example .env
Copy-Item config.example.json C:\FamilyBlockService\config.json
```

Set these values before running the service:

- `FAMILYBLOCK_BACKEND_URL`: same base URL as `VITE_BACKEND_URL` in the frontend
- `DEVICE_ID` and `DEVICE_SECRET`: created when a parent adds a device in the web app
- `FAMILYBLOCK_CONFIG_PATH`: optional override for the local config file path

You can store device credentials either in Windows environment variables or in `config.json`.

## Build

1. Open `FamilyBlockService.sln` in Visual Studio.
2. Select `Release` and `x64`.
3. Build the solution.

The executable is produced under `x64/Release/`.

## Local Development Against Docker Backend

If the backend is running locally through the workspace Docker stack:

```env
FAMILYBLOCK_BACKEND_URL=http://localhost:8081
```

Then create a device in the frontend parent UI and copy the device ID and password into `.env` or `config.json`.

## Backend Endpoint

The agent sends heartbeats to:

```text
POST /agent/heartbeat
```

This endpoint is public on the backend and authenticates using the device ID and secret.

## Install Notes

- Run the service with administrator privileges.
- Keep backend URL and device secrets out of Git.
- For production, point `FAMILYBLOCK_BACKEND_URL` at your Render backend URL over HTTPS.

See `DEPLOYMENT.md` for the full multi-repo deployment flow.
