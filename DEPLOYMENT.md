# FamilyBlock Agent Deployment

The Windows agent is a local service installed on a child device. It does not run on Vercel, Render, or Supabase.

## What It Connects To

- Backend API: `POST /agent/heartbeat`
- Backend base URL: same value as `VITE_BACKEND_URL` in the frontend, stored locally as `FAMILYBLOCK_BACKEND_URL`

Examples:

```env
FAMILYBLOCK_BACKEND_URL=http://localhost:8081
FAMILYBLOCK_BACKEND_URL=https://your-backend.onrender.com
```

## Required Local Configuration

Copy the example files:

```powershell
Copy-Item .env.example .env
Copy-Item config.example.json C:\FamilyBlockService\config.json
```

Set these values:

```env
FAMILYBLOCK_BACKEND_URL=https://your-backend.onrender.com
DEVICE_ID=child-device-id
DEVICE_SECRET=device-password-from-parent-ui
FAMILYBLOCK_CONFIG_PATH=C:\FamilyBlockService\config.json
```

Where to get device credentials:

1. Log in to the frontend as a parent.
2. Open the device management page for a child.
3. Add a new device and copy the device ID and password.
4. Put those values into `.env` or `config.json`.

## Build And Install

1. Open `FamilyBlockService.sln` in Visual Studio 2022.
2. Build `Release | x64`.
3. Install and run the service with administrator privileges on the Windows machine.

## Local Development With Docker Backend

If you use the workspace Docker stack:

```env
FAMILYBLOCK_BACKEND_URL=http://localhost:8081
```

Start the backend first:

```bash
make up-build
```

Then configure the agent on the Windows machine to point at the backend host reachable from that machine. If the backend runs on your dev machine, `localhost:8081` is usually correct.

## Production Notes

- Use HTTPS in production: `FAMILYBLOCK_BACKEND_URL=https://your-backend.onrender.com`
- Keep device secrets out of Git.
- The agent requires Windows admin rights because it modifies the hosts file and manages processes.
- If the backend sleeps on Render free tier, heartbeats may fail until the backend wakes up. Use a health-check cron on the backend for smoother demos.

See the workspace `DEPLOYMENT.md` for the full frontend/backend/database deployment flow.
