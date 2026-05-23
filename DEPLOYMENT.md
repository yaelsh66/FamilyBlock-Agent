# FamilyBlock Agent Deployment

Local Windows service on child devices. Not Vercel, Render, or Supabase.

## Backend Connection

- Endpoint: `POST /agent/heartbeat`
- Base URL: same as `VITE_BACKEND_URL`, stored as `FAMILYBLOCK_BACKEND_URL`

```env
FAMILYBLOCK_BACKEND_URL=http://localhost:8081
FAMILYBLOCK_BACKEND_URL=https://your-backend.onrender.com
```

## Setup

```powershell
Copy-Item .env.example .env
Copy-Item config.example.json C:\FamilyBlockService\config.json
```

```env
FAMILYBLOCK_BACKEND_URL=https://your-backend.onrender.com
DEVICE_ID=child-device-id
DEVICE_SECRET=device-password-from-parent-ui
FAMILYBLOCK_CONFIG_PATH=C:\FamilyBlockService\config.json
```

Device credentials: parent login → device management → add device → copy ID/password → `.env` or `config.json`.

## Build & Install

1. Open `FamilyBlockService.sln` in Visual Studio 2022
2. Build `Release | x64`
3. Install/run service as admin

## Local Docker Backend

```env
FAMILYBLOCK_BACKEND_URL=http://localhost:8081
```

```bash
make up-build
```

Point agent at backend host reachable from the Windows machine. Dev machine backend = `localhost:8081` usually works.

## Production

- HTTPS: `FAMILYBLOCK_BACKEND_URL=https://your-backend.onrender.com`
- Device secrets out of Git
- Admin rights required (hosts file + process management)
- Render free tier sleep → heartbeat failures until wake; use backend health-check cron for demos

Full stack: workspace `DEPLOYMENT.md`
