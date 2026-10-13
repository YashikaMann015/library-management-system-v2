# Library Management API

Base URL: `http://localhost:8080/api`

## Authentication

### POST /auth/login
Request:
```json
{"email":"student@library.local","password":"student123"}
```

Response:
```json
{
  "success": true,
  "user": {
    "id": 1,
    "name": "Rahul Sharma",
    "email": "student@library.local",
    "role": "STUDENT"
  },
  "token": "..."
}
```

## Books

GET `/books`
GET `/books/:id`
POST `/books`
PUT `/books/:id`
DELETE `/books/:id`

## Borrowing

POST `/requests`
GET `/requests`
PUT `/requests/:id/approve`
PUT `/requests/:id/reject`

POST `/issues`
GET `/issues`
POST `/issues/:id/return`

## Fines

GET `/fines`
PUT `/fines/:id/pay`

## Authorization matrix

| Endpoint | Student | Librarian | Admin |
|---|---:|---:|---:|
| Search books | Yes | Yes | Yes |
| Request book | Yes | No | No |
| Add book | No | Yes | Yes |
| Delete book | No | Yes | Yes |
| Approve request | No | Yes | Yes |
| Manage users | No | Limited | Yes |
| Manage librarians | No | No | Yes |
| Reports | No | Yes | Yes |
| Settings | No | No | Yes |
