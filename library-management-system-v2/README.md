# Library Management System — Version 2

## What is implemented

### Frontend
- React 19 + Vite
- React Router
- Axios
- Responsive dashboard
- Role-aware navigation
- Real login against C backend
- Real `/auth/me`
- Real logout
- Real book list from C API
- Student book requests
- Librarian/Admin book creation
- Live dashboard statistics

### Backend
- C HTTP server using sockets
- JSON REST endpoints
- SQLite database
- Server-side in-memory sessions
- Authentication
- Role-based authorization
- Books listing
- Book creation
- Student borrow request
- Librarian/Admin request approval/rejection
- Dashboard statistics
- CORS
- SQLite schema and seed data

## Important security note

This version uses **server-side opaque session tokens**, not fake JWT strings. This is intentional: a session token is simpler and safer for this educational C implementation. If you specifically need JWT, add a JWT library and replace the session store.

Passwords in the seed database are plain demo values for local development. For production, replace this with Argon2/bcrypt/scrypt hashing.

## Run on Windows with MSYS2 UCRT64

### 1. Install dependencies

```bash
pacman -Syu
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-sqlite3
```

### 2. Build backend

```bash
cd backend
make
./library_server.exe
```

Backend:
http://localhost:8080/health

### 3. Start frontend

Open another terminal:

```bash
cd frontend
npm install
npm run dev
```

Frontend:
http://localhost:5173

## Demo users

| Role | Email | Password |
|---|---|---|
| Student | student@library.local | student123 |
| Librarian | librarian@library.local | librarian123 |
| Admin | admin@library.local | admin123 |

## API

- POST `/api/auth/login`
- GET `/api/auth/me`
- POST `/api/auth/logout`
- GET `/api/books`
- POST `/api/books`
- POST `/api/requests`
- GET `/api/requests`
- PUT `/api/requests/:id/approve`
- PUT `/api/requests/:id/reject`
- GET `/api/dashboard/:role`

## Current scope

The architecture and database contain all major LMS entities: users, books, categories, requests, issued books, fines and activity logs.

The included C HTTP implementation fully connects authentication, sessions, RBAC, books, requests and dashboards. Issue/return and fine endpoints are database-ready and should be the next implementation milestone if you want every workflow exposed through the HTTP layer.
